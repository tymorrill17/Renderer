#include "utility/input_manager.h"

void InputManager::initialize(Window* window) {
    this->window = window;
}

void InputManager::process_inputs() {
#ifdef ENABLE_GUI
    Gui& gui = Gui::getGui();
#endif

	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event) != 0) {

#ifdef ENABLE_GUI
        gui->processInputs(&sdl_event);
#endif

		switch (sdl_event.type) {
	 	case SDL_QUIT:
			window->window_should_close = true;
			break;
		case SDL_MOUSEMOTION:
			update_mouse_position(&sdl_event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatch_event(InputEvent::left_mouse_down);
				break;
			case SDL_BUTTON_RIGHT:
				dispatch_event(InputEvent::right_mouse_down);
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatch_event(InputEvent::left_mouse_up);
				break;
			case SDL_BUTTON_RIGHT:
				dispatch_event(InputEvent::right_mouse_up);
				break;
			}
			break;
		case SDL_WINDOWEVENT:
			switch (sdl_event.window.event) {
			case SDL_WINDOWEVENT_MINIMIZED:
                window->pause_rendering = true;
				break;
			case SDL_WINDOWEVENT_RESTORED:
                window->pause_rendering = false;
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (sdl_event.key.keysym.sym) {
			case SDLK_F11:
				if (window->fullscreen) {
					SDL_SetWindowFullscreen(window->sdl_window, 0);
                    window->fullscreen = false;
				}
				else {
					SDL_SetWindowFullscreen(window->sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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
		case SDL_KEYUP:
			switch (sdl_event.key.keysym.sym) {
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
