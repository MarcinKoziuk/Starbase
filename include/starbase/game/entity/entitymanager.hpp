#pragma once

#include <cstddef>
#include <limits>
#include <vector>
#include <map>
#include <unordered_map>
#include <tuple>
#include <algorithm>
#include <functional>

#include <boost/signals2.hpp>

#include <starbase/game/logging.hpp>

#include "entity.hpp"
#include "detail/tmp.hpp"

namespace Starbase {

template<typename ...ComponentTypes>
class EParams {
public:
	static constexpr std::size_t componentCount{ sizeof...(ComponentTypes) };

	template<typename C>
	static constexpr int componentIndex() { return (int)TMP::IndexOf<C, ComponentTypes...>(); }


	typedef std::tuple<ComponentTypes...> component_types;

	typedef std::tuple<std::vector<ComponentTypes>...> component_vectors;
	typedef std::tuple<std::map<entity_id, ComponentTypes>...> component_maps;
	typedef std::tuple<std::vector<ComponentTypes*>...> component_freelists;
	typedef std::vector<std::unordered_map<entity_id, int>> component_indexes;
};

template<typename EParams>
class TEntityManager {
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
	typename EParams::component_vectors m_components;
	typename EParams::component_maps m_componentsNew;
	typename EParams::component_freelists m_componentsFree;
	typename EParams::component_indexes m_componentsIndex;

	entity_id GenerateId()
	{
		return ++m_entityIdCounter;
	}
	
	template<typename C>
	std::vector<C>& GetComponents()
	{
		return TMP::GetTupleContainer<std::vector<C>, EParams::component_vectors>(m_components);
	}

	template<typename C>
	std::map<entity_id, C>& GetComponentsNew()
	{
		return TMP::GetTupleContainer<std::map<entity_id, C>, EParams::component_maps>(m_componentsNew);
	}

	template<typename C>
	std::vector<C*>& GetComponentsFree()
	{
		return TMP::GetTupleContainer<std::vector<C*>, EParams::component_freelists>(m_componentsFree);
	}

	template<typename C>
	std::unordered_map<entity_id, int>& GetComponentsIndex()
	{
		return m_componentsIndex[EParams::componentIndex<C>()];
	}

	int IndexOfEntity(Entity* pEnt)
	{
		const std::size_t indexOfEntity = pEnt - &m_entities.front();
		assert(indexOfEntity < std::numeric_limits<int>::max());
		return (int)indexOfEntity;
	}

	template<typename C>
	int IndexOfComponent(C* pCom)
	{
		std::vector<C>& components = GetComponents<C>();
		const std::size_t indexOfComponent = pCom - &components.front();

		assert(indexOfComponent < std::numeric_limits<int>::max());
		return (int)indexOfComponent;
	}

	template<typename C>
	void SetBit(Entity& ent, bool val)
	{
		ent.bitset[EParams::componentIndex<C>()] = val;
	}

	template<typename C>
	void SetBit(Entity::component_bitset& bitset, bool val)
	{
		bitset[EParams::componentIndex<C>()] = val;
	}

	template<typename C>
	bool GetBit(const Entity& ent) const
	{
		return ent.bitset[EParams::componentIndex<C>()];
	}

	template<typename C>
	bool GetBit(const Entity::component_bitset& bitset) const
	{
		return bitset[EParams::componentIndex<C>()];
	}

	template<typename C>
	C& GetComponentExistingImpl(entity_id id)
	{
		auto& components = GetComponents<C>();
		auto& componentsIndex = GetComponentsIndex<C>();

		assert(componentsIndex.count(id));
		return components.at(componentsIndex.at(id));
	}

	template<typename C>
	C& GetComponentNewImpl(entity_id id)
	{
		auto& componentsNew = GetComponentsNew<C>();

		assert(componentsNew.count(id));
		return componentsNew.at(id);
	}

	template<typename C>
	C& AddComponentExistingImpl(entity_id id)
	{
		auto& components = GetComponents<C>();
		auto& componentsIndex = GetComponentsIndex<C>();

		components.emplace_back(C{});
		C* pcom = &components.back();

		componentsIndex[id] = IndexOfComponent<C>(pcom);
		return *pcom;
	}

	template<typename C>
	C& AddComponentNewImpl(entity_id id)
	{
		auto& componentsNew = GetComponentsNew<C>();
		C& com = componentsNew.emplace(id, C{}).first->second;
		return com;
	}

	template<typename C>
	C& AddComponentNewAndSetBitImpl(Entity& ent)
	{
		SetBit<C>(ent, true);
		return AddComponentNewImpl<C>(ent.id);
	}

