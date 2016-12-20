#pragma once

#include <vector>

namespace Starbase {

template<typename T>
class BasicHandle {
private:
	int index;
	std::vector<T>& vec;

	template<typename>
	friend class TEntityManager;

	BasicHandle(int index, std::vector<T>& vec)
		: index(index)
		, vec(vec)
	{}

public:
	T& operator*()
	{
		return vec[index];
	}

	const T& operator*() const
	{
		return vec[index];
	}

	T* operator->()
	{
		return &vec[index];
	}

	const T* operator->() const
	{
		return &vec[index];
	}
};

} // namespace Starbase
