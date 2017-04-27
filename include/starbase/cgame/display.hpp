#pragma once

#include <glm/vec2.hpp>

struct SDL_Window;

namespace Starbase {

class Display {
private:
    bool m_initialized;
    SDL_Window* m_window;

public:
    Display();
    ~Display();
    
    bool Init();
    void Shutdown();
	void Render();
	void Swap();

    bool IsGL2() const;
    bool IsGL3() const;
    bool IsGLES() const;
    static const char* GLVersion();
    glm::tvec2<int> GetWindowSize() const;
};

} // namespace Starbase
