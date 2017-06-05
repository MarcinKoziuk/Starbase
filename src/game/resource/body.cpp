#include <cstring>

#include <starbase/game/logging.hpp>
#include <starbase/game/resource/detail/model_common.hpp>
#include <starbase/game/resource/detail/casteljau.hpp>
#include <starbase/game/resource/body.hpp>

namespace Starbase {

typedef glm::tvec2<cpFloat> cpvec2;

static bool IsCircle(const NSVGshape* shape);
static std::vector<std::vector<cpvec2>> ShapeToPolygons(const NSVGshape* shape, const glm::mat4& transform);
static Body::CircleShape ShapeToCircle(const NSVGshape* shape, const glm::mat4& transform);

std::shared_ptr<const Body> Body::placeholder = std::make_shared<Body>();

Body::Body()
	: m_mass(1.f)
	, m_friction(0.f)
{}

std::size_t Body::CalculateSize() const
{
	return sizeof(*this)
		+ PodContainerSize(m_circleShapes)
		+ PodContainerContainerSize(m_polygonShapes);
}

std::shared_ptr<const Body> Body::Placeholder()
{
	return Body::placeholder;
}

std::shared_ptr<const Body> Body::Create(id_t id, IFilesystem& filesystem)
{
	auto body = std::make_shared<Body>();

	std::string path;
	if (!filesystem.GetPathForId(id, path)) {
		return nullptr;
	}

	std::unique_ptr<ModelFiles> modelFiles = GetModelFiles(filesystem, path);
	glm::mat4 transform = GetTransformMatrix(*modelFiles);

	const YAML::Node& cfg = modelFiles->cfg;
	if (cfg["physics"]) {
		const YAML::Node& physicsCfg = cfg["physics"];

		if (physicsCfg["mass"]) {
			body->m_mass = physicsCfg["mass"].as<float>();
		}
		if (physicsCfg["friction"]) {
			body->m_friction = physicsCfg["friction"].as<float>();
		}
	}

	NSVGimage& svg = *modelFiles->svg;

	for (const NSVGshape* shape = svg.shapes; shape != nullptr; shape = shape->next) {
		const char* group = shape->groupLabel;
		const char* label = shape->label;

		if (std::strcmp(group, BODY_GROUP_LABEL) == 0) {
			body->AddShape(shape, transform);
		}
		else if (std::strcmp(group, HARDPOINTS_GROUP_LABEL) == 0) {
			body->AddHardpoint(shape, transform);
		}
	}

	return body;
}

void Body::AddHardpoint(const NSVGshape* shape, const glm::mat4& transform)
{
	const char* name = shape->id;

	Hardpoint hardpoint;
	hardpoint.pos = glm::vec2(transform * glm::vec4(GetShapeCenter(shape), 1.f, 1.f));
	hardpoint.supports.weapons = true;

	if (std::strcmp(name, "medium")) {
		hardpoint.size = Body::Hardpoint::MEDIUM;
	}
	else if (std::strcmp(name, "small")) {
		hardpoint.size = Body::Hardpoint::SMALL;
	}

	m_hardpoints.push_back(hardpoint);
}

void Body::AddShape(const NSVGshape* shape, const glm::mat4& transform)
{
	if (IsCircle(shape)) {
		const CircleShape circle = ShapeToCircle(shape, transform);
		m_circleShapes.push_back(circle);
	}
	else {
		const std::vector<std::vector<cpvec2>> polygons = ShapeToPolygons(shape, transform);

		for (const std::vector<cpvec2>& polygon : polygons) {
			m_polygonShapes.push_back(polygon);
		}
	}
}

static Body::CircleShape ShapeToCircle(const NSVGshape* shape, const glm::mat4& transform)
{
	const NSVGpath* path = shape->paths;
	const cpvec2 min(transform * glm::vec4(path->bounds[0], path->bounds[1], 1.f, 1.f));
	const cpvec2 max(transform * glm::vec4(path->bounds[2], path->bounds[3], 1.f, 1.f));

	double radius = (max.y - min.y) / 2.f;
	cpvec2 pos(min.x + radius, min.y + radius);

	return Body::CircleShape(pos, radius);
}

static std::vector<std::vector<cpvec2>> ShapeToPolygons(const NSVGshape* shape, const glm::mat4& transform)
{
	std::vector<std::vector<cpvec2>> lines;

	for (const NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
		std::stringstream pathInfo;
		pathInfo << "(" << shape->id << " of " << path << ")";

		if (!path->closed) {
			LOG(warning) << "SVG body path is not closed in model " << pathInfo.str();
		}

		std::vector<cpvec2> points;

		for (int i = 0; i < path->npts - 1; i += 3) {
			float* p = &path->pts[i * 2];
			std::vector<glm::vec2> cubicBezier;
			cubicBezier.push_back(cpvec2(p[0], p[1]));
			cubicBezier.push_back(cpvec2(p[2], p[3]));
			cubicBezier.push_back(cpvec2(p[4], p[5]));
			cubicBezier.push_back(cpvec2(p[6], p[7]));

			std::vector<glm::vec2> approximatedBezier = Casteljau(cubicBezier);

			int iz = 0;
			for (const auto& p : approximatedBezier) {
				// TODO: we are losing precision here! (Casteljau should return double)
				const cpvec2 pt = cpvec2(transform * glm::vec4(p, 1.f, 1.f));

				if (iz % 3 == 0) {
					points.push_back(pt);
				}
				iz++;
			}
		}

		if (points.size() < 3) {
			LOG(error) << "SVG body path has less than 3 points " << pathInfo.str();
			continue;
		}
		/*if (points.size() > 8) {
			int reduction = std::ceil((double)points.size() / 8.);

			LOG(warning) << "SVG body path has more than 8 points ("
				<< points.size() << "), reducing x" << reduction << " " << pathInfo.str();


			std::vector<cpvec2> newpoints;
			for (std::size_t i = 0; i < points.size(); i++) {
				if (i % reduction == 0) {
					newpoints.push_back(points[i]);
				}
			}

			points.swap(newpoints);
		}*/

		lines.push_back(points);
	}

	return lines;
}

static bool IsCircle(const NSVGshape* shape)
{
	const NSVGpath* p = shape->paths;
	const double diff = std::abs((p->bounds[2] - p->bounds[0]) - (p->bounds[3] - p->bounds[1]));

	return p->closed
		&& p->npts == 16
		&& p->next == nullptr
		&& diff < 0.001;
}

} // namespace Starbase
