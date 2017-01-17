#pragma once

#include <vector>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <starbase/starbase.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

struct NSVGshape;

namespace Starbase {
namespace Resource {

class Model : public IResource {
public:
	struct Style {
		glm::vec4 color;
		float thickness;
		bool useTeamColor;

		Style() : thickness(1.f), useTeamColor(true) {}
	};

	struct Path {
		std::vector<glm::vec2> points;
		Style style;
		bool closed;

		const std::size_t CalculateSize() const
		{
			return sizeof(*this) + PodContainerSize(points);
		}
	};

private:
	std::vector<Path> m_paths;

	static std::shared_ptr<const Model> placeholder;

	void AddShape(const NSVGshape* shape, const glm::mat4& transform);

public:
    virtual ~Model() {}

    const std::vector<Path>& GetPaths() const
    { return m_paths; }

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "model"; }

    static std::shared_ptr<const Model> Placeholder();

	static std::shared_ptr<const Model> MakePlaceholder();

    static std::shared_ptr<const Model> Create(id_t id, IFilesystem& filesystem);
};

} // namespace Resource
} // namespace Starbase
