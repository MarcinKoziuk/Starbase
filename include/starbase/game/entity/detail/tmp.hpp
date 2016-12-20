#pragma once

#include <type_traits>

namespace Starbase {
namespace TMP {

template<class T1, class T2>
struct SameType {
	static const bool value = false;
};

template<class T>
struct SameType<T, T> {
	static const bool value = true;
};

template<int N, typename T, typename Tuple>
struct ContainerOfType : SameType<T, typename std::tuple_element<N, Tuple>::type::value_type>
{};

template<int N, class C, class Tuple, bool Match = false> // this =false is only for clarity
struct MatchingField {
	static typename C& get(Tuple& tp)
	{
		// The "non-matching" version
		return MatchingField<N + 1, C, Tuple,
			ContainerOfType<N + 1, C::value_type, Tuple>::value>::get(tp);
	}
};

template<int N, class C, class Tuple>
struct MatchingField<N, C, Tuple, true> {
	static typename C& get(Tuple& tp)
	{
		return std::get<N>(tp);
	}
};

template<typename C, typename Tuple>
C& GetTupleContainer(Tuple& tuple)
{
	return TMP::MatchingField<0, C, Tuple,
		TMP::ContainerOfType<0, C::value_type, Tuple>::value>::get(tuple);
}

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

template<std::size_t I, typename Tuple, typename Functor, typename = typename std::enable_if<I == std::tuple_size<Tuple>::value>::type >
void ForEach(Functor&, std::integral_constant<size_t, std::tuple_size<typename std::remove_reference<Tuple>::type >::value>) {}

template<std::size_t I, typename Tuple, typename Functor, typename = typename std::enable_if<I != std::tuple_size<Tuple>::value>::type >
void ForEach(Functor& f, std::integral_constant<size_t, I>)
{
	f. operator()<typename std::tuple_element<I, Tuple>::type>();

	ForEach<I + 1, Tuple, Functor>(f, std::integral_constant<size_t, I + 1>());
}

template<typename Tuple, typename Functor>
void ForEach(Functor& f)
{
	ForEach<std::size_t(0), Tuple, Functor>(f, std::integral_constant<size_t, 0>());
}

// overload for single argument
/*template<typename T>
bool And(T&& f) {
	return f();
}

// extract the first parameter then recursively call self for others
template<typename U, typename... T>
bool And(U&& f, T&&... t) {
	// this will short-circuit...
	return f() && And(std::forward<T>(t)...);
}
*/
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

} // namespace TMP
} // namespace Starbase
