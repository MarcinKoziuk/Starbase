#include <memory>
#include <functional>

#include <starbase/game/fwd.hpp>
#include <starbase/game/fs/filesystem_physfs.hpp>
#include <starbase/game/game.hpp>

namespace Starbase {

Game::Game(IFilesystem& filesystem)
	: m_entityManager(m_eventManager)
	, m_physicsSystem(m_eventManager)
	, m_filesystem(filesystem)
	, m_resourceLoader(filesystem)
	, m_shipControlsSystem(m_entityManager, m_resourceLoader)
	, m_autoDestructSystem(m_entityManager)
	, m_step(0)
{}

bool Game::Init()
{
	return true;
}

void Game::Update()
{
	using namespace std::placeholders;

	m_entityManager.Update();

	m_physicsSystem.Simulate(1.f / 60.f);

    m_entityManager.ForEachEntityWithComponents<Transform, Physics>(
        std::bind(&PhysicsSystem::Update, &m_physicsSystem, _1, _2, _3));

	m_entityManager.ForEachEntityWithComponents<Transform, Physics, ShipControls>(
        std::bind(&ShipControlsSystem::Update, &m_shipControlsSystem, m_step, _1, _2, _3, _4));

	m_entityManager.ForEachEntityWithComponents<AutoDestruct>(
		std::bind(&AutoDestructSystem::Update, &m_autoDestructSystem, m_step, _1, _2));

	m_step++;
}

std::unique_ptr<IFilesystem> InitFilesystem()
{
	auto filesystem = std::make_unique<FilesystemPhysFS>();

	if (!filesystem->Init()) {
		std::abort();
		return nullptr;
	}

	return std::move(filesystem);
}

} // namespace Starbase
