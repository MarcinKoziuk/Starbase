#pragma once

#include "entity.hpp"
#include "template/event_list.hpp"

namespace Starbase {

struct TextEvent {
	std::string text;

	TextEvent(const std::string& text) : text(text) {}
};

template<typename Entity>
struct TCollisionEvent {
	Entity& first;
	Entity& second;

	TCollisionEvent(Entity& first, Entity& second) : first(first), second(second) {}
};

template<typename Entity>
using EventList = TEventList<TextEvent, TCollisionEvent<Entity>>;

} // namespace Starbase
