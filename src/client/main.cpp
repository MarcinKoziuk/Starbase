#include <memory>
#include <cstdlib>

#include <SDL2/SDL.h>

#include <tb/tb_core.h>
#include <tb/tb_renderer.h>
#include <tb/tb_system.h>
#include <tb/animation/tb_widget_animation.h>
#include <tb/tb_language.h>
#include <tb/tb_font_renderer.h>

#include <starbase/starbase.hpp>
#include <starbase/game/id.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/game/fs/filesystem_physfs.hpp>
#include <starbase/game/resource/resourceloader.hpp>
#include <starbase/game/resource/text.hpp>
#include <starbase/game/resource/body.hpp>
#include <starbase/game/entity/entity.hpp>
#include <starbase/game/entity/entitymanager.hpp>
#include <starbase/game/component/physics.hpp>
#include <starbase/game/component/transform.hpp>
#include <starbase/game/component/shipcontrols.hpp>
#include <starbase/game/system/physics_system.hpp>
#include <starbase/game/system/shipcontrols_system.hpp>
#include <starbase/cgame/resource/model.hpp>
#include <starbase/cgame/display.hpp>
#include <starbase/cgame/renderer/renderer.hpp>
#include <starbase/cgame/ui/mainwindow.hpp>
#include <starbase/cgame/ui/renderer/tb_renderer_gl.hpp>
#include <starbase/cgame/component/renderable.hpp>
#include <starbase/cgame/renderer/camera.hpp>

using namespace Starbase;

static std::unique_ptr<IFilesystem> InitFilesystem();
static std::unique_ptr<Display> InitDisplay();
static std::unique_ptr<tb::TBRenderer> InitUI();
static void ShutdownUI();
static void MainLoop(UI::MainWindow& mainWindow, Display& display, IFilesystem& fs);


float Rand(float a, float b)
{
	return ((b - a)*((float)rand() / RAND_MAX)) + a;
}

class Scene {
private:
	EntityManager m_entityManager;
	ResourceLoader m_resourceLoader;
	Renderer m_renderer;
	PhysicsSystem m_cpPhysics;
	ShipControlsSystem m_shipControlsSystem;
	Display& m_display;
	IFilesystem& m_fs;
	Camera m_camera;

	entity_id testShipId;

public:
	Scene(Display& display, IFilesystem& fs)
		: m_display(display)
		, m_resourceLoader(fs)
		, m_renderer(display, fs, m_resourceLoader)
		, m_shipControlsSystem(m_entityManager, m_resourceLoader)
        , m_fs(fs)
	{
		m_renderer.Init();

		m_entityManager.entityAdded.connect([this](const Entity& ent) {
			if (m_entityManager.HasComponent<Renderable>(ent)) {
				m_renderer.RenderableAdded(m_entityManager.GetComponent<Renderable>(ent));
			}
			if (m_entityManager.HasComponents<Transform, Physics>(ent)) {
				m_renderer.PhysicsAdded(m_entityManager.GetComponent<Physics>(ent));
				m_cpPhysics.PhysicsAdded(ent, m_entityManager.GetComponent<Transform>(ent), m_entityManager.GetComponent<Physics>(ent));
			}
		});
		m_entityManager.entityWillBeRemoved.connect([this](const Entity& ent) {
			if (m_entityManager.HasComponent<Renderable>(ent)) {
				m_renderer.RenderableRemoved(m_entityManager.GetComponent<Renderable>(ent));
			}
			if (m_entityManager.HasComponents<Transform, Physics>(ent)) {
				m_renderer.PhysicsRemoved(m_entityManager.GetComponent<Physics>(ent));
				m_cpPhysics.PhysicsRemoved(ent, m_entityManager.GetComponent<Transform>(ent), m_entityManager.GetComponent<Physics>(ent));
			}
		});
		m_entityManager.componentAdded.connect([this](const Entity& ent, Entity::component_bitset oldComponents) {
			if (m_entityManager.HasComponent<Renderable>(ent) && !Entity::HasComponent<Renderable>(oldComponents)) {
				m_renderer.RenderableAdded(m_entityManager.GetComponent<Renderable>(ent));
			}
			if (m_entityManager.HasComponents<Transform, Physics>(ent) && !m_entityManager.HasComponents<Transform, Physics>(ent)) {
				m_renderer.PhysicsAdded(m_entityManager.GetComponent<Physics>(ent));
				m_cpPhysics.PhysicsAdded(ent, m_entityManager.GetComponent<Transform>(ent), m_entityManager.GetComponent<Physics>(ent));
			}
		});
		m_entityManager.componentWillBeRemoved.connect([this](const Entity& ent, Entity::component_bitset newComponents) {
			if (m_entityManager.HasComponent<Renderable>(ent) && !Entity::HasComponent<Renderable>(newComponents)) {
				m_renderer.RenderableRemoved(m_entityManager.GetComponent<Renderable>(ent));
			}
			if (m_entityManager.HasComponents<Transform, Physics>(ent) && !m_entityManager.HasComponents<Transform, Physics>(ent)) {
				m_renderer.PhysicsRemoved(m_entityManager.GetComponent<Physics>(ent));
				m_cpPhysics.PhysicsRemoved(ent, m_entityManager.GetComponent<Transform>(ent), m_entityManager.GetComponent<Physics>(ent));
			}
		});

		AddTestShip(ID("models/planets/simple"), glm::vec2(0.f, -50.f), 0, 1.4f);
		//AddTestShip(ID("models/ships/interceptor-0"), glm::vec2(80.f, 80.f), 0.f, 2.f);
		//AddTestShip(ID("models/ships/linesan"), glm::vec2(2000.f, 0.f), 0.4f, 100.f);
		testShipId = AddTestShip(ID("models/ships/fighter-1"), glm::vec2(-80.f, 0.f), 0.4f, 1.f);
		m_entityManager.Refresh();

		Entity& testShip = m_entityManager.GetEntity(testShipId);
		const Transform& testShipTransform = m_entityManager.GetComponent<Transform>(testShip);

		m_camera = Camera(glm::vec2(400, 200));
	}

