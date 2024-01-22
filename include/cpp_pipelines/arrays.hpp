#pragma once

#include <cpp_pipelines/iterator_interface.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tpl.hpp>

namespace cpp_pipelines
{

using index_type = std::ptrdiff_t;

static constexpr index_type dynamic_value = -1;

template <index_type I>
struct value_holder
{
    value_holder(index_type i = I)
    {
        assert(i == I);
    }

    constexpr index_type get() const
    {
        return I;
    }
};

template <>
struct value_holder<dynamic_value>
{
    index_type v_;

    value_holder(index_type v)
        : v_(v)
    {
    }

    constexpr index_type get() const
    {
        return v_;
    }
};

template <index_type I>
std::ostream& operator<<(std::ostream& os, const value_holder<I> item)
{
    return os << item.get();
}

template <index_type L, index_type R>
bool operator==(value_holder<L> lhs, value_holder<R> rhs)
{
    return lhs.get() == rhs.get();
}

template <index_type L, index_type R>
bool operator!=(value_holder<L> lhs, value_holder<R> rhs)
{
    return !(lhs == rhs);
}

template <index_type Size = dynamic_value, index_type Stride = dynamic_value>
struct dim
{
    value_holder<Size> size_;
    value_holder<Stride> stride_;

    dim(index_type size = Size, index_type stride = Stride)
        : size_(size)
        , stride_(stride)
    {
    }

    constexpr index_type min() const
    {
        return 0;
    }

    constexpr index_type max() const
    {
        return min() + size() - 1;
    }

    constexpr index_type lower() const
    {
        return min();
    }

    constexpr index_type upper() const
    {
        return min() + size();
    }

    constexpr index_type size() const
    {
        return size_.get();
    }

    constexpr index_type stride() const
    {
        return stride_.get();
    }

    constexpr index_type offset(index_type i) const
    {
        return (i - min()) * stride();
    }

    constexpr bool contains(index_type i) const
    {
        return min() <= i < upper();
    }

    constexpr auto begin() const
    {
        return seq::range(lower(), upper()).begin();
    }

    constexpr auto end() const
    {
        return seq::range(lower(), upper()).end();
    }

    friend std::ostream& operator<<(std::ostream& os, const dim& item)
    {
        return os << "[" << item.min() << ", " << item.size() << ", " << item.stride() << "]";
    }
};

template <class T, std::size_t D>
struct vector : std::array<T, D>
{
    using base_type = std::array<T, D>;

    using base_type::base_type;
    using base_type::begin;
    using base_type::end;
    using base_type::size;
    using base_type::operator[];
    using base_type::at;

    vector()
    {
        std::fill(begin(), end(), T{});
    }

    vector(std::initializer_list<T> init)
    {
        std::copy(init.begin(), init.end(), begin());
    }

    friend bool operator==(const vector& lhs, const vector& rhs)
    {
        return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
    }

    friend bool operator!=(const vector& lhs, const vector& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const vector& item)
    {
        os << "[";
        for (std::size_t i = 0; i < D; ++i)
        {
            os << (i != 0 ? ", " : "") << item[i];
        }
        os << "]";
        return os;
    }
};

template <class... Dims>
struct shape;

template <class... Dims>
auto to_shape(std::tuple<Dims...> tuple) -> shape<Dims...>
{
    return { tuple };
}

template <class... Dims>
struct shape
{
    static constexpr std::size_t rank = sizeof...(Dims);
    using coords = vector<index_type, rank>;
    using dims_tuple = std::tuple<Dims...>;
    dims_tuple dims_;

    shape() = default;

    shape(Dims... dims)
        : dims_{ std::move(dims)... }
    {
    }

    shape(dims_tuple dims)
        : dims_{ std::move(dims) }
    {
    }

    friend std::ostream& operator<<(std::ostream& os, const shape& item)
    {
        return os << item.dims_;
    }

    template <std::size_t D>
    const auto slice() const
    {
        return to_shape(tpl::erase<D>(dims_));
    }

    template <std::size_t D>
    constexpr auto get() const -> const typename std::tuple_element<D, dims_tuple>::type&
    {
        return std::get<D>(dims_);
    }

    constexpr coords min() const
    {
        return min(std::index_sequence_for<Dims...>{});
    }

