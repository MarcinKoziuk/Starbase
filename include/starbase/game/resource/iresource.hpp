#pragma once

#include <cstddef>
#include <cassert>
#include <numeric>

namespace Starbase {

class IResource {
public:
    virtual ~IResource() {}

    virtual std::size_t CalculateSize() const = 0;

    virtual const char* GetResourceName() const = 0;
};

template<typename T>
std::size_t PodContainerSize(const T& container)
{
	return sizeof(typename T::value_type) * container.size();
}

template<typename T>
std::size_t PodContainerContainerSize(const T& container)
{
	return std::accumulate(
		container.begin(),
		container.end(),
		std::size_t(0L),
		[](std::size_t v, const typename T::value_type& e) {
			return v + PodContainerSize(e);
		}
	);
}

template<typename T>
std::size_t SizeAwareContainerSize(const T& container)
{
	return std::accumulate(
		container.begin(),
		container.end(),
		std::size_t(0L),
		[](std::size_t v, const typename T::value_type& e) {
			return v + e.CalculateSize();
		}
	);
}

} // namespace Starbase
