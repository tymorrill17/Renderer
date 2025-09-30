#include "utility/input_manager.h"

InputManager::InputManager(Window& window) :
	_window(window) {}

void InputManager::processInputs() {
	static Gui& _gui = Gui::getGui();

	SDL_Event sdl_event;
	//Handle events on queue
	while (SDL_PollEvent(&sdl_event) != 0) {

		// Let the gui backend handle its inputs
		_gui.processInputs(&sdl_event);

		switch (sdl_event.type) {
	 	case SDL_QUIT: //close the window when user alt-f4s or clicks the X button
			_window.closeWindow();
			break;
		case SDL_MOUSEMOTION:
			updateMousePosition(&sdl_event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatchEvent(InputEvent::leftMouseDown);
				break;
			case SDL_BUTTON_RIGHT:
				dispatchEvent(InputEvent::rightMouseDown);
				break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
				dispatchEvent(InputEvent::leftMouseUp);
				break;
			case SDL_BUTTON_RIGHT:
				dispatchEvent(InputEvent::rightMouseUp);
				break;
			}
			break;
		case SDL_WINDOWEVENT:
			switch (sdl_event.window.event) {
			case SDL_WINDOWEVENT_MINIMIZED:
				_window.setPauseRendering(true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				_window.setPauseRendering(false);
				break;
			}
			break;
		case SDL_KEYDOWN:
			switch (sdl_event.key.keysym.sym) {
			case SDLK_F11:
				if (_window.isFullscreen()) {
					SDL_SetWindowFullscreen(_window.SDL_window(), 0);
					_window.setFullscreen(false);
				}
				else {
					SDL_SetWindowFullscreen(_window.SDL_window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
					_window.setFullscreen(true);
				}
				break;
			case SDLK_SPACE:
				dispatchEvent(InputEvent::spacebarDown);
				break;
			case SDLK_RIGHT:
				dispatchEvent(InputEvent::rightArrowDown);
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

void InputManager::updateMousePosition(SDL_Event* e) {
	// Taking the mouse position from SDL, which has an origin in the top left corner, to
	// our coordinate system which has the origin in the middle
	_mousePosition.x = (e->motion.x * 2.0f / _window.extent().height) - static_cast<float>(_window.extent().width) / static_cast<float>(_window.extent().height);
	_mousePosition.y = (-e->motion.y * 2.0f / _window.extent().height) + 1.0f;
}

void InputManager::addListener(InputEvent inputEvent, std::function<void()> callback) {
	_listeners[inputEvent].push_back(std::move(callback));
}

void InputManager::dispatchEvent(InputEvent event) {
	for (auto& callback : _listeners[event]) {
		callback();
	}
}
