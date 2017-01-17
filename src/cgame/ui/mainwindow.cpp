#include <atomic>
#include <cstdint>

#include <SDL2/SDL.h>

#include <tb/tb_core.h>
#include <tb/tb_skin.h>
#include <tb/tb_system.h>
#include <tb/tb_msg.h>
#include <tb/tb_editfield.h>
#include <tb/tb_widgets_reader.h>
#include <tb/tb_font_renderer.h>
#include <tb/animation/tb_widget_animation.h>

#include <starbase/game/logging.hpp>
#include <starbase/cgame/ui/mainwindow.hpp>
#include <starbase/cgame/ui/renderer/tb_renderer_gl.hpp>
#include <starbase/cgame/ui/startmenu.hpp>

static tb::MODIFIER_KEYS GetModifierKeys(SDL_Keymod mods)
{
    tb::MODIFIER_KEYS code = tb::TB_MODIFIER_NONE;
    if (mods & KMOD_ALT)    code |= tb::TB_ALT;
    if (mods & KMOD_CTRL)   code |= tb::TB_CTRL;
    if (mods & KMOD_SHIFT)  code |= tb::TB_SHIFT;
    if (mods & KMOD_GUI)    code |= tb::TB_SUPER;
    return code;
}

static tb::MODIFIER_KEYS GetModifierKeys()
{
    SDL_Keymod mods = SDL_GetModState();
    return GetModifierKeys(mods);
}

static bool ShouldEmulateTouchEvent()
{
    // Used to emulate that mouse events are touch events when alt, ctrl and shift are pressed.
    // This makes testing a lot easier when there is no touch screen around :)
    return (GetModifierKeys() & (tb::TB_ALT | tb::TB_CTRL | tb::TB_SHIFT)) ? true : false;
}

// @return Return the upper case of a ascii charcter. Only for shortcut handling.
static int toupr_ascii(int ascii)
{
    if (ascii >= 'a' && ascii <= 'z')
        return ascii + 'A' - 'a';
    return ascii;
}

static bool InvokeShortcut(int key, tb::SPECIAL_KEY specialkey, tb::MODIFIER_KEYS modifierkeys, bool down)
{
#ifdef TB_TARGET_MACOSX
    bool shortcut_key = (modifierkeys & tb::TB_SUPER) ? true : false;
#else
    bool shortcut_key = (modifierkeys & tb::TB_CTRL) ? true : false;
#endif
    if (!tb::TBWidget::focused_widget || !down || !shortcut_key)
        return false;

    bool reverse_key = (modifierkeys & tb::TB_SHIFT) ? true : false;
    int upper_key = toupr_ascii(key);
    tb::TBID id;

    if (upper_key == 'X')
        id = TBIDC("cut");
    else if (upper_key == 'C' || specialkey == tb::TB_KEY_INSERT)
        id = TBIDC("copy");
    else if (upper_key == 'V' || (specialkey == tb::TB_KEY_INSERT && reverse_key))
        id = TBIDC("paste");
    else if (upper_key == 'A')
        id = TBIDC("selectall");
    else if (upper_key == 'Z' || upper_key == 'Y')
    {
        bool undo = upper_key == 'Z';
        if (reverse_key)
            undo = !undo;
        id = undo ? TBIDC("undo") : TBIDC("redo");
    }
    else if (upper_key == 'N')
        id = TBIDC("new");
    else if (upper_key == 'O')
        id = TBIDC("open");
    else if (upper_key == 'S')
        id = TBIDC("save");
    else if (upper_key == 'W')
        id = TBIDC("close");
    else if (specialkey == tb::TB_KEY_PAGE_UP)
        id = TBIDC("prev_doc");
    else if (specialkey == tb::TB_KEY_PAGE_DOWN)
        id = TBIDC("next_doc");
    else
        return false;

	tb::TBWidgetEvent ev(tb::EVENT_TYPE_SHORTCUT);
    ev.modifierkeys = modifierkeys;
    ev.ref_id = id;
    return tb::TBWidget::focused_widget->InvokeEvent(ev);
}

static void QueueUserEvent(std::int32_t code, void* data1, void* data2)
{
	// queue a user event to cause the SDL event loop to run
	SDL_Event event;
	SDL_UserEvent userevent;
	userevent.type = SDL_USEREVENT;
	userevent.code = code;
	userevent.data1 = data1;
	userevent.data2 = data2;
	event.type = SDL_USEREVENT;
	event.user = userevent;
	SDL_PushEvent(&event);
}

