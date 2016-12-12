#pragma once

#include <memory>

#include <SDL2/SDL_events.h>
#include <glm/vec2.hpp>

#include <tb/tb_widgets.h>
#include <tb/tb_window.h>

#include <starbase/cgame/ui/startmenu.hpp>

namespace tb {
	class TBRendererGL;
}

namespace Starbase {
namespace UI {

class MainWindow;

class RootWidget : public tb::TBWidget {
private:
    const MainWindow& m_mainWindow;
	StartMenu* m_startMenu;

public:
	RootWidget(const MainWindow& mainWindow, const tb::TBRect& size);
	virtual ~RootWidget();
    const MainWindow& getMainWindow() const { return m_mainWindow; }
};

class MainWindow {
private:
    RootWidget m_root;
	tb::TBRenderer& m_renderer;
	SDL_Window* m_sdlWindow;
    bool m_needUpdate;

    bool InvokeKey(int key, tb::SPECIAL_KEY specialkey, tb::MODIFIER_KEYS modifierkeys, bool down);
	void LoadResource(tb::TBNode &node);
	bool LoadResourceFile(const char *filename);

public:
    MainWindow(glm::tvec2<int> size, tb::TBRenderer& renderer);
    ~MainWindow();

	glm::tvec2<int> GetSize() const;
    void OnResized(glm::vec2 size);
    void Process();
    void Render();
    bool MainWindow::HandleSDLEvent(SDL_Event& event);

};

} // namespace UI
} // namespace Starbase