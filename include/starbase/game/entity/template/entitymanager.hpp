#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>

#include <boost/signals2.hpp>

#include <starbase/starbase.hpp>

#include "component_list.hpp"
#include "entity.hpp"

namespace Starbase {

template<typename CL>
class TEntityManager {
public:
    using Entity = TEntity<CL>;
	using component_bitset = typename TEntity<CL>::component_bitset;

private:
	entity_id m_entityIdCounter;

	// The list of entities that systems iterate over,
	// unused spots are marked with active=false
	std::vector<Entity> m_entities;

	// Newly added (non-indexed) entities. Added to m_entities after Refresh()
	std::map<entity_id, Entity> m_entitiesNew;

	// Pointers to keep track of empty spots in m_entities (active=false)
	std::vector<Entity*> m_entitiesFree;

	// For each entity_id, its index in the m_entities vector (except new entities)
	std::unordered_map<entity_id, int> m_entitiesIndex;

	// same principle goes here, except that components don't have an active flag
	typename CL::vector_type m_components;
	typename CL::map_type m_componentsNew;
	typename CL::freelist_type m_componentsFree;
	typename CL::index_type m_componentsIndex;

	entity_id GenerateId();

	template<typename C>
	std::vector<C>& GetComponents();

	template<typename C>
	std::map<entity_id, C>& GetComponentsNew();

	template<typename C>
	std::vector<C*>& GetComponentsFree();

	template<typename C>
	std::unordered_map<entity_id, int>& GetComponentsIndex();

	int IndexOfEntity(Entity* pEnt);

	template<typename C>
	int IndexOfComponent(C* pCom);

	template<typename C>
	C& GetComponentExistingImpl(entity_id id);

	template<typename C>
	C& GetComponentNewImpl(entity_id id);

	template<typename C, typename... Args>
	C& AddComponentExistingImpl(entity_id id, Args&&... args);

	template<typename C, typename... Args>
	C& AddComponentNewImpl(entity_id id, Args&&... args);

	template<typename C>
	C& AddComponentNewAndSetBitImpl(Entity& ent);

	template<typename C>
	void RemoveComponentNewImpl(entity_id id);

	template<typename C>
	void RemoveComponentExistingImpl(entity_id id);

	void RemoveComponentsNewIfBitIsSetImpl(Entity& ent);

	void RemoveComponentsExistingIfBitIsSetImpl(Entity& ent);

	void RemoveEntityExistingImpl(Entity& ent);

	void RemoveEntityNewImpl(Entity& ent);

public:
	boost::signals2::signal<void(Entity& entity)> entityAdded;
	boost::signals2::signal<void(Entity& entity)> entityWillBeRemoved;
	boost::signals2::signal<void(Entity& entity, component_bitset oldComponents)> componentAdded;
	boost::signals2::signal<void(Entity& entity, component_bitset newComponents)> componentWillBeRemoved;

	TEntityManager();

	Entity& GetEntity(entity_id id);

    template<typename C>
	C& GetComponent(const Entity& ent);

	template<typename F>
	void ForEachEntity(F fun);

	template<typename ...Cs, typename F>
	void ForEachEntityWithComponents(F fun);

	Entity& CreateEntity();

	template<typename ...Cs>
	std::tuple<Entity&, Cs&...> CreateEntity();

	void RemoveEntity(Entity& ent);

	template<typename C, typename... Args>
	C& AddComponent(Entity& ent, Args&&... args);

	template<typename C>
	void RemoveComponent(Entity& ent);

	void Refresh();
};

} // namespace Starbase

#include "detail/entitymanager.inl"