namespace Starbase {
namespace UI {

MainWindow::MainWindow(glm::tvec2<int> size, tb::TBRenderer& renderer)
    : m_root(*this, tb::TBRect(0, 0, size.x, size.y))
    , m_renderer(renderer)
{}

MainWindow::~MainWindow()
{}

glm::tvec2<int> MainWindow::GetSize() const
{
	return glm::tvec2<int>(m_root.GetRect().w, m_root.GetRect().h);
}

void MainWindow::OnResized(glm::tvec2<int> size)
{
    m_root.SetRect(tb::TBRect(0, 0, size.x, size.y));
}

void MainWindow::Update()
{
    tb::TBAnimationManager::Update();
    m_root.InvokeProcessStates();
    m_root.InvokeProcess();
}

void MainWindow::Render()
{
	tb::TBRect rect = m_root.GetRect(); // get rekt!
    m_renderer.BeginPaint(rect.w, rect.h);
    m_root.InvokePaint(tb::TBWidget::PaintProps());
    m_renderer.EndPaint();

    // If animations are running, reinvalidate immediately
    if (tb::TBAnimationManager::HasAnimationsRunning())
        m_root.Invalidate();
}

bool MainWindow::InvokeKey(int key, tb::SPECIAL_KEY specialkey, tb::MODIFIER_KEYS modifierkeys, bool down)
{
	if (InvokeShortcut(key, specialkey, modifierkeys, down))
		return true;
	else
		return m_root.InvokeKey(key, specialkey, modifierkeys, down);
}

// Attempt to convert an sdl event to a TB event, return true if handled
bool MainWindow::HandleSDLEvent(SDL_Event& event)
{
    bool handled = true;
    switch (event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN: {
        // SDL_KeyboardEvent
        // Handle any key presses here.
        bool down = event.type == SDL_KEYDOWN;
        tb::MODIFIER_KEYS modifier = GetModifierKeys((SDL_Keymod)event.key.keysym.mod);
        if (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_DELETE)
        {
            unsigned int character = event.key.keysym.sym;
            InvokeKey(character, tb::TB_KEY_UNDEFINED, modifier, down);
        }
        else
        {
            // handle special keys
            switch (event.key.keysym.sym)
            {
            case SDLK_F1:			InvokeKey(0, tb::TB_KEY_F1, modifier, down); break;
            case SDLK_F2:			InvokeKey(0, tb::TB_KEY_F2, modifier, down); break;
            case SDLK_F3:			InvokeKey(0, tb::TB_KEY_F3, modifier, down); break;
            case SDLK_F4:			InvokeKey(0, tb::TB_KEY_F4, modifier, down); break;
            case SDLK_F5:			InvokeKey(0, tb::TB_KEY_F5, modifier, down); break;
            case SDLK_F6:			InvokeKey(0, tb::TB_KEY_F6, modifier, down); break;
            case SDLK_F7:			InvokeKey(0, tb::TB_KEY_F7, modifier, down); break;
            case SDLK_F8:			InvokeKey(0, tb::TB_KEY_F8, modifier, down); break;
            case SDLK_F9:			InvokeKey(0, tb::TB_KEY_F9, modifier, down); break;
            case SDLK_F10:			InvokeKey(0, tb::TB_KEY_F10, modifier, down); break;
            case SDLK_F11:			InvokeKey(0, tb::TB_KEY_F11, modifier, down); break;
            case SDLK_F12:			InvokeKey(0, tb::TB_KEY_F12, modifier, down); break;
            case SDLK_LEFT:			InvokeKey(0, tb::TB_KEY_LEFT, modifier, down); break;
            case SDLK_UP:			InvokeKey(0, tb::TB_KEY_UP, modifier, down); break;
            case SDLK_RIGHT:		InvokeKey(0, tb::TB_KEY_RIGHT, modifier, down); break;
            case SDLK_DOWN:			InvokeKey(0, tb::TB_KEY_DOWN, modifier, down); break;
            case SDLK_PAGEUP:		InvokeKey(0, tb::TB_KEY_PAGE_UP, modifier, down); break;
            case SDLK_PAGEDOWN:		InvokeKey(0, tb::TB_KEY_PAGE_DOWN, modifier, down); break;
            case SDLK_HOME:			InvokeKey(0, tb::TB_KEY_HOME, modifier, down); break;
            case SDLK_END:			InvokeKey(0, tb::TB_KEY_END, modifier, down); break;
            case SDLK_INSERT:		InvokeKey(0, tb::TB_KEY_INSERT, modifier, down); break;
            case SDLK_TAB:			InvokeKey(0, tb::TB_KEY_TAB, modifier, down); break;
            case SDLK_DELETE:		InvokeKey(0, tb::TB_KEY_DELETE, modifier, down); break;
            case SDLK_BACKSPACE:	InvokeKey(0, tb::TB_KEY_BACKSPACE, modifier, down); break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:		InvokeKey(0, tb::TB_KEY_ENTER, modifier, down); break;
            case SDLK_ESCAPE:		InvokeKey(0, tb::TB_KEY_ESC, modifier, down); break;
            case SDLK_MENU:
                if (tb::TBWidget::focused_widget && !down) {
                    tb::TBWidgetEvent ev(tb::EVENT_TYPE_CONTEXT_MENU);
                    ev.modifierkeys = modifier;
                    tb::TBWidget::focused_widget->InvokeEvent(ev);
                }
                break;
                /* just ignore lone modifier key presses */
            case SDLK_LCTRL:
            case SDLK_RCTRL:
            case SDLK_LALT:
            case SDLK_RALT:
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
            case SDLK_LGUI:
            case SDLK_RGUI:
                break;
            default:
                handled = false;
                break;
            }
        }

        break;
    }
    case SDL_FINGERMOTION:
    case SDL_FINGERDOWN:
    case SDL_FINGERUP:
        //event.tfinger;
        break;

    case SDL_MOUSEMOTION: {
        if (!(ShouldEmulateTouchEvent() && !tb::TBWidget::captured_widget))
            m_root.InvokePointerMove(event.motion.x, event.motion.y,
                GetModifierKeys(),
                ShouldEmulateTouchEvent());

        break;
    }
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN: {
        // Handle mouse clicks here.
        tb::MODIFIER_KEYS modifier = GetModifierKeys();
        int x = event.button.x;
        int y = event.button.y;
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                // This is a quick fix with n-click support :)
                static double last_time = 0;
                static int last_x = 0;
                static int last_y = 0;
                static int counter = 1;

                double time = tb::TBSystem::GetTimeMS();
                if (time < last_time + 600 && last_x == x && last_y == y)
                    counter++;
                else
                    counter = 1;
                last_x = x;
                last_y = y;
                last_time = time;

                m_root.InvokePointerDown(x, y, counter, modifier, ShouldEmulateTouchEvent());
            } else {
                m_root.InvokePointerUp(x, y, modifier, ShouldEmulateTouchEvent());
            }
        } else if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONUP) {
            m_root.InvokePointerMove(x, y, modifier, ShouldEmulateTouchEvent());
            if (tb::TBWidget::hovered_widget) {
                tb::TBWidget::hovered_widget->ConvertFromRoot(x, y);
                tb::TBWidgetEvent ev(tb::EVENT_TYPE_CONTEXT_MENU, x, y, false, modifier);
                tb::TBWidget::hovered_widget->InvokeEvent(ev);
            }
        }
    }
                              break;
    case SDL_MOUSEWHEEL: {
        int x, y;
        SDL_GetMouseState(&x, &y);
        m_root.InvokeWheel(x, y, (int)event.wheel.x, -(int)event.wheel.y, GetModifierKeys());
        break;
    }
    case SDL_MULTIGESTURE:
        //event.mgesture;
        break;
    case SDL_SYSWMEVENT:
        //event.syswm;
        break;
    case SDL_TEXTEDITING:
        //event.edit;
        break;
    case SDL_TEXTINPUT:
        //event.text;
        break;
    case SDL_WINDOWEVENT: {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            //SDL_Log("Window %d shown", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            //SDL_Log("Window %d hidden", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            //SDL_Log("Window %d exposed", event.window.windowID);
            //OnAppEvent(EVENT_PAINT_REQUEST); <<<<<<<<<<<<<<<<<<<<<<<<<<< TODO
            m_needUpdate = true;
            break;
        case SDL_WINDOWEVENT_MOVED:
            //SDL_Log("Window %d moved to %d,%d",
            //		event.window.windowID, event.window.data1,
            //		event.window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
            OnResized(glm::tvec2<int>(event.window.data1, event.window.data2));
            //SDL_Log("Window %d resized to %dx%d",
            //		event.window.windowID, event.window.data1,
            //		event.window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            //SDL_Log("Window %d size changed to %dx%d",
            //		event.window.windowID, event.window.data1,
            //		event.window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            //SDL_Log("Window %d minimized", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            //SDL_Log("Window %d maximized", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            //SDL_Log("Window %d restored", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            //SDL_Log("Mouse entered window %d", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            //SDL_Log("Mouse left window %d", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            //SDL_Log("Window %d gained keyboard focus", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            //SDL_Log("Window %d lost keyboard focus", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            //SDL_Log("Window %d closed", event.window.windowID);
            break;
        default:
            handled = false;
            LOG(warning) << "Window got unknown event " << event.window.event;
            break;
        }
        break;
    }
    case SDL_USEREVENT:
        // event.user;
        // draw event
        if (m_needUpdate) {
            Update();
            m_needUpdate = false;
            // Bail out if we get here with invalid dimensions.
            // This may happen when minimizing windows (GLFW 3.0.4, Windows 8.1).
            if (GetSize() == glm::tvec2<int>(0, 0))
                ; // ignore
            else {
                Render();
                //SDL_GL_SwapWindow(mainWindow); <<<<<<<<<<<<<<<TODO
            }
        }
        break;
    default:
        handled = false;
    }
    return handled;
}

RootWidget::RootWidget(const MainWindow& mainWindow, const tb::TBRect& size)
	: m_mainWindow(mainWindow)
	, m_startMenu(nullptr)
{
	tb::TBAnimationBlocker blocker;

	SetRect(size);
	SetSkinBg(TBIDC("background_transparent"));

	m_startMenu = new StartMenu(*this);

	//m_startMenu->SetSkinBg(TBIDC("background"));
}

RootWidget::~RootWidget()
{}

} // namespace UI
} // namespace Starbase
