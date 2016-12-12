#pragma once

#include <starbase/cgame/ui/window.hpp>

namespace Starbase {
namespace UI {

class StartMenu : public Window {
public:
	StartMenu(tb::TBWidget& root);
	virtual ~StartMenu() {}
};

} // namespace UI
} // namespace Starbase