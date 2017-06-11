#include <starbase/game/logging.hpp>

#include <starbase/cgame/cgame.hpp>
#include <starbase/cgame/display.hpp>

namespace Starbase {

CGame::CGame(Display& display, IFilesystem& filesystem)
	: Game(filesystem)
	, m_display(display)
	, m_renderer(display, filesystem, m_resourceLoader, m_eventManager)
{}

bool CGame::Init()
{
	if (!m_renderer.Init())
		return false;

	return true;
}

} // namespace Starbase
