#pragma once

#include <memory>

#include <starbase/game/id.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/entity/eventmanager.hpp>

#include <starbase/game/system/physics_system.hpp>
#include <starbase/game/system/shipcontrols_system.hpp>
#include <starbase/game/system/autodestruct_system.hpp>

namespace Starbase {

class Game {
protected:
	IFilesystem& m_filesystem;
	ResourceLoader m_resourceLoader;
	EventManager m_eventManager;
	EntityManager m_entityManager;

	PhysicsSystem m_physicsSystem;
	ShipControlsSystem m_shipControlsSystem;
	AutoDestructSystem m_autoDestructSystem;

	int m_step;

	static constexpr id_t TEST_SPACE = IDC("TEST_SPACE");

public:
	Game(IFilesystem& filesystem);

	virtual ~Game() {}

	virtual bool Init();

	void Update();
};

std::unique_ptr<IFilesystem> InitFilesystem();

} // namespace Starbase
