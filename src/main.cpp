#include <algorithm>
#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/format.hpp>
#include <cpp_pipelines/seq.hpp>
#include <cpp_pipelines/tpl.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <vector>

struct stop_iteration_t
{
};

static constexpr inline stop_iteration_t stop_iteration;

template <class T>
struct iteration_result
{
    using reference = T;
    struct pointer
    {
        T value;

        T* operator->()
        {
            return &value;
        }
    };

    std::optional<T> value_;

    template <class U>
    iteration_result(U&& value) : value_{ std::forward<U>(value) }
    {
    }

    iteration_result(stop_iteration_t) : value_{}
    {
    }

    explicit operator bool() const
    {
        return static_cast<bool>(value_);
    }

    reference operator*() const&
    {
        return *value_;
    }

    reference operator*() &&
    {
        return *std::move(value_);
    }

    pointer operator->() const&
    {
        return pointer{ **this };
    }

    pointer operator->() &&
    {
        return pointer{ *std::move(*this) };
    }
};

template <class T>
struct iteration_result<T&>
{
    using reference = T&;
    using pointer = T*;

    T* value_;

    iteration_result(T& value) : value_{ &value }
    {
    }

    iteration_result(stop_iteration_t) : value_{}
    {
    }

    explicit operator bool() const
    {
        return static_cast<bool>(value_);
    }

    reference operator*() const
    {
        return *value_;
    }

    pointer operator->() const
    {
        return value_;
    }
};

template <class T>
struct iterable
{
    using next_fn = std::function<iteration_result<T>()>;

    struct iterator
    {
        using reference = typename iteration_result<T>::reference;
        using pointer = typename iteration_result<T>::pointer;
        using value_type = typename std::decay<reference>::type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        next_fn next_;
        iteration_result<T> current_;
        difference_type index_;

        explicit iterator(next_fn next) : next_{ std::move(next) }, current_{ next_() }, index_{ 0 }
        {
        }

        iterator() : next_{}, current_{ stop_iteration }, index_{ std::numeric_limits<difference_type>::max() }
        {
        }

        iterator(const iterator&) = default;
        iterator(iterator&&) = default;

        iterator& operator=(iterator other)
        {
            std::swap(next_, other.next_);
            std::swap(current_, other.current_);
            std::swap(index_, other.index_);
            return *this;
        }

        reference operator*() const&
        {
            return current_.operator*();
        }

        reference operator*() &&
        {
            return std::move(current_).operator*();
        }

        pointer operator->() const&
        {
            return current_.operator->();
        }

        pointer operator->() &&
        {
            return std::move(current_).operator->();
        }

        iterator& operator++()
        {
            current_ = next_();
            ++index_;
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp{ *this };
            ++(*this);
            return temp;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
        {
            return (!static_cast<bool>(lhs.current_) && !static_cast<bool>(rhs.current_))
                   || (static_cast<bool>(lhs.current_) && static_cast<bool>(rhs.current_) && lhs.index_ == rhs.index_);
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
        {
            return !(lhs == rhs);
        }
    };

    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using pointer = typename iterator::pointer;

    using const_reference = reference;
    using const_pointer = pointer;
    using const_iterator = iterator;

    next_fn next_;

    explicit iterable(next_fn next) : next_{ std::move(next) }
    {
    }

    iterable()
        : iterable([]() -> iteration_result<T> { return stop_iteration; })
    {
    }

    iterable(const iterable&) = default;
    iterable(iterable&&) = default;

    const_iterator begin() const
    {
        return const_iterator{ next_ };
    }

    const_iterator end() const
    {
        return const_iterator{};
    }
};

template <class T, class S>
iterable<T> unfold(S state, const std::function<iteration_result<std::tuple<T, S>>(const S&)>& func)
{
    auto next = [=]() mutable -> iteration_result<T>
    {
        auto result = func(state);
        if (!result)
        {
            return stop_iteration;
        }
        state = std::get<1>(*result);
        return iteration_result<T>{ std::get<0>(*result) };
    };
    return iterable<T>{ std::move(next) };
}

template <class T>
void print(iterable<T> range)
{
    for (const auto& item : range)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl << "---" << std::endl;
}

template <class T>
iterable<const T&> as_iterable(const std::vector<T>& vect)
{
    return unfold<const T&, typename std::vector<T>::const_iterator>(
        std::begin(vect),
        [&](typename std::vector<T>::const_iterator n)
            -> iteration_result<std::tuple<const T&, typename std::vector<T>::const_iterator>>
        {
            if (n == std::end(vect))
            {
                return stop_iteration;
            }
            return std::tuple<const T&, typename std::vector<T>::const_iterator>{ *n, std::next(n) };
        });
}

iterable<int> fibonacci(int n)
{
    using State = std::tuple<int, int>;
    return unfold<int, State>(
        State{ 0, 1 },
        [=](const State& state) -> iteration_result<std::tuple<int, State>>
        {
            const int prev = std::get<0>(state);
            const int current = std::get<1>(state);

            if (prev > n)
            {
                return stop_iteration;
            }

            const int next = prev + current;

            return std::tuple<int, State>{ prev, State{ current, next } };
        });
}

iterable<int> collatz(int n)
{
    using State = std::tuple<int, int>;
    return unfold<int, State>(
        State{ n, n },
        [](const State& state) -> iteration_result<std::tuple<int, State>>
        {
            const int prev = std::get<0>(state);
            const int current = std::get<1>(state);

            if (prev == 1)
            {
                return stop_iteration;
            }

            const int next = current % 2 == 0 ? current / 2 : 3 * current + 1;

            return std::tuple<int, State>{ current, State{ current, next } };
        });
}

void run()
{
    std::vector<int> vect{ 2, 3, 5, 7, 11, 13, 17 };
    print(as_iterable(vect));
    print(fibonacci(100));
    print(collatz(12));
}

int main()
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "exception: " << e.what() << '\n';
    }
}
