#pragma once

#include <starbase/cgame/ui/startmenu.hpp>

namespace Starbase {
namespace UI {

StartMenu::StartMenu(tb::TBWidget& root)
	: Window(root)
{
	LoadResourceFile("ui/layout/startmenu.tb.txt");
}

} // namespace UI
} // namespace Starbase