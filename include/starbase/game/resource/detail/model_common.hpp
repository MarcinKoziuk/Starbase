#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include <yaml-cpp/yaml.h>
#include <nanosvg.h>

#include <starbase/game/fs/ifilesystem.hpp>

namespace Starbase {
namespace Resource {

static const char ORIGIN_GROUP_LABEL[] = "origin";
static const char BODY_GROUP_LABEL[] = "body";
static const char HARDPOINTS_GROUP_LABEL[] = "hardpoints";

struct NSVGimage_deleter {
	void operator()(NSVGimage* p) { nsvgDelete(p); }
};

typedef std::unique_ptr<NSVGimage, NSVGimage_deleter> NSVGImageUniqPtr;

struct ModelFiles {
	YAML::Node cfg;
	NSVGImageUniqPtr svg;

	ModelFiles(YAML::Node cfg, NSVGImageUniqPtr svg)
		: cfg(std::move(cfg)), svg(std::move(svg))
	{}
};

std::unique_ptr<ModelFiles> GetModelFiles(IFilesystem& fs, const std::string& path);

glm::mat4 GetTransformMatrix(const ModelFiles& mf);

glm::vec3 Hex3ToNormalizedColor3(std::uint32_t hex);

glm::vec4 Hex3ToNormalizedColor4(std::uint32_t hex, float alpha);

} // namespace Resource
} // namespace Starbase