	template<typename C>
	void RemoveComponentNewImpl(entity_id id)
	{
		auto& componentsNew = GetComponentsNew<C>();
		componentsNew.erase(id);
	}

	template<typename C>
	void RemoveComponentExistingImpl(entity_id id)
	{
		auto& components = GetComponents<C>();
		auto& componentsFree = GetComponentsFree<C>();
		auto& componentsIndex = GetComponentsIndex<C>();

		C& com = components[componentsIndex[id]];
		com = C{}; // clearing not necessary, but handy for debugging

		componentsFree.push_back(&com);
		componentsIndex.erase(id);
	}

	template<typename C>
	void RemoveComponentNewIfBitIsSetImpl(Entity& ent)
	{
		if (GetBit<C>(ent))
			RemoveComponentNewImpl<C>(ent.id);
	}

	template<typename ThisRef>
	struct RemoveComponentNewIfBitIsSetFn {
		ThisRef* em;
		Entity& ent;

		RemoveComponentNewIfBitIsSetFn(ThisRef* em, Entity& ent) : em(em), ent(ent) {}

		template<typename C>
		void operator()() {
			em->RemoveComponentNewIfBitIsSetImpl<C>(ent);
		}
	};

	void RemoveComponentsNewIfBitIsSetImpl(Entity& ent)
	{
		typedef RemoveComponentNewIfBitIsSetFn<TEntityManager<EParams>> Functor;
		Functor f(this, ent);
		TMP::ForEach<EParams::component_types, Functor>(f);
	}

	template<typename C>
	void RemoveComponentExistingIfBitIsSetImpl(Entity& ent)
	{
		if (GetBit<C>(ent))
			RemoveComponentExistingImpl<C>(ent.id);
	}

	template<typename ThisRef>
	struct RemoveComponentExistingIfBitIsSetFn {
		ThisRef* em;
		Entity& ent;

		RemoveComponentExistingIfBitIsSetFn(ThisRef* em, Entity& ent) : em(em), ent(ent) {}

		template<typename C>
		void operator()() {
			em->RemoveComponentExistingIfBitIsSetImpl<C>(ent);
		}
	};

	void RemoveComponentsExistingIfBitIsSetImpl(Entity& ent)
	{
		typedef RemoveComponentExistingIfBitIsSetFn<TEntityManager<EParams>> Functor;
		Functor f(this, ent);
		TMP::ForEach<EParams::component_types, Functor>(f);
	}

	void RemoveEntityExistingImpl(Entity& ent)
	{
		RemoveComponentsExistingIfBitIsSetImpl(ent);

		m_entitiesIndex.erase(ent.id);

		ent = Entity(); // clearing not necessary, but handy for debugging
		ent.alive = false;

		m_entitiesFree.push_back(&ent);
	}

	void RemoveEntityNewImpl(Entity& ent)
	{
		RemoveComponentsNewIfBitIsSetImpl(ent);

		m_entitiesNew.erase(ent.id);
	}

	template<typename ThisRef>
	struct MoveComponentNewIfBitIsSetFn {
		ThisRef* em;
		Entity& ent;

		MoveComponentNewIfBitIsSetFn(ThisRef* em, Entity& ent) : em(em), ent(ent) {}

		template<typename C>
		void operator()() {
			auto& components = em->GetComponents<C>();
			auto& componentsNew = em->GetComponentsNew<C>();
			auto& componentsIndex = em->GetComponentsIndex<C>();

			if (em->GetBit<C>(ent)) {
				C com = componentsNew[ent.id];
				componentsNew.erase(ent.id);

				components.emplace_back(std::move(com));

				C* pCom = &components.back();
				componentsIndex[ent.id] = em->IndexOfComponent(pCom);
			}
		}
	};

public:
	boost::signals2::signal<void (Entity& entity)> entityAdded;
	boost::signals2::signal<void (Entity& entity)> entityWillBeRemoved;
	boost::signals2::signal<void (Entity& entity, Entity::component_bitset oldComponents)> componentAdded;
	boost::signals2::signal<void (Entity& entity, Entity::component_bitset newComponents)> componentWillBeRemoved;

	TEntityManager()
		: m_entityIdCounter(0)
	{
		static_assert(EParams::componentCount <= MAX_COMPONENTS, "MAX_COMPONENTS size is unsufficient!");
		m_componentsIndex.resize(EParams::componentCount);
	}

	Entity& GetEntity(entity_id id)
	{
		return m_entities[m_entitiesIndex[id]];
	}

	template<typename C>
	bool HasComponent(const Entity& ent) const
	{
		return GetBit<C>(ent);
	}

