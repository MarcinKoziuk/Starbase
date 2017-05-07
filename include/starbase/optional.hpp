#pragma once

#include <optional/optional.hpp>

namespace Starbase {

template<typename T>
using optional = std::experimental::optional<T>;

constexpr auto nullopt = std::experimental::nullopt;

} // namespace Starbase
