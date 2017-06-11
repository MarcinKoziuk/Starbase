#include <memory>

#include <SDL2/SDL.h>

#include <tb/tb_core.h>
#include <tb/tb_renderer.h>
#include <tb/tb_system.h>
#include <tb/animation/tb_widget_animation.h>
#include <tb/tb_language.h>
#include <tb/tb_font_renderer.h>

#include <starbase/game/logging.hpp>

#include <starbase/cgame/cgame.hpp>
#include <starbase/cgame/display.hpp>
#include <starbase/cgame/ui/mainwindow.hpp>
#include <starbase/cgame/ui/renderer/tb_renderer_gl.hpp>

void register_freetype_font_renderer();

namespace Starbase {

CGame::CGame(Display& display, IFilesystem& filesystem, UI::MainWindow& mainWindow)
	: Game(filesystem)
	, m_display(display)
	, m_renderer(display, filesystem, m_resourceLoader, m_eventManager)
	, m_mainWindow(mainWindow)
{}

bool CGame::Init()
{
	if (!Game::Init())
		return false;

	if (!m_renderer.Init())
		return false;

	AddTestEntity(
		"models/planets/simple",
		Transform(
			glm::vec2(0.f, -50.f),
			0.f,
			glm::vec2(1.4f, 1.4f)
		)
	);

	m_playerEntityId = AddTestEntity(
		"models/ships/fighter-1",
		Transform(
			glm::vec2(-80.f, 0.f),
			0.4f
		)
	);

	m_entityManager.Update();

	m_camera = Camera(glm::vec2(400, 200));

	return true;
}

void CGame::CMain()
{
	bool running = true;

	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event) > 0) {
			if (HandleSDLEvent(event)) {
				continue;
			}
			if (m_mainWindow.HandleSDLEvent(event)) {
				continue;
			}

			switch (event.type) {
			case(SDL_QUIT):
				running = false;
				break;
			}
		}

		Update();
		Render();

		m_mainWindow.Update();
		m_mainWindow.Render();

		m_display.Swap();
	}
}

entity_id CGame::AddTestEntity(const char* id, const Transform& transf)
{
	const ResourcePtr<Model> model = m_resourceLoader.Load<Model>(ID(id));
	const ResourcePtr<Body> body = m_resourceLoader.Load<Body>(ID(id));

	return m_entityManager.CreateEntity<Transform, Physics, ShipControls, Renderable>(
		Transform(transf),
		Physics(TEST_SPACE, body),
		ShipControls(),
		Renderable(model)
	).id;
}

bool CGame::HandleSDLEvent(SDL_Event event)
{
	Entity& playerEntity = m_entityManager.GetEntity(m_playerEntityId);
	Transform& transf = playerEntity.GetComponent<Transform>();
	ShipControls& scontrols = playerEntity.GetComponent<ShipControls>();
	RenderParams& renderParams = m_renderer.m_renderParams;

	switch (event.type) {
	case (SDL_KEYDOWN):
		if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
			scontrols.actionFlags.rotateLeft = true;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
			scontrols.actionFlags.rotateRight = true;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
			scontrols.actionFlags.thrustForward = true;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			AddTestEntity(
				"models/doodads/box",
				Transform(
					transf.pos - glm::vec2(0, -15.f),
					transf.rot,
					glm::vec2(5.f, 5.f),
					transf.vel
				)
			);
		}
		return true;
	case (SDL_KEYUP):
		if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
			scontrols.actionFlags.rotateLeft = false;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
			scontrols.actionFlags.rotateRight = false;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
			scontrols.actionFlags.thrustForward = false;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
			scontrols.actionFlags.firePrimary = true;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_D) {
			LOG(info) << "Setting debug drawing to " << !renderParams.debug;
			renderParams.debug = !renderParams.debug;
		}
		if (event.key.keysym.scancode == SDL_SCANCODE_W) {
			LOG(info) << "Setting wireframe mode to " << !renderParams.wireframe;
			renderParams.wireframe = !renderParams.wireframe;
		}
		return true;
	case (SDL_MOUSEWHEEL):
		renderParams.zoom += event.wheel.y * (0.05f * renderParams.zoom);
		return true;
	}

	return false;
}

void CGame::Render()
{
	Entity& playerEntity = m_entityManager.GetEntity(m_playerEntityId);
	m_camera.Follow(playerEntity.GetComponent<Transform>().pos);
	m_renderer.m_renderParams.offset = m_camera.m_pos;

	m_renderer.BeginDraw();
	m_entityManager.ForEachEntityWithComponents<Transform, Renderable>([this](Entity& ent, Transform& trans, Renderable& rend) {
		const Physics* phys = ent.GetComponentOrNull<Physics>();
		const ShipControls* contr = ent.GetComponentOrNull<ShipControls>();
		m_renderer.Draw(Renderer::ComponentGroup(ent, trans, rend, phys, contr));
	});
	m_renderer.EndDraw();
}

std::unique_ptr<Display> InitDisplay()
{
	auto display = std::make_unique<Display>();

	if (!display->Init()) {
		std::abort();
		return nullptr;
	}

	return std::move(display);
}

std::unique_ptr<tb::TBRenderer> InitUI()
{
	std::unique_ptr<tb::TBRenderer> tbRenderer = std::make_unique<::tb::TBRendererGL>();

	if (!tb::tb_core_init(tbRenderer.get())) {
		LOG(error) << "Problem initializing tb core";
		std::abort();
		return nullptr;
	}

	// Do not optimize our custom implementation away while linking (fixes linux build issue)
	SB_UNUSED(const static auto& _tbFsFix = &tb::TBFile::Open);
	_tbFsFix("wut", tb::TBFile::MODE_READ);

	tb::TBWidgetsAnimationManager::Init();

	// Load language file
	tb::g_tb_lng->Load("ui/language/lng_en.tb.txt");

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	tb::g_tb_skin->Load("ui/starbase_skin/skin.tb.txt"); // , "ui/skin/skin.tb.txt");

														 // Register font renderers.

	register_freetype_font_renderer();

	// Add fonts we can use to the font manager.


	tb::g_font_manager->AddFontInfo("ui/fonts/Overpass/Overpass-Regular.ttf", "Overpass");
	tb::g_font_manager->AddFontInfo("ui/fonts/Exo/Exo-Regular.ttf", "Exo");
	tb::g_font_manager->AddFontInfo("ui/fonts/Geo/Geo-Regular.ttf", "Geo");
	//tb::g_font_manager->AddFontInfo("ui/fonts/Muli/Muli-Regular.ttf", "Cantarell");
	tb::g_font_manager->AddFontInfo("ui/fonts/muli.ttf", "Cantarell");

	// Set the default font description for widgets to one of the fonts we just added
	tb::TBFontDescription fd;

	fd.SetID(TBIDC("Geo"));
	fd.SetSize(tb::g_tb_skin->GetDimensionConverter()->DpToPx(19));
	tb::g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	tb::TBFontFace *font = tb::g_font_manager->CreateFontFace(tb::g_font_manager->GetDefaultFontDescription());

	return std::move(tbRenderer);
}

void ShutdownUI()
{
	if (tb::tb_core_is_initialized()) {
		tb::TBWidgetsAnimationManager::Shutdown();
		tb::tb_core_shutdown();
	}
}

} // namespace Starbase
