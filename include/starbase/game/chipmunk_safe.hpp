#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include <chipmunk/chipmunk.h>

namespace Starbase {

// 2D Vector conversions

static cpVect to_cpv(const glm::vec2& v)
{
	return{
		static_cast<cpFloat>(v.x),
		static_cast<cpFloat>(v.y)
	};
}

static glm::vec2 to_vec2f(const cpVect& v)
{
	return {
		static_cast<float>(v.x),
		static_cast<float>(v.y)
	};
}

struct cpShapeDeleter {
	void operator()(cpShape* shape) const
	{
		if (shape != nullptr) {
			cpSpace* space = cpShapeGetSpace(shape);
			if (space != nullptr)
				cpSpaceRemoveShape(space, shape);

			cpShapeFree(shape);
		}
	}
};

struct cpBodyDeleter {
	void operator()(cpBody* body) const
	{
		if (body != nullptr) {
			cpSpace* space = cpBodyGetSpace(body);
			if (space != nullptr)
				cpSpaceRemoveBody(space, body);

			cpBodyFree(body);
		}
	}
};

struct cpSpaceDeleter {
	void operator()(cpSpace* space) const
	{
		if (space != nullptr)
			cpSpaceFree(space);
	}
};

typedef std::unique_ptr<cpShape, cpShapeDeleter> cpShapeUniquePtr;
typedef std::unique_ptr<cpBody, cpBodyDeleter> cpBodyUniquePtr;
typedef std::unique_ptr<cpSpace, cpSpaceDeleter> cpSpaceUniquePtr;


} // namespace Starbase
