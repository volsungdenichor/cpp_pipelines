#pragma once
#include <cpp_pipelines/pipeline.hpp>
#include <optional>

namespace cpp_pipelines::opt
{
namespace customization
{
template <class T>
struct optional_traits
{
    template <class Opt>
    constexpr bool has_value(Opt&& opt) const
    {
        return static_cast<bool>(opt);
    }

    template <class Opt>
    constexpr decltype(auto) get_value(Opt&& opt) const
    {
        return *std::forward<Opt>(opt);
    }
};

};  // namespace customization

struct to_optional_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const
    {
        return std::optional{ std::forward<T>(item) };
    }

    template <class T>
    constexpr auto operator()(T& item) const
    {
        return std::optional{ std::ref(item) };
    }
};

template <class Opt>
constexpr bool has_value(Opt&& opt)
{
    const auto traits = customization::optional_traits<std::decay_t<Opt>>{};
    return traits.has_value(opt);
}

template <class Opt>
constexpr decltype(auto) get_value(Opt&& opt)
{
    const auto traits = customization::optional_traits<std::decay_t<Opt>>{};
    return traits.get_value(std::forward<Opt>(opt));
}

struct filter_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            return has_value(opt) && invoke(pred, get_value(opt))
                       ? std::forward<Opt>(opt)
                       : std::decay_t<Opt>{};
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

struct and_then_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            using result_type = decltype(invoke(func, get_value(std::forward<Opt>(opt))));
            return has_value(opt)
                       ? invoke(func, get_value(std::forward<Opt>(opt)))
                       : result_type{};
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct transform_fn
{
    template <class Func, class ToOptional>
    struct impl
    {
        Func func;
        ToOptional to_optional;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            using result_type = decltype(invoke(to_optional, invoke(func, get_value(std::forward<Opt>(opt)))));
            return has_value(opt)
                       ? invoke(to_optional, invoke(func, get_value(std::forward<Opt>(opt))))
                       : result_type{};
        }
    };

    template <class Func, class ToOptional = to_optional_fn>
    constexpr auto operator()(Func func, ToOptional to_optional = {}) const
    {
        return make_pipeline(impl<Func, ToOptional>{ std::move(func), std::move(to_optional) });
    }
};

struct or_else_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            using result_type = decltype(std::invoke(func));
            if constexpr (std::is_void_v<result_type>)
            {
                return has_value(opt)
                           ? std::forward<Opt>(opt)
                           : (std::invoke(func), std::decay_t<Opt>{});
            }
            else
            {
                return has_value(opt)
                           ? std::forward<Opt>(opt)
                           : std::invoke(func);
            }
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct inspect_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            if (has_value(opt))
            {
                invoke(get_value(opt));
            }
            return std::forward<Opt>(opt);
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct value_or_fn
{
    template <class T>
    struct impl
    {
        T default_value;

        template <class Opt>
        constexpr decltype(auto) operator()(Opt&& opt) const
        {
            return to_return_type(has_value(opt) ? get_value(std::forward<Opt>(opt)) : default_value);
        }
    };

    template <class T>
    constexpr auto operator()(T default_value) const
    {
        return make_pipeline(impl<T>{ std::move(default_value) });
    }
};

struct value_or_else_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Opt>
        constexpr decltype(auto) operator()(Opt&& opt) const
        {
            return to_return_type(has_value(opt) ? get_value(std::forward<Opt>(opt)) : std::invoke(func));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct value_fn
{
    template <class Exception>
    struct impl
    {
        Exception ex;

        template <class Opt>
        constexpr decltype(auto) operator()(Opt&& opt) const
        {
            if (!has_value(opt))
            {
                throw ex;
            }
            return to_return_type(get_value(std::forward<Opt>(opt)));
        }
    };

    template <class Exception = std::bad_optional_access>
    constexpr auto operator()(Exception exception = {}) const
    {
        if constexpr (std::is_base_of_v<std::exception, Exception>)
        {
            return make_pipeline(impl<Exception>{ std::move(exception) });
        }
        else if constexpr (std::is_constructible_v<std::string, Exception>)
        {
            return (*this)(std::runtime_error{ std::move(exception) });
        }
    }
};

template <class T, class U>
using equality_comparable_impl = decltype(std::declval<T&>() == std::declval<U&>());

template <class Pred, class T>
constexpr bool check(const Pred& pred, const T& value)
{
    if constexpr (is_detected_v<equality_comparable_impl, T, Pred>)
    {
        return value == pred;
    }
    else
    {
        return invoke(pred, value);
    }
}

struct all_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return !has_value(opt) || check(pred, get_value(opt));
    }
};

struct any_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return has_value(opt) && check(pred, get_value(opt));
    }
};

struct none_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return !has_value(opt) || !check(pred, get_value(opt));
    }
};

template <class Policy>
struct check_element_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Opt>
        constexpr bool operator()(Opt&& opt) const
        {
            const auto policy = Policy{};
            return policy(pred, opt);
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

struct accumulate_fn
{
    template <class BinaryFunc, class Init>
    struct impl
    {
        BinaryFunc func;
        Init init;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            return has_value(opt)
                       ? invoke(init, get_value(std::forward<Opt>(opt)))
                       : init;
        }
    };

    template <class BinaryFunc, class Init>
    constexpr auto operator()(BinaryFunc func, Init init) const
    {
        return make_pipeline(impl<BinaryFunc, Init>{ std::move(func), std::move(init) });
    }
};

static constexpr inline auto filter = filter_fn{};
static constexpr inline auto and_then = and_then_fn{};
static constexpr inline auto transform = transform_fn{};
static constexpr inline auto or_else = or_else_fn{};
static constexpr inline auto inspect = inspect_fn{};

static constexpr inline auto value_or = value_or_fn{};
static constexpr inline auto value_or_else = value_or_else_fn{};
static constexpr inline auto value = value_fn{};

static constexpr inline auto all_of = check_element_fn<all_of_fn>{};
static constexpr inline auto any_of = check_element_fn<any_of_fn>{};
static constexpr inline auto none_of = check_element_fn<none_of_fn>{};
static constexpr inline auto matches = any_of;

static constexpr inline auto accumulate = accumulate_fn{};

}  // namespace cpp_pipelines::opt