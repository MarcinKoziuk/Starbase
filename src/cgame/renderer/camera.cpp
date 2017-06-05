#include <starbase/cgame/renderer/camera.hpp>

namespace Starbase {

Camera::Camera()
{}

Camera::Camera(const glm::vec2& bounds)
	: m_bounds(bounds)
{}

void Camera::Follow(const glm::vec2& entityPos)
{
	const float maxHorizOffs = m_bounds.x / 2;
	const float maxVerticalOffs = m_bounds.y / 2;
	const float horizOffs = m_pos.x - entityPos.x;
	const float verticalOffs = m_pos.y - entityPos.y;

	if (horizOffs > maxHorizOffs)
		m_pos.x += (maxHorizOffs - horizOffs);
	if (horizOffs < -maxHorizOffs)
		m_pos.x -= (maxHorizOffs + horizOffs);
	if (verticalOffs > maxVerticalOffs)
		m_pos.y += (maxVerticalOffs - verticalOffs);
	if (verticalOffs < -maxVerticalOffs)
		m_pos.y -= (maxVerticalOffs + verticalOffs);
}

} // namespace Starbase
