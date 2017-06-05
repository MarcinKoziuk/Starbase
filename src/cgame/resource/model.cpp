#include <numeric>
#include <cstring>

#include <yaml-cpp/yaml.h>

#include <nanosvg.h>

#include <starbase/game/logging.hpp>
#include <starbase/game/resource/detail/model_common.hpp>
#include <starbase/game/resource/detail/casteljau.hpp>

#include <starbase/cgame/resource/model.hpp>

namespace Starbase {

static std::vector<Model::Path> ShapeToPaths(const NSVGshape* shape, Model::Style style, const glm::mat4& transform, id_t group);

std::shared_ptr<const Model> Model::placeholder = Model::MakePlaceholder();

std::size_t Model::CalculateSize() const
{
	return sizeof(*this) + SizeAwareContainerSize(m_paths);
}

std::shared_ptr<const Model> Model::Placeholder()
{
	return Model::placeholder;
}

std::shared_ptr<const Model> Model::Create(id_t id, IFilesystem& filesystem)
{
	auto model = std::make_shared<Model>();

	std::string path;
	if (!filesystem.GetPathForId(id, path)) {
		return nullptr;
	}

	std::unique_ptr<ModelFiles> modelFiles = GetModelFiles(filesystem, path);
	glm::mat4 transform = GetTransformMatrix(*modelFiles);

	NSVGimage& svg = *modelFiles->svg;

	for (const NSVGshape* shape = svg.shapes; shape != nullptr; shape = shape->next) {
		const char* group = shape->groupLabel;

		if (std::strlen(group) > 0 && group[0] == GROUP_LABEL_PREFIX) {
			continue;
		}

		model->AddShape(shape, transform, ID(group));
	}

	return model;
}

void Model::AddShape(const NSVGshape* shape, const glm::mat4& transform, id_t group)
{
	Model::Style style;
	style.thickness = shape->strokeWidth;
	style.color = glm::vec4(0.f, 0.f, 1.f, 1.f);
	style.color.w = shape->opacity;

	if (shape->stroke.type == NSVG_PAINT_COLOR) {
		const unsigned currentColor = shape->stroke.color;
		if (currentColor != 0xff000000 && currentColor != 0xffffffff) {
			style.color = Hex3ToNormalizedColor4(shape->stroke.color, style.color.w);
		}
	}

	const auto paths = ShapeToPaths(shape, style, transform, group);
	m_paths.insert(m_paths.end(), paths.begin(), paths.end());
}

static std::vector<Model::Path> ShapeToPaths(const NSVGshape* shape, Model::Style style, const glm::mat4& transform, id_t group)
{
	std::vector<Model::Path> paths;

	for (const NSVGpath* svgPath = shape->paths; svgPath != nullptr; svgPath = svgPath->next) {
		Model::Path path;
		path.style = style;
		path.closed = svgPath->closed == 1;
		path.group = group;
		
		for (int i = 0; i < svgPath->npts - 1; i += 3) {
			float* p = &svgPath->pts[i * 2];
			std::vector<glm::vec2> cubicBezier;
			cubicBezier.push_back(glm::vec2(p[0], p[1]));
			cubicBezier.push_back(glm::vec2(p[2], p[3]));
			cubicBezier.push_back(glm::vec2(p[4], p[5]));
			cubicBezier.push_back(glm::vec2(p[6], p[7]));

			std::vector<glm::vec2> approximatedBezier = Casteljau(cubicBezier);

			int iz = 0;
			for (const auto& p : approximatedBezier) {
				auto v4 = transform * glm::vec4(p, 1.f, 1.f);
				const glm::vec2 pt = glm::vec2(v4);

				if (iz % 3 == 0) {
				path.points.push_back(pt);
				}
				iz++;
			}
		}

		if (path.points.size() < 2) {
			LOG(error) << "Path in " << shape->groupLabel << ", has less than 2 points (" << path.points.size() << ")";
			continue;
		}

		paths.push_back(path);
	}

	return paths;
}

std::shared_ptr<const Model> Model::MakePlaceholder()
{
	std::shared_ptr<Model> model = std::make_shared<Model>();

	Style outer;
	outer.color = glm::vec4(1.f, 0.f, 0.f, 1.f);
	outer.thickness = 1.f;

	Style inner;
	inner.color = glm::vec4(1.f, 0.f, 1.f, .8f);
	inner.thickness = .5f;

	// BOX
	Path p1;
	p1.style = outer;
	p1.closed = true;
	p1.points = {
		{-1, 1},
		{-1, -1},
		{1, -1},
		{1, 1}
	};

	// Diagonal \ of X
	Path p2;
	p2.style = inner;
	p2.closed = false;
	p2.points = {
		{ -1, 1 },
		{ 1, -1 }
	};

	// Diagonal / of X
	Path p3;
	p3.style = inner;
	p3.closed = false;
	p3.points = {
		{ -1, -1 },
		{ 1, 1 }
	};

	model->m_paths.push_back(p1);
	model->m_paths.push_back(p2);
	model->m_paths.push_back(p3);

	return model;
}

} // namespace Starbase
