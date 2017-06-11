#pragma once

#include "entity.hpp"
#include "template/event_list.hpp"

namespace Starbase {

struct TextEvent {
	std::string text;

	TextEvent(const std::string& text) : text(text) {}
};

template<typename Entity>
struct CollisionEvent {
	Entity& first;
	Entity& second;

	CollisionEvent(Entity& first, Entity& second) : first(first), second(second) {}
};

template<typename Entity>
using EventList = TEventList<TextEvent, CollisionEvent<Entity>>;

} // namespace Starbase
