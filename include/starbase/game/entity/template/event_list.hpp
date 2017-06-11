#pragma once

#include <functional>

#include <wink/signal.hpp>

#include "detail/tmp.hpp"
#include "entity.hpp"

namespace Starbase {

template<typename ...EventTypes>
struct TEventList {
	static constexpr std::size_t count{ sizeof...(EventTypes) };

	template<typename C>
	static constexpr int indexOf()
	{
		return static_cast<int>(TMP::IndexOf<C, EventTypes...>());
	}

	typedef std::tuple<EventTypes...> types;

	template<typename EventType>
	using signal_type_one = wink::signal<std::function<void(EventType&)>>;

	using signal_type = std::tuple<signal_type_one<EventTypes>...>;
};

} // namespace Starbase
