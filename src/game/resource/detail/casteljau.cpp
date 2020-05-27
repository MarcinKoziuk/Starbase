#include <cmath>
#include <algorithm>

#include <glm/vec4.hpp>

#include <starbase/game/resource/detail/casteljau.hpp>

namespace Starbase {

static bool IsFlat(const std::vector<glm::vec2>& curve)
{
    const float tol = 10.0;

    float ax = 3.f*curve[1][0] - 2.f*curve[0][0] - curve[3][0]; ax *= ax;
    float ay = 3.f*curve[1][1] - 2.f*curve[0][1] - curve[3][1]; ay *= ay;
    float bx = 3.f*curve[2][0] - curve[0][0] - 2.f*curve[3][0]; bx *= bx;
    float by = 3.f*curve[2][1] - curve[0][1] - 2.f*curve[3][1]; by *= by;

    return std::max(ax, bx) + std::max(ay, by) <= tol;
}

static glm::vec2 Midpoint(const glm::vec2& p, const glm::vec2& q)
{
    return glm::vec2((p.x + q.x) / 2, (p.y + q.y) / 2);
}

static std::vector<glm::vec2> Midpoints(const std::vector<glm::vec2>& curve)
{
    std::vector<glm::vec2> res;
    for (std::size_t i = 0; i < curve.size() - 1; i++) {
        res.push_back(Midpoint(curve[i], curve[i + 1]));
    }

    return res;
}

static std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> Subdivide(const std::vector<glm::vec2>& curve)
{
    std::vector<glm::vec2> midpoints1 = Midpoints(curve);
    std::vector<glm::vec2> midpoints2 = Midpoints(midpoints1);
    std::vector<glm::vec2> midpoints3 = Midpoints(midpoints2);

    std::vector<glm::vec2> piece1 { curve[0], midpoints1[0], midpoints2[0], midpoints3[0] };
    std::vector<glm::vec2> piece2 { midpoints3[0], midpoints2[1], midpoints1[2], curve[3] };

    return std::make_pair(piece1, piece2);
}

std::vector<glm::vec2> CasteljauRaw(const std::vector<glm::vec2>& curve)
{
    std::vector<glm::vec2> res;
    if (IsFlat(curve)) {
        return curve;
    } else {
        auto pieces = Subdivide(curve);
        auto v1 = CasteljauRaw(pieces.first);
        auto v2 = CasteljauRaw(pieces.second);
        res.insert(res.end(), v1.begin(), v1.end());
        res.insert(res.end(), v2.begin(), v2.end());
    }

    return res;
}

std::vector<glm::vec2> Casteljau(const std::vector<glm::vec2>& curve, const glm::mat4& transform, bool closed)
{
	std::vector<glm::vec2> pre = CasteljauRaw(curve);
	std::vector<glm::vec2> post;

	for (const glm::vec2& pt : pre) {
		const glm::vec2& tpt = glm::vec2(transform * glm::vec4(pt, 1.f, 1.f));

		if (post.empty() || post.back() != tpt) {
			post.push_back(tpt);
		}
	}

	if (closed && !post.empty()) {
		if (post.front() == post.back()) {
			post.pop_back();
		}
	}

	return post;
}

} // namespace Starbase