    constexpr coords max() const
    {
        return max(std::index_sequence_for<Dims...>{});
    }

    constexpr coords lower() const
    {
        return lower(std::index_sequence_for<Dims...>{});
    }

    constexpr coords upper() const
    {
        return upper(std::index_sequence_for<Dims...>{});
    }

    constexpr coords size() const
    {
        return size(std::index_sequence_for<Dims...>{});
    }

    constexpr coords stride() const
    {
        return stride(std::index_sequence_for<Dims...>{});
    }

    constexpr index_type volume() const
    {
        return volume(std::index_sequence_for<Dims...>{});
    }

    constexpr index_type offset(const coords& c) const
    {
        return offset(c, std::index_sequence_for<Dims...>{});
    }

    struct iter
    {
        const shape* owner_;
        coords c_;

        iter() = default;

        iter(const shape* owner, coords c)
            : owner_(owner)
            , c_(c)
        {
        }

        constexpr coords deref() const
        {
            return c_;
        }

        constexpr void inc()
        {
            inc<0>();
        }

        constexpr bool is_equal(const iter& other) const
        {
            return c_ == other.c_;
        }

        template <std::size_t D>
        void inc()
        {
            ++c_[D];
            if (c_[D] == owner_->get<D>().upper())
            {
                if constexpr (D + 1 < rank)
                {
                    c_[D] = owner_->get<D>().lower();
                    inc<D + 1>();
                }
            }
        }
    };

    using iterator = iterator_interface<iter>;

    constexpr iterator begin() const
    {
        return iterator{ iter{ this, min() } };
    }

    constexpr iterator end() const
    {
        coords end_location;
        end_location[rank - 1] = get<rank - 1>().size();
        return iterator{ iter{ this, end_location } };
    }

private:
    template <std::size_t... I>
    constexpr coords min(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().min()... } };
    }

    template <std::size_t... I>
    constexpr coords max(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().max()... } };
    }

    template <std::size_t... I>
    constexpr coords lower(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().lower()... } };
    }

    template <std::size_t... I>
    constexpr coords upper(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().upper()... } };
    }

    template <std::size_t... I>
    constexpr coords size(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().size()... } };
    }

    template <std::size_t... I>
    constexpr coords stride(std::index_sequence<I...>) const
    {
        return coords{ { get<I>().stride()... } };
    }

    template <std::size_t... I>
    constexpr index_type volume(std::index_sequence<I...>) const
    {
        return (1 * ... * get<I>().size());
    }

    template <std::size_t... I>
    constexpr index_type offset(const coords& c, std::index_sequence<I...>) const
    {
        return (0 + ... + get<I>().offset(std::get<I>(c)));
    }
};

template <class T, class Shape>
struct array_ref;

template <class T, class Shape>
auto to_array_ref(T* ptr, const Shape& shape) -> array_ref<T, Shape>
{
    return { ptr, shape };
}

using byte = std::uint8_t;

template <class T, class Shape>
struct array_ref
{
    using shape_type = Shape;
    using coords = typename shape_type::coords;
    byte* ptr_;
    shape_type shape_;

    array_ref(T* ptr, const shape_type& shape = {})
        : ptr_{ reinterpret_cast<byte*>(ptr) }
        , shape_{ shape }
    {
    }

    const shape_type& shape() const
    {
        return shape_;
    }

    T& operator[](const coords& c) const
    {
        return *reinterpret_cast<T*>(ptr_ + shape_.offset(c));
    }

    template <std::size_t D>
    auto slice(index_type n) const
    {
        coords c;
        c[D] = n;
        return to_array_ref(reinterpret_cast<T*>(ptr_ + shape_.offset(c)), shape_.template slice<D>());
    }

    friend std::ostream& operator<<(std::ostream& os, const array_ref& item)
    {
        return os << item.shape_;
    }

    auto values() const
    {
        return shape_ |= seq::transform([&](const coords& c) -> T&
                                        { return (*this)[c]; });
    }

    auto values_and_coords() const
    {
        return shape_ |= seq::transform([&](const coords& c) -> std::tuple<T&, coords>
                                        { return { (*this)[c], c }; });
    }
};

}  // namespace cpp_pipelines
