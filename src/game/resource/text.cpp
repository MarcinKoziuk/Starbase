#include <starbase/game/resource/text.hpp>

namespace Starbase {
namespace Resource {

std::shared_ptr<const Text> Text::placeholder = std::make_shared<Text>();

std::size_t Text::CalculateSize() const
{
	return m_text.capacity();
}

std::shared_ptr<const Text> Text::Placeholder()
{
	return Text::placeholder;
}

std::shared_ptr<const Text> Text::Create(id_t id, IFilesystem& filesystem)
{
	auto text = std::make_shared<Text>();
	bool ok = filesystem.ReadString(id, &text->m_text);
	if (ok) {
		return text;
	}
	else {
		return nullptr;
	}
}

} // namespace Resource
} // namespace Starbase
