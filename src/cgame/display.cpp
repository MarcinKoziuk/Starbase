#include <SDL2/SDL.h>

#include <starbase/starbase.hpp>
#include <starbase/gl.hpp>
#include <starbase/game/logging.hpp>
#include <starbase/cgame/display.hpp>

namespace Starbase {

Display::Display()
    : m_initialized(false)
    , m_window(nullptr)
{}

Display::~Display()
{
    if (m_initialized)
        SDL_Quit();
}

bool Display::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        LOG(fatal) << "SDL initialization failed: " << SDL_GetError();
        return false;
    }

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    m_window = SDL_CreateWindow(STARBASE_NAME,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        1600, 900,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (m_window == NULL) {
        LOG(fatal) << "SDL Window creation failed: " << SDL_GetError();
        return false;
    }

	SDL_GLContext glContext = SDL_GL_CreateContext(m_window);
	SDL_GL_MakeCurrent(m_window, glContext);

    SDL_GL_SetSwapInterval(0);

	//SDL_ShowCursor(1);

#ifdef STARBASE_USING_GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		LOG(fatal) << "Could not init GLEW.";
		return false;
	}

	// GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
	glGetError();
#endif

	m_initialized = true;

    return true;
}

void Display::Shutdown()
{
    if (m_window != nullptr)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}

void Display::Render()
{

}

void Display::Swap()
{
	SDL_GL_SwapWindow(m_window);
}

glm::tvec2<int> Display::GetWindowSize() const
{
    int x, y;
	SDL_GL_GetDrawableSize(m_window, &x, &y);
    return glm::tvec2<int>(x, y);
}

bool Display::IsGL2() const
{
    int maj, profile;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
    return maj == 2 && profile != SDL_GL_CONTEXT_PROFILE_ES;
}

bool Display::IsGL3() const
{
    int maj, profile;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
    return maj == 3 && profile != SDL_GL_CONTEXT_PROFILE_ES;
}

bool Display::IsGLES() const
{
    int profile;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
    return profile == SDL_GL_CONTEXT_PROFILE_ES;
}

const char* Display::GLVersion()
{
    return (const char *) glGetString(GL_VERSION);
}

} // namespace Starbase
