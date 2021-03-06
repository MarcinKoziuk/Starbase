#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <functional>

#include <wink/signal.hpp>

#include "entity.hpp"
#include "detail/tmp.hpp"

namespace Starbase {

template<typename ...ComponentTypes>
struct TComponentList {
	static constexpr std::size_t count{ sizeof...(ComponentTypes) };

	template<typename C>
	static constexpr int indexOf()
	{
		return static_cast<int>(TMP::IndexOf<C, ComponentTypes...>());
	}

	typedef std::tuple<ComponentTypes...> types;

	typedef std::tuple<std::vector<ComponentTypes>...> vector_type;
	typedef std::tuple<std::map<entity_id, ComponentTypes>...> map_type;
	typedef std::tuple<std::vector<ComponentTypes*>...> freelist_type;
	typedef std::vector<std::unordered_map<entity_id, int>> index_type;

    template<typename EntityType, typename C>
	using signal_type_one = wink::signal<std::function<void(EntityType&, C&)>>;

	template<typename EntityType>
    using signal_type = std::tuple<signal_type_one<EntityType, ComponentTypes>...>;
};

} // namespace Starbase
