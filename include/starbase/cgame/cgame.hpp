#pragma once

#include <SDL2/SDL_events.h>

#include <starbase/game/game.hpp>
#include <starbase/game/fwd.hpp>
#include <starbase/game/entity/entity.hpp>

#include <starbase/cgame/fwd.hpp>
#include <starbase/cgame/renderer/renderer.hpp>
#include <starbase/cgame/renderer/camera.hpp>

namespace tb { class TBRenderer; }

namespace Starbase {

class CGame : public Game {
protected:
	Display& m_display;
	UI::MainWindow& m_mainWindow;
	Renderer m_renderer;
	Camera m_camera;

	entity_id m_playerEntityId;

	bool HandleSDLEvent(SDL_Event event);

	entity_id AddTestEntity(const char* id, const Transform& transf);

public:
	CGame(Display& m_display, IFilesystem& filesystem, UI::MainWindow& mainWindow);

	virtual ~CGame() {}

	virtual bool Init();

	bool PollEvents();

	void Render(double alpha);

	void CMain();
};

std::unique_ptr<Display> InitDisplay();

std::unique_ptr<tb::TBRenderer> InitUI();

void ShutdownUI();

} // namespace Starbase
