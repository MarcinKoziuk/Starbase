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
	return std::get<std::vector<C>>(m_components);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::map<entity_id, C>& TENTITYMANAGER_DECL::GetComponentsNew()
{
	return std::get<std::map<entity_id, C>>(m_componentsNew);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
std::vector<C*>& TENTITYMANAGER_DECL::GetComponentsFree()
{
	return std::get<std::vector<C*>>(m_componentsFree);
}
/*
TENTITYMANAGER_TEMPLATE
template<typename C>
auto TENTITYMANAGER_DECL::GetComponentAddedSignal() -> component_signal<C>&
{
	return std::get<component_signal<C>>(componentAdded2);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
auto TENTITYMANAGER_DECL::GetComponentRemovedSignal() -> component_signal<C>&
{
	return std::get<component_signal<C>>(componentRemoved2);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::EmitComponentAddedSignal(Entity& ent, C& comp)
{
	GetComponentAddedSignal<C>().emit(ent, comp);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::EmitComponentRemovedSignal(Entity& ent, C& comp)
{
	GetComponentRemovedSignal<C>().emit(ent, comp);
}
*/
TENTITYMANAGER_TEMPLATE
template<typename C>
std::unordered_map<entity_id, int>& TENTITYMANAGER_DECL::GetComponentsIndex()
{
	return m_componentsIndex[CL::template indexOf<C>()];
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
C& TENTITYMANAGER_DECL::AddComponentExistingImpl(Entity& ent, Args&&... args)
{
	auto& components = GetComponents<C>();
	auto& componentsIndex = GetComponentsIndex<C>();

	components.emplace_back(C(std::forward<Args>(args)...));

	C& com = components.back();
	componentsIndex[ent.id] = static_cast<int>(components.size() - 1);

	ent.template SetBit<C>(true);

	return com;
}

TENTITYMANAGER_TEMPLATE
template<typename C, typename... Args>
C& TENTITYMANAGER_DECL::AddComponentNewImpl(Entity& ent, Args&&... args)
{
	auto& componentsNew = GetComponentsNew<C>();
	C& com = componentsNew.emplace(ent.id, C(std::forward<Args>(args)...)).first->second;

	ent.template SetBit<C>(true);
	return com;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponentNewImpl(Entity& ent)
{
	auto& componentsNew = GetComponentsNew<C>();
	componentsNew.erase(ent.id);

	ent.template SetBit<C>(false);
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponentExistingImpl(Entity& ent)
{
	auto& components = GetComponents<C>();
	auto& componentsFree = GetComponentsFree<C>();
	auto& componentsIndex = GetComponentsIndex<C>();

	C& com = components[componentsIndex[ent.id]];
	com = C{}; // clear component to force destructor to be called

	componentsFree.push_back(&com);
    componentsIndex.erase(ent.id);

	ent.template SetBit<C>(false);
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveComponentsNew(Entity& ent)
{
	TMP::ForEach<typename CL::types>([&](auto type_holder) {
		(void)type_holder;
		using C = typename decltype(type_holder)::type;
		if (ent.template HasComponent<C>())
            this->RemoveComponentNewImpl<C>(ent);
	});
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveComponentsExisting(Entity& ent)
{
	TMP::ForEach<typename CL::types>([&](auto type_holder) {
		(void)type_holder;
		using C = typename decltype(type_holder)::type;
		if (ent.template HasComponent<C>())
            this->RemoveComponentExistingImpl<C>(ent);
	});
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntityExistingImpl(Entity& ent)
{
	RemoveComponentsExisting(ent);

	m_entitiesIndex.erase(ent.id);

	ent = Entity(); // clearing not necessary, but handy for debugging
	ent.alive = false;

	m_entitiesFree.push_back(&ent);
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntityNewImpl(Entity& ent)
{
	RemoveComponentsNew(ent);

	m_entitiesNew.erase(ent.id);
}

TENTITYMANAGER_TEMPLATE
TENTITYMANAGER_DECL::TEntityManager(TEventManagerBase<CL>& eventManager)
	: m_entityIdCounter(0)
	, m_eventManager(eventManager)
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
    return std::forward_as_tuple(ent, AddComponentNewImpl<Cs>(ent)...);
}

TENTITYMANAGER_TEMPLATE
template<typename ...Cs>
auto TENTITYMANAGER_DECL::CreateEntity(Cs&&... cs) -> Entity&
{
	Entity& ent = CreateEntity();

	std::initializer_list<int> _ = {
        ((void)AddComponentNewImpl<std::decay_t<Cs>>(ent, std::forward<Cs>(cs)), 0)...
	};

	return ent;
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::RemoveEntity(Entity& ent)
{
	if (SB_LIKELY(!ent.isnew)) {
		// send signal (not necessary for new ones, because added signal had not been sent)
        m_eventManager.template Emit<entity_removed>(ent);

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
	C* comPtr = nullptr;

	if (SB_LIKELY(!ent.isnew)) {
		const auto oldComponents = ent.bitset;

		comPtr = &AddComponentExistingImpl<C, Args...>(ent, std::forward<Args>(args)...);

		// send signal
		componentAdded.emit(ent, oldComponents);
        m_eventManager.template Emit<C, component_added>(ent, *comPtr);
	}
	else {
		LOG(warning) << "Performance warning: components for new entities should be directly inserted on-construction: " << ent.id;

		comPtr = &AddComponentNewImpl<C, Args...>(ent, std::forward<Args>(args)...);
	}

	return *comPtr;
}

TENTITYMANAGER_TEMPLATE
template<typename C>
void TENTITYMANAGER_DECL::RemoveComponent(Entity& ent)
{
	if (SB_LIKELY(ent.template GetBit<C>())) {
		if (SB_LIKELY(!ent.isnew)) {
			C& comp = ent.template GetComponent<C>();

			auto newComponents = ent.bitset;
			Entity::template SetBit<C>(newComponents, false);

			// send signal
			componentWillBeRemoved.emit(ent, newComponents);
            m_eventManager.template Emit<C, component_removed>(ent, comp);

			RemoveComponentExistingImpl<C>(ent.id);
		}
		else {
			LOG(warning) << "Performance warning: components for new entities should not be removed in the same loop! " << ent.id;

			RemoveComponentNewImpl<C>(ent.id);
		}
	}
	else {
		LOG(warning) << "Entity " << ent.id << " does not have component " << CL::template indexOf<C>() << ", so cannot remove!";
	}
}

TENTITYMANAGER_TEMPLATE
void TENTITYMANAGER_DECL::Update()
{
	while (!m_entitiesNew.empty()) {
		// Make copy of the new entity and remove it
		auto iter = m_entitiesNew.begin();
		Entity tmpEnt = iter->second;
		m_entitiesNew.erase(iter);

		// Push it to the main vector
		tmpEnt.isnew = false;
		m_entities.emplace_back(std::move(tmpEnt));

		// Set its index
		Entity& ent = m_entities.back();
		m_entitiesIndex[ent.id] = static_cast<int>(m_entities.size() - 1);

		// Do the same for its belonging components
		TMP::ForEach<typename CL::types>([&](auto type_holder) {
			(void)type_holder;
			using C = typename decltype(type_holder)::type;

            auto& components = this->GetComponents<C>();
            auto& componentsNew = this->GetComponentsNew<C>();
            auto& componentsIndex = this->GetComponentsIndex<C>();

			if (ent.template HasComponent<C>()) {
				C& com = componentsNew.at(ent.id);

				components.emplace_back(std::move(com));
				componentsNew.erase(ent.id);
				componentsIndex[ent.id] = static_cast<int>(components.size() - 1);
			}
		});

		// Send signal
        m_eventManager.template Emit<entity_added>(ent);
	}

	for (Entity& ent : m_entities) {
		if (ent.needsToDie) {
			RemoveEntity(ent);
		}
	}
}

} // namespace Starbase
