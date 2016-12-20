#include <memory>
#include <cstdlib>

#include <SDL2/SDL.h>

#include <tb/tb_core.h>
#include <tb/tb_renderer.h>
#include <tb/animation/tb_widget_animation.h>
#include <tb/tb_language.h>
#include <tb/tb_font_renderer.h>

#include <starbase/starbase.hpp>
#include <starbase/game/id.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/game/fs/filesystem_physfs.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/game/resource/text.hpp>
#include <starbase/cgame/resource/model.hpp>
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
static void MainLoop(UI::MainWindow& mainWindow, Display& display, IRenderer& renderer, IFilesystem& fs);

#include <starbase/game/entity/entity.hpp>
#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/component/body.hpp>
#include <starbase/cgame/component/renderable.hpp>
#include <starbase/game/component/transform.hpp>

using CGameParams = EParams<Body, Transform, Renderable>;
using EntityManager = TEntityManager<CGameParams>;

class Scene {
private:
	EntityManager m_entityManager;
	ResourceLoader m_resourceLoader;
	IFilesystem& m_fs;
	IRenderer& m_renderer;

public:
	Scene(IRenderer& renderer, IFilesystem& fs)
		: m_renderer(renderer)
		, m_fs(fs)
		, m_resourceLoader(fs)
	{
		std::shared_ptr<const Resource::Model> errorModel = m_resourceLoader.Load<Resource::Model>("blah");

		std::tuple<Entity&, Transform&, Renderable&> test = m_entityManager.CreateEntity<Transform, Renderable>();
		Entity& testEnt = std::get<0>(test);
		Transform& testTrans = std::get<1>(test);
		Renderable& testRend = std::get<2>(test);

		testRend.modelId = ID("blah");
		testTrans.rot = 0.7f;
		testTrans.scale = glm::vec2(200.f, 200.f);
		testTrans.pos = glm::vec2(140.f, 140.f);

		std::tuple<Entity&, Transform&, Renderable&> pest = m_entityManager.CreateEntity<Transform, Renderable>();
		Entity& pestEnt = std::get<0>(pest);
		Transform& pestTrans = std::get<1>(pest);
		Renderable& pestRend = std::get<2>(pest);

		pestRend.modelId = ID("blah");
		pestTrans.rot = 0.f;
		pestTrans.scale = glm::vec2(50.f, 50.f);
		pestTrans.pos = glm::vec2(0.f, 0.f);

	}

	void Update()
	{
		m_entityManager.Refresh();
	}

	void Draw()
	{
		m_entityManager.ForEachEntityWithComponents<Transform, Renderable>([this](Entity& ent, Transform& trans, Renderable& rend) {
			m_renderer.Draw(ent, trans, rend, m_resourceLoader);
		});
	}
};

int main(int argc, char* argv[])
{
	std::unique_ptr<IFilesystem> filesystem = InitFilesystem();
	std::unique_ptr<Display> display = InitDisplay();
	std::unique_ptr<IRenderer> renderer = InitRenderer(*filesystem);
	std::unique_ptr<tb::TBRenderer> tbRenderer = InitUI();
	auto mainWindow = std::make_unique<UI::MainWindow>(display->GetWindowSize(), *tbRenderer);

	//////////
	MainLoop(*mainWindow, *display, *renderer, *filesystem);
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

static void MainLoop(UI::MainWindow& mainWindow, Display& display, IRenderer& renderer, IFilesystem& fs)
{
	Scene scene(renderer, fs);

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

		scene.Update();

		//renderer.DrawTest();
		scene.Draw();
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
