#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>

#include "detail/tmp.hpp"

namespace Starbase {

typedef std::uint32_t entity_id;

struct Entity {
    entity_id id;
    bool dead : 1;
};

template<typename T>
static int component_index();


template<typename ...ComponentTypes>
class EntityManager {
private:
    std::vector<Entity> m_entities;

	typedef std::tuple<std::vector<ComponentTypes>...> component_grid;
	component_grid m_components;

    // keeps track of each entity's index in m_entities
    std::unordered_map<entity_id, int> m_entityIndex;

	static int s_componentTypeIndexCounter;

	template<typename C>
	static int component_index()
	{
		static int index = s_componentTypeIndexCounter++;
		return index;
	}

public:
    EntityManager()
    {
		int typeindexes[] = { component_index<ComponentTypes>()... };

#ifndef NDEBUG
		// ensure they are sequential and start with zero
		for (int i = 0; i < (sizeof(typeindexes) / sizeof(int)); i++) {
			assert(typeindexes[i] == i);
		}
#else
		(void)typeindexes;
#endif
    }

	template <typename C>
	std::vector<C>& GetComponents()
	{
		return TMP::GetTupleVector<C, component_grid>(m_components);
		/*return TMP::MatchingField<0, C, component_grid,
			TMP::VectorOfType<0, C, component_grid>::value>::get(m_components);*/
	}

	template<typename C>
	C& GetComponent(std::size_t index)
	{
		return GetComponents<C>()[index];
	}
};

template<typename ...ComponentTypes>
int EntityManager<ComponentTypes...>::s_componentTypeIndexCounter = 0;

#include "detail/component_index.inl"

} // namespace Starbase