#pragma once

#include <cpp_pipelines/seq/views.hpp>
#include <memory>

namespace cpp_pipelines
{
template <class T>
struct iterable_base
{
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

    struct i_iterator
    {
        virtual ~i_iterator() = default;
        virtual T deref() const = 0;
        virtual void inc() = 0;
        virtual bool is_equal(const i_iterator& other) const = 0;
        virtual std::unique_ptr<i_iterator> clone() const = 0;
    };

    struct i_range
    {
        virtual ~i_range() = default;
        virtual std::unique_ptr<i_iterator> begin() const = 0;
        virtual std::unique_ptr<i_iterator> end() const = 0;
    };

    template <class Range>
    struct range_wrapper : public i_range
    {
        Range range;

        struct iterator : public i_iterator
        {
            using inner_iter = iterator_t<Range>;
            inner_iter it;

            iterator(inner_iter it)
                : it{ it }
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

            bool is_equal(const i_iterator& other) const override
            {
                return it == static_cast<const iterator&>(other).it;
            }

            std::unique_ptr<i_iterator> clone() const override
            {
                return std::make_unique<iterator>(it);
            }
        };

        range_wrapper(Range range)
            : range{ std::move(range) }
        {
        }

        std::unique_ptr<i_iterator> begin() const override
        {
            return std::make_unique<iterator>(std::begin(range));
        }

        std::unique_ptr<i_iterator> end() const override
        {
            return std::make_unique<iterator>(std::end(range));
        }
    };

    struct iter
    {
        std::unique_ptr<i_iterator> it;

        iter()
            : it{}
        {
        }

        iter(std::unique_ptr<i_iterator> it)
            : it{ std::move(it) }
        {
        }

        iter(const iter& other)
            : it{ other.it ? other.it->clone() : nullptr }
        {
        }

        iter(iter&& other)
            : it{ std::move(other.it) }
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

    std::unique_ptr<i_range> impl;
};

template <class T>
struct iterable : public view_interface<iterable_base<T>>
{
    using base_type = view_interface<iterable_base<T>>;

    template <class Range>
    constexpr iterable(Range&& range)
        : base_type{ std::forward<Range>(range) }
    {
    }
};

template <class T>
struct is_view_interface<iterable<T>> : std::true_type
{
};

}  // namespace cpp_pipelines
