#pragma once

#include "template/eventmanager.hpp"
#include "component_list.hpp"
#include "event_list.hpp"
#include "entity.hpp"

namespace Starbase {
using EventManagerBase = TEventManagerBase<ComponentList>;
using EventManager = TEventManager<ComponentList, EventList<Entity>>;
}
