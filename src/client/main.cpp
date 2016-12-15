#include <memory>
#include <cstdlib>

#include <SDL2/SDL.h>

#include <tb/tb_core.h>
#include <tb/tb_renderer.h>
#include <tb/animation/tb_widget_animation.h>
#include <tb/tb_language.h>
#include <tb/tb_font_renderer.h>

#include <starbase/starbase.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/game/fs/filesystem_physfs.hpp>
#include <starbase/cgame/display.hpp>
#include <starbase/cgame/renderer/renderer_gl2.hpp>
#include <starbase/cgame/ui/mainwindow.hpp>
#include <starbase/cgame/ui/renderer/tb_renderer_gl.hpp>

using namespace Starbase;

static std::unique_ptr<IFilesystem> InitFilesystem();
static std::unique_ptr<Display> InitDisplay();
static std::unique_ptr<IRenderer> InitRenderer(IFilesystem& filesystem);
static std::unique_ptr<tb::TBRenderer> InitUI();
static void ShutdownUI();
static void MainLoop(UI::MainWindow& mainWindow, Display& display, IRenderer& renderer);

#include <starbase/game/entity/entity.hpp>
#include <starbase/game/component/body.hpp>
#include <starbase/game/component/transform.hpp>



void testecs()
{
	EntityManager<Body, Transform> em;

	std::vector<Body>& mahBodies = em.GetComponents<Body>();
	Body body1;
	body1.mass = 9;
	mahBodies.push_back(body1);

	Body& theBody = em.GetComponent<Body>(0);
	LOG(info) << "The mass is " << theBody.mass;
}

int main(int argc, char* argv[])
{
	std::unique_ptr<IFilesystem> filesystem = InitFilesystem();
	std::unique_ptr<Display> display = InitDisplay();
	std::unique_ptr<IRenderer> renderer = InitRenderer(*filesystem);
	std::unique_ptr<tb::TBRenderer> tbRenderer = InitUI();
	auto mainWindow = std::make_unique<UI::MainWindow>(display->GetWindowSize(), *tbRenderer);

	testecs();

	//////////
	MainLoop(*mainWindow, *display, *renderer);
	//////////

	delete mainWindow.release();

	ShutdownUI();
	delete tbRenderer.release();

	renderer->Shutdown();
	delete renderer.release();

	display->Shutdown();
	filesystem->Shutdown();

	return 0;
}

static void MainLoop(UI::MainWindow& mainWindow, Display& display, IRenderer& renderer)
{
	bool running = true;
	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event) > 0) {
			if (mainWindow.HandleSDLEvent(event)) {
				continue;
			}

			switch (event.type) {
			case(SDL_QUIT):
				running = false;
				break;
			}
		}

		renderer.DrawTest();
		mainWindow.Process();
		mainWindow.Render();

		display.Swap();
	}
}

static std::unique_ptr<IFilesystem> InitFilesystem()
{
	auto filesystem = std::make_unique<FilesystemPhysFS>();

	if (!filesystem->Init()) {
		std::abort();
		return nullptr;
	}

	return std::move(filesystem);
}

static std::unique_ptr<Display> InitDisplay()
{
	auto display = std::make_unique<Display>();

	if (!display->Init()) {
		std::abort();
		return nullptr;
	}

	return std::move(display);
}

static std::unique_ptr<IRenderer> InitRenderer(IFilesystem& filesystem)
{
	auto renderer = std::make_unique<RendererGL2>(filesystem);

	if (!renderer->Init()) {
		std::abort();
		return nullptr;
	}

	return std::move(renderer);
}

static std::unique_ptr<tb::TBRenderer> InitUI()
{
	std::unique_ptr<tb::TBRenderer> tbRenderer = std::make_unique<tb::TBRendererGL>();

	if (!tb::tb_core_init(tbRenderer.get())) {
		LOG(error) << "Problem initializing tb core";
		std::abort();
		return nullptr;
	}

	tb::TBWidgetsAnimationManager::Init();

	// Load language file
	tb::g_tb_lng->Load("ui/language/lng_en.tb.txt");

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	tb::g_tb_skin->Load("ui/default_skin/skin.tb.txt", "ui/skin/skin.tb.txt");

	// Register font renderers.

	void register_freetype_font_renderer();
	register_freetype_font_renderer();

	// Add fonts we can use to the font manager.

	tb::g_font_manager->AddFontInfo("ui/vera.ttf", "Vera");


	// Set the default font description for widgets to one of the fonts we just added
	tb::TBFontDescription fd;

	fd.SetID(TBIDC("Vera"));
	fd.SetSize(tb::g_tb_skin->GetDimensionConverter()->DpToPx(14));
	tb::g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	tb::TBFontFace *font = tb::g_font_manager->CreateFontFace(tb::g_font_manager->GetDefaultFontDescription());

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	if (font)
		font->RenderGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ï∑Â‰ˆ≈ƒ÷");

	return std::move(tbRenderer);
}

static void ShutdownUI()
{
	if (tb::tb_core_is_initialized()) {
		tb::TBWidgetsAnimationManager::Shutdown();
		tb::tb_core_shutdown();
	}
}