	template<typename ...Cs>
	bool HasComponents(const Entity& ent) const
	{
		return TMP::And(HasComponent<Cs>(ent)...);
	}

	template<typename C>
	bool HasComponent(const Entity::component_bitset& bitset) const
	{
		return GetBit<C>(bitset);
	}
	
	template<typename ...Cs>
	bool HasComponents(const Entity::component_bitset& bitset) const
	{
		return TMP::And(HasComponent<Cs>(bitset)...);
	}

	template<typename C>
	C& GetComponent(const Entity& ent)
	{
		if (SB_LIKELY(!ent.isnew)) {
			return GetComponentExistingImpl<C>(ent.id);
		}
		else {
			LOG(warning) << "Performance warning: don't call GetComponent on newly constructed entities!";
			return GetComponentNewImpl<C>(ent.id);
		}
	}

	template<typename F>
	void ForEachEntity(F& lambda)
	{
		for (Entity& ent : m_entities) {
			if (SB_LIKELY(ent.alive)) {
				lambda(ent);
			}
		}
	}

	template<typename ...Cs, typename F>
	void ForEachEntityWithComponents(F& lambda)
	{
		for (Entity& ent : m_entities) {
			if (SB_LIKELY(ent.alive)) {
				if (HasComponents<Cs...>(ent)) {
					lambda(ent, std::forward<Cs>(GetComponent<Cs>(ent))...);
				}
			}
		}
	}

	Entity& CreateEntity()
	{
		entity_id id = GenerateId();
		return m_entitiesNew.emplace(id, id).first->second;
	}

	template<typename ...Cs>
	std::tuple<Entity&, Cs&...> CreateEntity()
	{
		Entity& ent = CreateEntity();
		return std::forward_as_tuple(ent, AddComponentNewAndSetBitImpl<Cs>(ent)...);
	}

	void RemoveEntity(Entity& ent)
	{
		if (SB_LIKELY(!ent.isnew)) {
			// send signal (not necessary for new ones, because added signal had not been sent)
			entityWillBeRemoved(ent);

			RemoveEntityExistingImpl(ent);
		}
		else {
			LOG(warning) << "Performance warning: deleting entity inserted within the same frame: " << ent.id;
			RemoveEntityNewImpl(ent);
		}
	}

	template<typename C>
	C& AddComponent(Entity& ent)
	{
		C* pCom = nullptr;

		if (SB_LIKELY(!ent.isnew)) {
			pCom = &AddComponentExistingImpl<C>(ent.id);

			const auto oldComponents = ent.bitset;
			SetBit<C>(ent, true);

			// send signal
			componentAdded(ent, oldComponents);
		}
		else {
			LOG(warning) << "Performance warning: components for new entities should be directly inserted on-construction: " << ent.id;
			
			pCom = &AddComponentNewImpl<C>(ent.id);
			
			SetBit<C>(ent, true);
		}

		return *pCom;
	}

	template<typename C>
	void RemoveComponent(Entity& ent)
	{
		if (SB_LIKELY(GetBit<C>(ent))) {
			if (SB_LIKELY(!ent.isnew)) {
				auto newComponents = ent.bitset;
				SetBit<C>(newComponents, false);

				// send signal
				componentWillBeRemoved(ent, newComponents);

				RemoveComponentExistingImpl<C>(ent.id);
				ent.bitset = newComponents;
			}
			else {
				LOG(warning) << "Performance warning: components for new entities should not be removed in the same loop! " << ent.id;
				
				RemoveComponentNewImpl<C>(ent.id);

				SetBit<C>(ent, false);
			}
		}
		else {
			LOG(warning) << "Entity " << ent.id << " does not have component " << componentIndex << ", so cannot remove!";
		}
	}

	void Refresh()
	{
		while (!m_entitiesNew.empty()) {
			// Make copy of the new entity and remove it
			auto& iter = m_entitiesNew.begin();
			Entity ent = iter->second;
			m_entitiesNew.erase(iter);

			// Push it to the main vector
			ent.isnew = false;
			m_entities.emplace_back(std::move(ent));

			// Set its index in the index ;)
			Entity* pEnt = &m_entities.back();
			m_entitiesIndex[pEnt->id] = IndexOfEntity(pEnt);

			// Do the same for its belonging components
			typedef MoveComponentNewIfBitIsSetFn<TEntityManager<EParams>> Functor;
			Functor f(this, ent);
			TMP::ForEach<EParams::component_types, Functor>(f);

			// Send signal
			entityAdded(*pEnt);
		}

		for (Entity& ent : m_entities) {
			if (ent.needsToDie) {
				RemoveEntity(ent);
			}
		}
	}
};

} // namespace Starbase