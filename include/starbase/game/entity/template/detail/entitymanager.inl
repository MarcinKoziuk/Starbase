#pragma once

#include <cstddef>
#include <limits>
#include <algorithm>
#include <functional>

#include <starbase/game/logging.hpp>

#include "tmp.hpp"

namespace Starbase {

#define TENTITYMANAGER_TEMPLATE \
template<typename CL>

#define TENTITYMANAGER_DECL \
TEntityManager<CL>

TENTITYMANAGER_TEMPLATE
entity_id TENTITYMANAGER_DECL::GenerateId()
{
	return ++m_entityIdCounter;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::vector<C>& TENTITYMANAGER_DECL::GetComponents()
{
	return TMP::GetTupleContainer<std::vector<C>, typename CL::vector_type>(m_components);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::map<entity_id, C>& TENTITYMANAGER_DECL::GetComponentsNew()
{
	return TMP::GetTupleContainer<std::map<entity_id, C>, typename CL::map_type>(m_componentsNew);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::vector<C*>& TENTITYMANAGER_DECL::GetComponentsFree()
{
	return TMP::GetTupleContainer<std::vector<C*>, typename CL::freelist_type>(m_componentsFree);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::unordered_map<entity_id, int>& TENTITYMANAGER_DECL::GetComponentsIndex()
{
	return m_componentsIndex[CL::template indexOf<C>()];
}

TENTITYMANAGER_TEMPLATE
int TENTITYMANAGER_DECL::IndexOfEntity(Entity* pEnt)
{
	const std::size_t indexOfEntity = pEnt - &m_entities.front();
	assert(indexOfEntity < std::numeric_limits<int>::max());
	return (int)indexOfEntity;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
int TENTITYMANAGER_DECL::IndexOfComponent(C* pCom)
{
	std::vector<C>& components = GetComponents<C>();
	const std::size_t indexOfComponent = pCom - &components.front();

	assert(indexOfComponent < std::numeric_limits<int>::max());
	return (int)indexOfComponent;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
C& TENTITYMANAGER_DECL::GetComponentExistingImpl(entity_id id)
{
	auto& components = GetComponents<C>();
	auto& componentsIndex = GetComponentsIndex<C>();

	assert(componentsIndex.count(id));
	return components.at(componentsIndex.at(id));
}

TENTITYMANAGER_TEMPLATE
template<typename C>
C& TENTITYMANAGER_DECL::GetComponentNewImpl(entity_id id)
{
	auto& componentsNew = GetComponentsNew<C>();

	assert(componentsNew.count(id));
	return componentsNew.at(id);
}

TENTITYMANAGER_TEMPLATE
template<typename C, typename... Args>
C& TENTITYMANAGER_DECL::AddComponentExistingImpl(entity_id id, Args&&... args)
{
	auto& components = GetComponents<C>();
	auto& componentsIndex = GetComponentsIndex<C>();

	components.emplace_back(C(std::forward<Args>(args)...));
	C* pcom = &components.back();

	componentsIndex[id] = IndexOfComponent<C>(pcom);
	return *pcom;
}

TENTITYMANAGER_TEMPLATE
template<typename C, typename... Args>
C& TENTITYMANAGER_DECL::AddComponentNewImpl(entity_id id, Args&&... args)
{
	auto& componentsNew = GetComponentsNew<C>();
	C& com = componentsNew.emplace(id, C(std::forward<Args>(args)...)).first->second;
	return com;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
C& TENTITYMANAGER_DECL::AddComponentNewAndSetBitImpl(Entity& ent)
{
	ent.template SetBit<C>(true);
	return AddComponentNewImpl<C>(ent.id);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponentNewImpl(entity_id id)
{
	auto& componentsNew = GetComponentsNew<C>();
	componentsNew.erase(id);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponentExistingImpl(entity_id id)
{
	auto& components = GetComponents<C>();
	auto& componentsFree = GetComponentsFree<C>();
	auto& componentsIndex = GetComponentsIndex<C>();

	C& com = components[componentsIndex[id]];
	com = C{}; // clear component to force destructor to be called

	componentsFree.push_back(&com);
	componentsIndex.erase(id);
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveComponentsNewIfBitIsSetImpl(Entity& ent)
{
	TMP::ForEach<typename CL::types>([&](auto type_holder) {
		(void)type_holder;
		using C = typename decltype(type_holder)::type;
		if (ent.template HasComponent<C>())
			RemoveComponentNewImpl<C>(ent.id);
	});
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveComponentsExistingIfBitIsSetImpl(Entity& ent)
{
	TMP::ForEach<typename CL::types>([&](auto type_holder) {
		(void)type_holder;
		using C = typename decltype(type_holder)::type;
		if (ent.template HasComponent<C>())
			RemoveComponentExistingImpl<C>(ent.id);
	});
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntityExistingImpl(Entity& ent)
{
	RemoveComponentsExistingIfBitIsSetImpl(ent);

	m_entitiesIndex.erase(ent.id);

	ent = Entity(); // clearing not necessary, but handy for debugging
	ent.alive = false;

	m_entitiesFree.push_back(&ent);
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntityNewImpl(Entity& ent)
{
	RemoveComponentsNewIfBitIsSetImpl(ent);

	m_entitiesNew.erase(ent.id);
}

TENTITYMANAGER_TEMPLATE
TENTITYMANAGER_DECL::TEntityManager()
	: m_entityIdCounter(0)
{
	static_assert(CL::count <= MAX_COMPONENTS, "MAX_COMPONENTS size is unsufficient!");
	m_componentsIndex.resize(CL::count);
}

TENTITYMANAGER_TEMPLATE
auto TENTITYMANAGER_DECL::GetEntity(entity_id id) -> Entity&
{
	return m_entities[m_entitiesIndex[id]];
}

TENTITYMANAGER_TEMPLATE
template<typename C>
C& TENTITYMANAGER_DECL::GetComponent(const Entity& ent)
{
	if (SB_LIKELY(!ent.isnew)) {
		return GetComponentExistingImpl<C>(ent.id);
	}
	else {
		LOG(warning) << "Performance warning: don't call GetComponent on newly constructed entities!";
		return GetComponentNewImpl<C>(ent.id);
	}
}

TENTITYMANAGER_TEMPLATE
template<typename F>
void TENTITYMANAGER_DECL::ForEachEntity(F fun)
{
	for (Entity& ent : m_entities) {
		if (SB_LIKELY(ent.alive)) {
			fun(ent);
		}
	}
}

TENTITYMANAGER_TEMPLATE
template<typename ...Cs, typename F>
void TENTITYMANAGER_DECL::ForEachEntityWithComponents(F fun)
{
	for (Entity& ent : m_entities) {
		if (SB_LIKELY(ent.alive)) {
			if (ent.template HasComponents<Cs...>()) {
				fun(ent, ent.template GetComponent<Cs>()...);
			}
		}
	}
}

TENTITYMANAGER_TEMPLATE
auto TENTITYMANAGER_DECL::CreateEntity() -> Entity&
{
	entity_id id = GenerateId();
	return m_entitiesNew.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(id),
		std::forward_as_tuple(id, *this)).first->second;
}

TENTITYMANAGER_TEMPLATE
template<typename ...Cs>
auto TENTITYMANAGER_DECL::CreateEntity() -> std::tuple<Entity&, Cs&...>
{
	Entity& ent = CreateEntity();
	return std::forward_as_tuple(ent, AddComponentNewAndSetBitImpl<Cs>(ent)...);
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntity(Entity& ent)
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

TENTITYMANAGER_TEMPLATE
template<typename C, typename... Args>
C& TENTITYMANAGER_DECL::AddComponent(Entity& ent, Args&&... args)
{
	C* pCom = nullptr;

	if (SB_LIKELY(!ent.isnew)) {
		pCom = &AddComponentExistingImpl<C, Args...>(ent.id, std::forward<Args>(args)...);

		const auto oldComponents = ent.bitset;
		ent.template SetBit<C>(true);

		// send signal
		componentAdded(ent, oldComponents);
	}
	else {
		//LOG(warning) << "Performance warning: components for new entities should be directly inserted on-construction: " << ent.id;

		pCom = &AddComponentNewImpl<C, Args...>(ent.id, std::forward<Args>(args)...);

		ent.template SetBit<C>(true);
	}

	return *pCom;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponent(Entity& ent)
{
	if (SB_LIKELY(ent.template GetBit<C>())) {
		if (SB_LIKELY(!ent.isnew)) {
			auto newComponents = ent.bitset;
			Entity::template SetBit<C>(newComponents, false);

			// send signal
			componentWillBeRemoved(ent, newComponents);

			RemoveComponentExistingImpl<C>(ent.id);
			ent.bitset = newComponents;
		}
		else {
			LOG(warning) << "Performance warning: components for new entities should not be removed in the same loop! " << ent.id;

			RemoveComponentNewImpl<C>(ent.id);

			ent.template SetBit<C>(false);
		}
	}
	else {
		LOG(warning) << "Entity " << ent.id << " does not have component " << CL::template indexOf<C>() << ", so cannot remove!";
	}
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::Refresh()
{
	while (!m_entitiesNew.empty()) {
		// Make copy of the new entity and remove it
		auto iter = m_entitiesNew.begin();
		Entity ent = iter->second;
		m_entitiesNew.erase(iter);

		// Push it to the main vector
		ent.isnew = false;
		m_entities.emplace_back(std::move(ent));

		// Set its index in the index ;)
		Entity* pEnt = &m_entities.back();
		m_entitiesIndex[pEnt->id] = IndexOfEntity(pEnt);

		// Do the same for its belonging components
		TMP::ForEach<typename CL::types>([&](auto type_holder) {
			(void)type_holder;
			using C = typename decltype(type_holder)::type;

			auto& components = GetComponents<C>();
			auto& componentsNew = GetComponentsNew<C>();
			auto& componentsIndex = GetComponentsIndex<C>();

			if (ent.template HasComponent<C>()) {
				C& com = componentsNew.at(ent.id);
				components.emplace_back(std::move(com));
				componentsNew.erase(ent.id);

				C* pCom = &components.back();
				componentsIndex[ent.id] = IndexOfComponent(pCom);
			}
		});

		// Send signal
		entityAdded(*pEnt);
	}

	for (Entity& ent : m_entities) {
		if (ent.needsToDie) {
			RemoveEntity(ent);
		}
	}
}

} // namespace Starbase
