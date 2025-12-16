#include "utility/input_manager.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"

void InputManager::initialize(Window* window) {
    this->window = window;
}

void InputManager::process_inputs() {
    static Gui& gui = Gui::get_gui();

	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event) != 0) {

        gui.process_inputs(&sdl_event);

		switch (sdl_event.type) {
	 	case SDL_EVENT_QUIT:
			window->window_should_close = true;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			update_mouse_position(&sdl_event);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatch_event(InputEvent::left_mouse_down);
				break;
			case SDL_BUTTON_RIGHT:
				dispatch_event(InputEvent::right_mouse_down);
				break;
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatch_event(InputEvent::left_mouse_up);
				break;
			case SDL_BUTTON_RIGHT:
				dispatch_event(InputEvent::right_mouse_up);
				break;
			}
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
            window->pause_rendering = true;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            window->pause_rendering = false;
            break;
		case SDL_EVENT_KEY_DOWN:
			switch (sdl_event.key.key) {
			case SDLK_F11:
				if (window->fullscreen) {
					SDL_SetWindowFullscreen(window->sdl_window, false);
                    window->fullscreen = false;
				}
				else {
					SDL_SetWindowFullscreen(window->sdl_window, true);
                    window->fullscreen = true;
				}
				break;
			case SDLK_SPACE:
				dispatch_event(InputEvent::space_down);
				break;
			case SDLK_RIGHT:
				dispatch_event(InputEvent::right_arrow_down);
				break;
			default:
				break;
			}
			break;
		case SDL_EVENT_KEY_UP:
			switch (sdl_event.key.key) {
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

void InputManager::update_mouse_position(SDL_Event* e) {
	// Taking the mouse position from SDL, which has an origin in the top left corner, to
	// our coordinate system which has the origin in the middle
	mouse_position.x = (e->motion.x * 2.0f / window->extent.height) - static_cast<float>(window->extent.width) / static_cast<float>(window->extent.height);
	mouse_position.y = (-e->motion.y * 2.0f / window->extent.height) + 1.0f;
}

void InputManager::add_listener(InputEvent input_event, std::function<void()> callback) {
    listeners[input_event].push_back(std::move(callback));
}

void InputManager::dispatch_event(InputEvent event) {
	for (auto& callback : listeners[event]) {
		callback();
	}
}
