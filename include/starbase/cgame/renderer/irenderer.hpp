#pragma once

namespace Starbase {

class IRenderer {
public:
	virtual bool Init() = 0;

	virtual void Shutdown() = 0;

	virtual void DrawTest() = 0;

	~IRenderer() {}
};

} // namespace Starbase