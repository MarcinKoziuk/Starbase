#include <starbase/game/resource/body.hpp>

namespace Starbase {
namespace Resource {

std::shared_ptr<const Body> Body::placeholder = std::make_shared<Body>();

std::size_t Body::CalculateSize() const
{
	return 0L;
}

std::shared_ptr<const Body> Body::Placeholder()
{
	return Body::placeholder;
}

std::shared_ptr<const Body> Body::Create(id_t id, IFilesystem& filesystem)
{
	return nullptr;
}

} // namespace Resource
} // namespace Starbase
