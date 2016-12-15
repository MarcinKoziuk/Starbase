#pragma once

namespace Starbase {
namespace TMP {

template <class T1, class T2>
struct SameType {
	static const bool value = false;
};

template<class T>
struct SameType<T, T> {
	static const bool value = true;
};

template<int N, typename T, typename Tuple>
struct VectorOfType : SameType<T, typename std::tuple_element<N, Tuple>::type::value_type>
{};

template <int N, class T, class Tuple, bool Match = false> // this =false is only for clarity
struct MatchingField {
	static std::vector<T>& get(Tuple& tp)
	{
		// The "non-matching" version
		return MatchingField<N + 1, T, Tuple,
			VectorOfType<N + 1, T, Tuple>::value>::get(tp);
	}
};

template <int N, class T, class Tuple>
struct MatchingField<N, T, Tuple, true> {
	static std::vector<T>& get(Tuple& tp)
	{
		return std::get<N>(tp);
	}
};

template <typename T, typename Tuple>
std::vector<T>& GetTupleVector(Tuple& tuple)
{
	return TMP::MatchingField<0, T, Tuple,
		TMP::VectorOfType<0, T, Tuple>::value>::get(tuple);
}

} // namespace TMP
} // namespace Starbase