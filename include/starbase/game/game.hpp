#pragma once

#include <memory>

#include <starbase/game/fs/ifilesystem.hpp>

#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/entity/eventmanager.hpp>

#include <starbase/game/system/physics_system.hpp>
#include <starbase/game/system/shipcontrols_system.hpp>

namespace Starbase {

class Game {
protected:
	IFilesystem& m_filesystem;
	ResourceLoader m_resourceLoader;
	EventManager m_eventManager;
	EntityManager m_entityManager;

	PhysicsSystem m_physicsSystem;
	ShipControlsSystem m_shipControlsSystem;

public:
	Game(IFilesystem& filesystem);

	virtual bool Init();

	virtual ~Game() {}
};

} // namespace Starbase
