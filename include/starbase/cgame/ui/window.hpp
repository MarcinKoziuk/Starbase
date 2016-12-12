#pragma once

#include <tb/tb_widgets.h>
#include <tb/tb_window.h>

namespace Starbase {
namespace UI {

class Window : public tb::TBWindow {
public:
	Window(tb::TBWidget& root);
	virtual ~Window();

	bool LoadResourceFile(const char *filename);
	void LoadResource(tb::TBNode &node);
};

} // namespace UI
} // namespace Starbase