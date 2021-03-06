#pragma once

#include <utility>
#include <functional>
#include <type_traits>

namespace Starbase {
namespace TMP {

/* -------
 * IndexOf
 * ------- */

template<typename T, typename... Ts>
struct IndexOf;

template<typename T, typename... Ts>
struct IndexOf<T, T, Ts...>
	: std::integral_constant<std::size_t, 0>
{};

template <typename T, typename Tail, typename... Ts>
struct IndexOf<T, Tail, Ts...> :
	std::integral_constant<std::size_t, 1 + IndexOf<T, Ts...>::value>
{};

/* ---
 * And
 * --- */

// overload for single argument
template<typename B>
bool And(B b) {
	return b;
}

// extract the first parameter then recursively call self for others
template<typename B, typename ...O>
bool And(B b, O... o) {
	// this will short-circuit...
	return b && And(o...);
}

/* -------
 * ForEach
 * ------- */
namespace detail {
	template <typename T>
	struct type_holder {
		using type = T;
	};

	template<std::size_t I, typename Tuple, typename Functor, typename = typename std::enable_if<I == std::tuple_size<Tuple>::value>::type >
    void ForEach(Functor&&, std::integral_constant<size_t, std::tuple_size<typename std::remove_reference<Tuple>::type >::value>) {}

	template<std::size_t I, typename Tuple, typename Functor, typename = typename std::enable_if<I != std::tuple_size<Tuple>::value>::type >
    void ForEach(Functor&& f, std::integral_constant<size_t, I>)
	{
        (void) f(type_holder<typename std::tuple_element<I, Tuple>::type>{});
		ForEach<I + 1, Tuple, Functor>(std::forward<Functor>(f), std::integral_constant<size_t, I + 1>());
	}
}

template<typename Tuple, typename Functor>
void ForEach(Functor&& f)
{
	detail::ForEach<std::size_t(0), Tuple, Functor>(std::forward<Functor>(f), std::integral_constant<size_t, 0>());
}

} // namespace TMP
} // namespace Starbase
