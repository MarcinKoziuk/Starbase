#include <starbase/game/game.hpp>

namespace Starbase {

Game::Game(IFilesystem& filesystem)
	: m_entityManager(m_eventManager)
	, m_physicsSystem(m_eventManager)
	, m_filesystem(filesystem)
	, m_resourceLoader(filesystem)
	, m_shipControlsSystem(m_entityManager, m_resourceLoader)
{}

bool Game::Init()
{
	return true;
}

} // namespace Starbase