	bool HandleSDLEvent(SDL_Event event)
	{
		Entity& entity = m_entityManager.GetEntity(testShipId);
		Transform& transf = m_entityManager.GetComponent<Transform>(entity);
		ShipControls& scontrols = m_entityManager.GetComponent<ShipControls>(entity);
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
				AddTestShip(ID("models/doodads/box"), transf.pos - glm::vec2(0, -15.f), transf.rot, Rand(0.1,2.0), transf.vel);
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

	entity_id AddTestShip(id_t id, glm::vec2 pos, float rot, float scale, glm::vec2 initialVelocity = glm::vec2(0.f, 0.f))
	{
		entity_id ret;

		std::tuple<Entity&, Transform&, ShipControls&> ship = m_entityManager.CreateEntity<Transform, ShipControls>();

		Entity& entity = std::get<0>(ship);
		ret = entity.id;

		Transform& transf = std::get<1>(ship);
		transf.pos = pos;
		transf.rot = rot;
		transf.scale = glm::vec2(scale, scale);
		transf.vel = initialVelocity;

		ResourcePtr<Model> modelPtr = m_resourceLoader.Load<Model>(id);
		ResourcePtr<Body> bodyPtr = m_resourceLoader.Load<Body>(id);

		(void) m_entityManager.AddComponent<Renderable>(entity, modelPtr);
		(void) m_entityManager.AddComponent<Physics>(entity, ID("default"), bodyPtr);

		return ret;
	}

	void Update()
	{
		m_entityManager.Refresh();

		m_cpPhysics.Simulate(1.f / 60.f);
		m_entityManager.ForEachEntityWithComponents<Transform, Physics>([this](Entity& ent, Transform& trans, Physics& phys) {
			m_cpPhysics.Update(ent, trans, phys);
		});

		m_entityManager.ForEachEntityWithComponents<Transform, Physics, ShipControls>([this](Entity& ent, Transform& trans, Physics& phys, ShipControls& scontrols) {
			m_shipControlsSystem.Update(ent, trans, phys, scontrols);
		});
	}

	void Render()
	{
		Entity& testShip = m_entityManager.GetEntity(testShipId);
		const Transform& testShipTransform = m_entityManager.GetComponent<Transform>(testShip);

		m_camera.Follow(testShipTransform.pos);
		m_renderer.m_renderParams.offset = m_camera.m_pos;

		m_renderer.BeginDraw();
		m_entityManager.ForEachEntityWithComponents<Transform, Renderable>([this](Entity& ent, Transform& trans, Renderable& rend) {
			Physics* phys = ent.GetComponentOrNull<Physics>();
			ShipControls* contr = ent.GetComponentOrNull<ShipControls>();
			m_renderer.Draw(Renderer::ComponentGroup(ent, trans, rend, phys, contr));
		});
		m_renderer.EndDraw();
	}
};

int main(int argc, char* argv[])
{
	std::unique_ptr<IFilesystem> filesystem = InitFilesystem();
	std::unique_ptr<Display> display = InitDisplay();
	std::unique_ptr<tb::TBRenderer> tbRenderer = InitUI();
	auto mainWindow = std::make_unique<UI::MainWindow>(display->GetWindowSize(), *tbRenderer);

	//////////
	MainLoop(*mainWindow, *display, *filesystem);
	//////////

	delete mainWindow.release();

	ShutdownUI();
	delete tbRenderer.release();

	display->Shutdown();
	filesystem->Shutdown();

	return 0;
}

static void MainLoop(UI::MainWindow& mainWindow, Display& display, IFilesystem& fs)
{
	Scene scene(display, fs);

	bool running = true;
	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event) > 0) {
			if (scene.HandleSDLEvent(event)) {
				continue;
			}
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
		scene.Render();

		mainWindow.Update();
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

static std::unique_ptr<tb::TBRenderer> InitUI()
{
	std::unique_ptr<tb::TBRenderer> tbRenderer = std::make_unique<tb::TBRendererGL>();

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

	void register_freetype_font_renderer();
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

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	//if (font)
	//	font->RenderGlyphs(u8" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~•·åäöÅÄÖ");

	return std::move(tbRenderer);
}

static void ShutdownUI()
{
	if (tb::tb_core_is_initialized()) {
		tb::TBWidgetsAnimationManager::Shutdown();
		tb::tb_core_shutdown();
	}
}
