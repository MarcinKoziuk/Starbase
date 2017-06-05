#pragma once

#include <starbase/game/game.hpp>

#include <starbase/cgame/fwd.hpp>
#include <starbase/cgame/renderer/renderer.hpp>

namespace Starbase {

class CGame : public Game {
protected:
	Display& m_display;
	Renderer m_renderer;

public:
	CGame(Display& m_display, IFilesystem& filesystem);

	virtual bool Init();

	virtual ~CGame() {}
};

} // namespace Starbase
