#include <cstddef>
#include <cstring>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <starbase/game/logging.hpp>
#include <starbase/game/resource/detail/model_common.hpp>

namespace Starbase {

static std::string SplitFilename(const std::string& path)
{
	std::size_t idx = path.find_last_of("/");
	if (idx != std::string::npos) {
		if (idx < path.size() - 1)
			return path.substr(idx + 1, path.size());
		else
			return "";
	}
	else {
		return path;
	}
}

static std::string GetModelConfigPath(IFilesystem& fs, const std::string& path)
{
	std::string dirname = SplitFilename(path);
	return path + "/" + dirname + ".yml";
}

static std::string GetModelDefaultSvgPath(IFilesystem& fs, const std::string& path)
{
	std::string dirname = SplitFilename(path);
	return path + "/" + dirname + ".svg";
}


std::unique_ptr<ModelFiles> GetModelFiles(IFilesystem& fs, const std::string& path)
{
	std::string cfgPath = GetModelConfigPath(fs, path);

	std::unique_ptr<std::istream> istream = fs.OpenAsStream(cfgPath);
	if (istream != nullptr) {
		try {
			YAML::Node cfg = YAML::Load(*istream);
			std::string svgPath;

			if (cfg["model"]) {
				svgPath = path + "/" + cfg["model"].as<std::string>();
			}
			else {
				svgPath = GetModelDefaultSvgPath(fs, path);
			}

			std::string svgInput;
			bool ok = fs.ReadString(svgPath, &svgInput);
			if (ok) {
				auto svgPtr = NSVGImageUPtr(nsvgParse(reinterpret_cast<char *>(&svgInput.front()), "px", 96));
				if (svgPtr != nullptr) {
					return std::unique_ptr<ModelFiles>(new ModelFiles(std::move(cfg), std::move(svgPtr)));
				}
				else {
					LOG(error) << "Parsing svg file " << svgPath << " failed";
				}
			}
			else {
				LOG(error) << "Could not read model svg " << svgPath;
			}
		}
		catch (...) {
			LOG(error) << "Parsing yml file " << cfgPath << " failed";
		}
	}
	else {
		LOG(error) << "Could not read model config " << cfgPath;
	}

	return nullptr;
}

static glm::vec2 GetShapeCenter(const NSVGshape* shape)
{
	glm::vec2 center;
	NSVGpath* path = shape->paths;
	center.x = (path->bounds[0] + path->bounds[2]) / 2.f;
	center.y = (path->bounds[1] + path->bounds[3]) / 2.f;
	return center;
}

glm::mat4 GetTransformMatrix(const ModelFiles& mf)
{
	const YAML::Node& cfg = mf.cfg;
	const NSVGimage& svg = *mf.svg;

	float scale = 1.;
	float rotation = 0.;
	glm::vec2 origin;

	if (cfg["scale"]) {
		scale = cfg["scale"].as<float>();
	}

	if (cfg["rotation"]) {
		const float rotationDeg = cfg["rotation"].as<float>();
		rotation = glm::radians(rotationDeg);
	}

	for (const NSVGshape* shape = svg.shapes; shape != nullptr; shape = shape->next) {
		const char* group = shape->groupLabel;
		if (std::strcmp(group, ORIGIN_GROUP_LABEL) == 0) {
			origin = GetShapeCenter(shape);
		}
	}

	glm::mat4 transform;
	transform = glm::translate(transform, glm::vec3(-origin.x * scale, -origin.y * scale, 0));
	transform = glm::rotate(transform, rotation, glm::vec3(0.f, 0.f, 1.f));
	transform = glm::scale(transform, glm::vec3(scale, scale, 1.f));
	return transform;
}

glm::vec3 Hex3ToNormalizedColor3(std::uint32_t hex)
{
	glm::vec3 c;
	c.x = ((hex) & 0xFF) / 255.f;
	c.y = ((hex >> 8) & 0xFF) / 255.f;
	c.z = ((hex >> 16) & 0xFF) / 255.f;

	return c;
}

glm::vec4 Hex3ToNormalizedColor4(std::uint32_t hex, float alpha)
{
	glm::vec4 c;
	c.x = ((hex) & 0xFF) / 255.f;
	c.y = ((hex >> 8) & 0xFF) / 255.f;
	c.z = ((hex >> 16) & 0xFF) / 255.f;
	c.w = alpha;

	return c;
}

} // namespace Starbase
