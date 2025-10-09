#include "renderer/renderer.h"
#include "utility/window.h"
#include "utility/input_manager.h"
#include <cstdint>

static const uint32_t APPLICATION_WIDTH = 1920;
static const uint32_t APPLICATION_HEIGHT = 1080;

int main (int argc, char *argv[]) {

    Window window({APPLICATION_WIDTH, APPLICATION_HEIGHT}, "Renderer");
    Renderer renderer(window);
    InputManager inputManager(window);

    // Main loop
    while (!window.shouldClose()) {
        inputManager.processInputs();

    }

    return 0;
}
