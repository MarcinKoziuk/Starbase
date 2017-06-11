#pragma once

#include "template/eventmanager.hpp"
#include "component_list.hpp"
#include "event_list.hpp"
#include "entity.hpp"
#include "entitymanager.hpp"

namespace Starbase {
using EventManager = TEventManager<ComponentList, EventList<Entity>>;

using CollisionEvent = TCollisionEvent<Entity>;
}
