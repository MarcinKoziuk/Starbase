#ifdef _WIN32
	#include <starbase/windows.hpp>
	#include <Shellscalingapi.h>
#endif

#include <starbase/cgame/cgame.hpp>
#include <starbase/cgame/display.hpp>
#include <starbase/cgame/ui/mainwindow.hpp>

using namespace Starbase;

int main(int argc, char* argv[])
{
#ifdef _WIN32
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
#endif

	std::unique_ptr<IFilesystem> filesystem = InitFilesystem();
	std::unique_ptr<Display> display = InitDisplay();
	std::unique_ptr<tb::TBRenderer> tbRenderer = InitUI();
	auto mainWindow = std::make_unique<UI::MainWindow>(display->GetWindowSize(), *tbRenderer);

	CGame cgame(*display, *filesystem, *mainWindow);
	if (!cgame.Init())
		return 1;

	//////////////
	cgame.CMain();
	//////////////

	delete mainWindow.release();

	ShutdownUI();
	delete tbRenderer.release();

	display->Shutdown();
	filesystem->Shutdown();

	return 0;
}

