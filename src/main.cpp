#include "render_systems/gui_render_system.h"
#include "renderer/renderer.h"
#include "utility/window.h"
#include "utility/input_manager.h"
#include <cstdint>
#include <string>

#define ENABLE_GUI

static const uint32_t APPLICATION_WIDTH = 1920;
static const uint32_t APPLICATION_HEIGHT = 1080;

static std::vector<const char*> requested_validation_layers = {
	"VK_LAYER_KHRONOS_validation" // Standard validation layer preset
};

static std::vector<const char*> requestedDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME // Necessary extension to use swapchains
};

int main (int argc, char *argv[]) {

    Renderer renderer;
    InputManager input_manager;

    RendererCreateInfo renderer_info{
        .application_name  = "Renderer",
        .window_width      = APPLICATION_WIDTH,
        .window_height     = APPLICATION_HEIGHT,
        .validation_layers = &requested_validation_layers,
        .device_extensions = &requestedDeviceExtensions
    };

    renderer.initialize(&renderer_info);
    input_manager.initialize(&renderer.window);

    GuiRenderSystem gui_render_system;
    gui_render_system.initialize(&renderer);
    renderer.add_render_system(&gui_render_system);

    static Gui& gui = Gui::get_gui();

    // Main loop
    while (!renderer.window.window_should_close) {
        input_manager.process_inputs();

        gui_render_system.start_frame();
        gui.add_widget("test", [&](){
            ImGui::Text("testing...");
        });

        renderer.draw();

        gui_render_system.end_frame();
        renderer.resize_callback();
    }

    renderer.wait_for_idle();
    gui_render_system.cleanup();
    renderer.cleanup();

    return 0;
}
