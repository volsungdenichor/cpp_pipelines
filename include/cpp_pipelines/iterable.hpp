#pragma once

#include <cpp_pipelines/seq/views.hpp>
#include <memory>

namespace cpp_pipelines
{
namespace detail
{
template <class T>
struct i_iterator
{
    virtual ~i_iterator() = default;
    virtual T deref() const = 0;
    virtual void inc() = 0;
    virtual bool is_equal(const i_iterator& other) const = 0;
    virtual std::unique_ptr<i_iterator> clone() const = 0;
};

template <class T>
struct i_range
{
    virtual ~i_range() = default;
    virtual std::unique_ptr<i_iterator<T>> begin() const = 0;
    virtual std::unique_ptr<i_iterator<T>> end() const = 0;
};

template <class T>
struct iterable_base
{
    template <class Range>
    struct range_wrapper : public i_range<T>
    {
        Range range;

        struct iterator : public i_iterator<T>
        {
            using inner_iter = iterator_t<Range>;
            inner_iter it;

            iterator(inner_iter it) : it{ it }
            {
            }

            T deref() const override
            {
                return *it;
            }

            void inc() override
            {
                ++it;
            }

            bool is_equal(const i_iterator<T>& other) const override
            {
                return it == static_cast<const iterator&>(other).it;
            }

            std::unique_ptr<i_iterator<T>> clone() const override
            {
                return std::make_unique<iterator>(it);
            }
        };

        range_wrapper(Range range) : range{ std::move(range) }
        {
        }

        std::unique_ptr<i_iterator<T>> begin() const override
        {
            return std::make_unique<iterator>(std::begin(range));
        }

        std::unique_ptr<i_iterator<T>> end() const override
        {
            return std::make_unique<iterator>(std::end(range));
        }
    };

    struct iter
    {
        std::unique_ptr<i_iterator<T>> it;

        iter() : it{}
        {
        }

        iter(std::unique_ptr<i_iterator<T>> it) : it{ std::move(it) }
        {
        }

        iter(const iter& other) : it{ other.it ? other.it->clone() : nullptr }
        {
        }

        iter(iter&& other) : it{ std::move(other.it) }
        {
        }

        T deref() const
        {
            return it->deref();
        }

        void inc()
        {
            it->inc();
        }

        bool is_equal(const iter& other) const
        {
            return it->is_equal(*other.it);
        }
    };

    template <class Range>
    constexpr iterable_base(Range&& range)
    {
        auto r = all(std::forward<Range>(range));
        impl = std::make_unique<range_wrapper<decltype(r)>>(std::move(r));
    }

    constexpr auto begin() const
    {
        return iterator_interface{ iter{ impl->begin() } };
    }

    constexpr auto end() const
    {
        return iterator_interface{ iter{ impl->end() } };
    }

    std::unique_ptr<i_range<T>> impl;
};

}  // namespace detail

template <class T>
struct iterable : public view_interface<detail::iterable_base<T>>
{
    using base_type = view_interface<detail::iterable_base<T>>;

    template <class Range>
    constexpr iterable(Range&& range) : base_type{ std::forward<Range>(range) }
    {
    }
};

template <class T>
struct is_view_interface<iterable<T>> : std::true_type
{
};

}  // namespace cpp_pipelines
