#pragma once

#include <cstdint>
#include <memory>

#include <starbase/cgame/resource/model.hpp>

namespace Starbase {

struct Renderable {
	ResourcePtr<Model> model;

	Renderable()
	{}

	Renderable(const ResourcePtr<Model>& model)
		: model(model)
	{}
};

} // namespace Starbase
