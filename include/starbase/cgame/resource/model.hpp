#pragma once

#include <vector>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <starbase/starbase.hpp>
#include <starbase/game/fs/ifilesystem.hpp>
#include <starbase/game/resource/iresource.hpp>

namespace Starbase {
namespace Resource {

class Model : public IResource {
public:
	struct Path {
		std::vector<glm::vec2> positions;
		glm::vec3 color;
		float thickness;
		bool closed;

		const std::size_t CalculateSize() const
		{
			return sizeof(*this) + PodContainerSize(positions);
		}
	};

private:
	std::vector<Path> m_paths;

	static std::shared_ptr<const Model> placeholder;

public:
    virtual ~Model() {}

    const std::vector<Path>& GetPaths() const
    { return m_paths; }

    virtual std::size_t CalculateSize() const;

    virtual const char* GetResourceName() const
    { return "model"; }

    static std::shared_ptr<const Model> Placeholder();

	static std::shared_ptr<const Model> MakePlaceholder();

    static std::shared_ptr<const Model> Create(const std::string& filename, IFilesystem& filesystem);
};

} // namespace Resource
} // namespace Starbase
