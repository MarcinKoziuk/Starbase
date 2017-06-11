#pragma once

namespace Starbase {

	class Display;
	class Renderer;

	// Systems
	class PhysicsSystem;
	class ShipControlsSystem;

	// Components
    struct Renderable;
	struct Physics;
	struct ShipControls;
	struct Transform;

	// Resources
    class Model;
	class Body;

	// UI
	namespace UI {
		class MainWindow;
	}
}
