#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>

#include <wink/signal.hpp>

#include <starbase/starbase.hpp>

#include "component_list.hpp"
#include "entity.hpp"
#include "eventmanager.hpp"

namespace Starbase {

template<typename CL>
class TEntityManager {
public:
    using Entity = TEntity<CL>;

	using component_bitset = typename TEntity<CL>::component_bitset;
	/*
	using component_signals = typename CL::template signal_type<Entity>;

	template<typename C>
	using component_signal = typename CL::template signal_type_one<Entity, C>;*/

	friend Entity;

private:
	using entity_added = typename TEventManagerBase<CL>::entity_added;
	using entity_removed = typename TEventManagerBase<CL>::entity_removed;
	using component_added = typename TEventManagerBase<CL>::component_added;
	using component_removed = typename TEventManagerBase<CL>::component_removed;

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

	TEventManagerBase<CL>& m_eventManager;

	entity_id GenerateId();

	template<typename C>
	std::vector<C>& GetComponents();

	template<typename C>
	std::map<entity_id, C>& GetComponentsNew();

	template<typename C>
	std::vector<C*>& GetComponentsFree();

	template<typename C>
	std::unordered_map<entity_id, int>& GetComponentsIndex();
	/*
	template<typename C>
    component_signal<C>& GetComponentAddedSignal();

	template<typename C>
	component_signal<C>& GetComponentRemovedSignal();

	template<typename C>
	void EmitComponentAddedSignal(Entity&, C&);

	template<typename C>
	void EmitComponentRemovedSignal(Entity&, C&);*/

	template<typename C>
	C& GetComponentExistingImpl(entity_id id);

	template<typename C>
	C& GetComponentNewImpl(entity_id id);

	template<typename C>
	C& GetComponent(const Entity& ent);

	template<typename C, typename... Args>
	C& AddComponentExistingImpl(Entity& entity, Args&&... args);

	template<typename C, typename... Args>
	C& AddComponentNewImpl(Entity& entity, Args&&... args);
	
	template<typename C>
	void RemoveComponentNewImpl(Entity& entity);

	template<typename C>
	void RemoveComponentExistingImpl(Entity& entity);

	void RemoveComponentsNew(Entity& ent);

	void RemoveComponentsExisting(Entity& ent);

	void RemoveEntityExistingImpl(Entity& ent);

	void RemoveEntityNewImpl(Entity& ent);

	template<typename C, typename... Args>
	C& AddComponent(Entity& ent, Args&&... args);

	template<typename C>
	void RemoveComponent(Entity& ent);

public:
	/*component_signals componentAdded2;
	component_signals componentRemoved2;
	wink::signal<std::function<void(Entity& entity)>> entityAdded;
	wink::signal<std::function<void(Entity& entity)>> entityWillBeRemoved;*/
	wink::signal<std::function<void(Entity& entity, component_bitset oldComponents)>> componentAdded;
	wink::signal<std::function<void(Entity& entity, component_bitset newComponents)>> componentWillBeRemoved;

	TEntityManager(TEventManagerBase<CL>& eventManager);

	Entity& GetEntity(entity_id id);

	template<typename F>
	void ForEachEntity(F fun);

	template<typename ...Cs, typename F>
	void ForEachEntityWithComponents(F fun);

	Entity& CreateEntity();

	template<typename ...Cs>
	std::tuple<Entity&, Cs&...> CreateEntity();

	template<typename ...Cs>
	Entity& CreateEntity(Cs&&...);

	void RemoveEntity(Entity& ent);

	void Update();
	/*
	// Due to a MSVC compiler issue, these can not have a separate definition in entitymanager.inl (error C2244)
	template<typename E, typename std::enable_if<!std::is_base_of<entity_event, E>::value, entity_event>::type* = nullptr, typename... Args>
	void Connect(Args&&...)
	{
		assert(false);
	}

	template<typename E, typename std::enable_if<std::is_base_of<entity_added, E>::value, entity_added>::type* = nullptr, typename... Args>
	void Connect(Args&&... args)
	{
		entityAdded.connect(std::forward<Args>(args)...);
	}

	template<typename E, typename std::enable_if<std::is_base_of<entity_removed, E>::value, entity_removed>::type* = nullptr, typename... Args>
	void Connect(Args&&... args)
	{
        entityWillBeRemoved.connect(std::forward<Args>(args)...);
	}
	*/
};

} // namespace Starbase

#include "detail/entitymanager.inl"
