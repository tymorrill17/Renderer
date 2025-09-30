
#include "renderer/renderer.h"
#include "utility/window.h"
#include "utility/input_manager.h"

// @brief The main program
class Application : public NonCopyable {
public:
	// @brief Constructor for a new application
	Application(int windowWidth, int windowHeight, std::string appName) : _window({ windowWidth, windowHeight }, appName),
		_renderer(_window),
		_inputManager(_window) {}

	inline Window& window() { return _window; }
	inline Renderer& renderer() { return _renderer; }
	inline InputManager& inputManager() { return _inputManager; }

private:
	// @brief The main window to display the application
	Window _window;
	// @brief The graphics engine that manages drawing and rendering tasks
	Renderer _renderer;
	InputManager _inputManager;
};