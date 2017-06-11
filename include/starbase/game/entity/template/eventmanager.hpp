#pragma once

#include <functional>

#include <wink/signal.hpp>

#include "entity.hpp"
#include "component_list.hpp"
#include "event_list.hpp"

#define SB_IF_CLASS(E, c) typename std::enable_if_t<std::is_base_of<c, E>::value, c>* = nullptr
#define SB_IF_NOT_CLASS(E, c) typename std::enable_if_t<!std::is_base_of<c, E>::value, c>* = nullptr

namespace Starbase {

template<typename CL>
class TEventManagerBase {
public:
	using Entity = TEntity<CL>;

	struct entity_event {};
	struct entity_added : public entity_event {};
	struct entity_removed : public entity_event {};
	struct component_added : public entity_event {};
	struct component_removed : public entity_event {};

	friend TEntityManager<CL>;

private:
	template<typename C>
	using component_signal = typename CL::template signal_type_one<Entity, C>;

	using component_signals = typename CL::template signal_type<Entity>;

	component_signals componentAdded;
	component_signals componentRemoved;
	wink::signal<std::function<void(Entity& entity)>> entityAdded;
	wink::signal<std::function<void(Entity& entity)>> entityRemoved;

public:
	// Due to a MSVC compiler issue, these overloads can currently not have a separate
	// definition and declaration while using std::enable_if (error C2244)

	template<typename C, typename E, SB_IF_CLASS(E, component_added)>
	void Emit(Entity& ent, C& comp)
	{
		std::get<component_signal<C>>(componentAdded).emit(ent, comp);
	}

	template<typename C, typename E, SB_IF_CLASS(E, component_removed)>
	void Emit(Entity& ent, C& comp)
	{
		std::get<component_signal<C>>(componentRemoved).emit(ent, comp);
	}

	template<typename E, SB_IF_CLASS(E, entity_added)>
	void Emit(Entity& ent)
	{
		entityAdded.emit(ent);
	}

	template<typename E, SB_IF_CLASS(E, entity_removed)>
	void Emit(Entity& ent)
	{
		entityRemoved.emit(ent);
	}

	template<typename E, SB_IF_CLASS(E, entity_added), typename... Args>
	void Connect(Args&&... args)
	{
		entityAdded.connect(std::forward<Args>(args)...);
	}

	template<typename E, SB_IF_CLASS(E, entity_removed), typename... Args>
	void Connect(Args&&... args)
	{
		entityRemoved.connect(std::forward<Args>(args)...);
	}

	template<typename C, typename E, SB_IF_CLASS(E, component_added), typename... Args>
	void Connect(Args&&... args)
	{
		std::get<component_signal<C>>(componentAdded).connect(std::forward<Args>(args)...);
	}

	template<typename C, typename E, SB_IF_CLASS(E, component_removed), typename... Args>
	void Connect(Args&&... args)
	{
		std::get<component_signal<C>>(componentRemoved).connect(std::forward<Args>(args)...);
	}
};

template<typename CL, typename EL>
class TEventManager : public TEventManagerBase<CL> {
    template<typename E>
    using event_list_signal = typename EL::template signal_type_one<E>;

    using event_list_signals = typename EL::signal_type;

	event_list_signals signals;

public:
	using entity_event = typename TEventManagerBase<CL>::entity_event;
	using entity_added = typename TEventManagerBase<CL>::entity_added;
	using entity_removed = typename TEventManagerBase<CL>::entity_removed;
    using component_added = typename TEventManagerBase<CL>::component_added;
    using component_removed = typename TEventManagerBase<CL>::component_removed;

	template<typename E, SB_IF_NOT_CLASS(E, entity_event), typename... Args>
    void Emit(Args&&... args)
	{
        std::get<event_list_signal<E>>(signals).emit(std::forward<Args>(args)...);
	}

    template<typename E, SB_IF_NOT_CLASS(E, entity_event), typename... Args>
    void Connect(Args&&... args)
	{
        std::get<event_list_signal<E>>(signals).connect(std::forward<Args>(args)...);
	}

    // We must redefine Connect overloads from the base class, otherwise they
    // are shadowed by our subclass. Using "using TEventManagerBase<CL>::Connect"
    // does not seem to work in MSVC (error C2672)
	
	template<typename E, SB_IF_CLASS(E, entity_added), typename... Args>
	void Connect(Args&&... args)
	{
        TEventManagerBase<CL>::template Connect<E>(std::forward<Args>(args)...);
	}

	template<typename E, SB_IF_CLASS(E, entity_removed), typename... Args>
	void Connect(Args&&... args)
	{
        TEventManagerBase<CL>::template Connect<E>(std::forward<Args>(args)...);
	}

	template<typename C, typename E, SB_IF_CLASS(E, component_added), typename... Args>
	void Connect(Args&&... args)
	{
		TEventManagerBase<CL>::template Connect<C, E>(std::forward<Args>(args)...);
	}

	template<typename C, typename E, SB_IF_CLASS(E, component_removed), typename... Args>
	void Connect(Args&&... args)
	{
		TEventManagerBase<CL>::template Connect<C, E>(std::forward<Args>(args)...);
	}
};

#undef SB_IF_CLASS
#undef SB_IF_NOT_CLASS

} // namespace Starbase
