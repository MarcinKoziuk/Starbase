#include <numeric>

#include <starbase/cgame/resource/model.hpp>

namespace Starbase {
namespace Resource {

std::shared_ptr<const Model> Model::placeholder = Model::MakePlaceholder();

std::size_t Model::CalculateSize() const
{
	return sizeof(*this) + ResourceContainerSize(m_paths);
}

std::shared_ptr<const Model> Model::Placeholder()
{
	return Model::placeholder;
}

std::shared_ptr<const Model> Model::Create(const std::string& filename, IFilesystem& filesystem)
{
	auto model = std::make_shared<Model>();

    // parse something!

	return nullptr;
}

std::shared_ptr<const Model> Model::MakePlaceholder()
{
	std::shared_ptr<Model> model = std::make_shared<Model>();

	// BOX
	Path p1;
	p1.color = glm::vec3(1.f, 0.f, 0.f);
	p1.thickness = 1.0;
	p1.closed = true;
	p1.positions = {
		{-1, 1},
		{-1, -1},
		{1, -1},
		{1, 1}
	};

	// Diagonal \ of X
	Path p2;
	p2.color = glm::vec3(1.f, 0.f, 1.f);
	p2.thickness = 0.5;
	p2.closed = false;
	p2.positions = {
		{ -1, 1 },
		{ 1, -1 }
	};

	// Diagonal / of X
	Path p3;
	p3.color = glm::vec3(1.f, 0.f, 1.f);
	p3.thickness = 0.5;
	p3.closed = false;
	p3.positions = {
		{ -1, -1 },
		{ 1, 1 }
	};

	model->m_paths.push_back(p1);
	model->m_paths.push_back(p2);
	model->m_paths.push_back(p3);

	return model;
}

} // namespace Resource
} // namespace Starbase
