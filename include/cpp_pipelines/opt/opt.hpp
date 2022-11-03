#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <optional>

namespace cpp_pipelines::opt
{
namespace customization
{
template <class T>
struct optional_traits;

template <class T>
struct optional_traits<std::optional<T>>
{
    using type = std::optional<T>;

    constexpr bool has_value(const type& opt) const
    {
        return static_cast<bool>(opt);
    }

    constexpr T& get_value(type& opt) const
    {
        return *opt;
    }

    constexpr const T& get_value(const type& opt) const
    {
        return *opt;
    }

    constexpr T&& get_value(type&& opt) const
    {
        return *std::move(opt);
    }
};

template <class T>
struct optional_traits<std::optional<std::reference_wrapper<T>>>
{
    using type = std::optional<std::reference_wrapper<T>>;

    constexpr bool has_value(const type& opt) const
    {
        return static_cast<bool>(opt);
    }

    constexpr T& get_value(const type& opt) const
    {
        return opt->get();
    }
};

template <class T>
struct optional_traits<T*>
{
    constexpr bool has_value(T* opt) const
    {
        return static_cast<bool>(opt);
    }

    constexpr T& get_value(T* opt) const
    {
        return *opt;
    }
};

}  // namespace customization

namespace detail
{
struct has_value_fn
{
    template <class Opt>
    constexpr bool operator()(Opt&& opt) const
    {
        const auto traits = customization::optional_traits<std::decay_t<Opt>>{};
        return traits.has_value(opt);
    }
};

static constexpr inline auto has_value = make_pipeline(has_value_fn{});

template <class Opt>
constexpr decltype(auto) get_value(Opt&& opt)
{
    const auto traits = customization::optional_traits<std::decay_t<Opt>>{};
    return traits.get_value(std::forward<Opt>(opt));
}
struct lift_fn
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

static constexpr inline auto lift = make_pipeline(lift_fn{});

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

struct lift_if_fn
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return lift |= filter_fn{}(std::move(pred));
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
    template <class Func, class Lift>
    struct impl
    {
        Func func;
        Lift lift;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            using result_type = decltype(invoke(lift, invoke(func, get_value(std::forward<Opt>(opt)))));
            return has_value(opt)
                       ? invoke(lift, invoke(func, get_value(std::forward<Opt>(opt))))
                       : result_type{};
        }
    };

    template <class Func, class Lift = lift_fn>
    constexpr auto operator()(Func func, Lift lift = {}) const
    {
        return make_pipeline(impl<Func, Lift>{ std::move(func), std::move(lift) });
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
                invoke(func, get_value(opt));
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

struct all_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return !has_value(opt) || invoke(pred, get_value(opt));
    }
};

struct any_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return has_value(opt) && invoke(pred, get_value(opt));
    }
};

struct none_of_fn
{
    template <class Pred, class Opt>
    constexpr bool operator()(Pred&& pred, Opt&& opt) const
    {
        return !has_value(opt) || !invoke(pred, get_value(opt));
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

struct for_each_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Opt>
        constexpr void operator()(Opt&& opt) const
        {
            if (has_value(opt))
            {
                invoke(func, get_value(std::forward<Opt>(opt)));
            }
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
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

struct match_fn
{
    template <class OnValue, class Otherwise>
    struct impl
    {
        OnValue on_value;
        Otherwise otherwise;

        template <class Opt>
        constexpr auto operator()(Opt&& opt) const
        {
            return has_value(opt)
                       ? invoke(on_value, get_value(std::forward<Opt>(opt)))
                       : invoke(otherwise);
        }
    };

    template <class OnValue, class Otherwise>
    constexpr auto operator()(OnValue on_value, Otherwise otherwise) const
    {
        return make_pipeline(impl<OnValue, Otherwise>{ std::move(on_value), std::move(otherwise) });
    }
};

struct zip_transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class... Args>
        constexpr auto operator()(Args&&... args) const
        {
            using result_type = decltype(invoke(lift, invoke(func, get_value(std::forward<Args>(args))...)));
            return (... && has_value(args))
                       ? invoke(lift, invoke(func, get_value(std::forward<Args>(args))...))
                       : result_type{};
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

using detail::get_value;
using detail::has_value;

using detail::lift;
static constexpr inline auto some = lift;

static constexpr inline auto lift_if = detail::lift_if_fn{};

static constexpr inline auto filter = detail::filter_fn{};
static constexpr inline auto and_then = detail::and_then_fn{};
static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto or_else = detail::or_else_fn{};
static constexpr inline auto inspect = detail::inspect_fn{};

static constexpr inline auto value_or = detail::value_or_fn{};
static constexpr inline auto value_or_else = detail::value_or_else_fn{};
static constexpr inline auto value_or_throw = detail::value_fn{};
static const inline auto value = value_or_throw();

static constexpr inline auto all_of = detail::check_element_fn<detail::all_of_fn>{};
static constexpr inline auto any_of = detail::check_element_fn<detail::any_of_fn>{};
static constexpr inline auto none_of = detail::check_element_fn<detail::none_of_fn>{};
static constexpr inline auto matches = any_of;

static constexpr inline auto for_each = detail::for_each_fn{};
static constexpr inline auto accumulate = detail::accumulate_fn{};

static constexpr inline auto match = detail::match_fn{};
static constexpr inline auto zip_transform = detail::zip_transform_fn{};

}  // namespace cpp_pipelines::opt
