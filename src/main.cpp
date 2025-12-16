#include "glm/fwd.hpp"
#include "render_systems/gui_render_system.h"
#include "render_systems/mesh_render_system.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "utility/window.h"
#include "utility/input_manager.h"
#include "utility/asset_loading.h"
#include <cstdint>
#include <string>
#include <SDL3/SDL_main.h>

#define ENABLE_GUI

static const uint32_t APPLICATION_WIDTH = 1920;
static const uint32_t APPLICATION_HEIGHT = 1080;

static std::vector<const char*> requested_validation_layers = {
	"VK_LAYER_KHRONOS_validation", // Standard validation layer preset
};

static std::vector<const char*> requested_device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME, // Necessary extension to use swapchains
    VK_GOOGLE_USER_TYPE_EXTENSION_NAME
};

int main (int argc, char *argv[]) {

    Renderer renderer;
    InputManager input_manager;

    RendererCreateInfo renderer_info{
        .application_name  = "Renderer",
        .window_width      = APPLICATION_WIDTH,
        .window_height     = APPLICATION_HEIGHT,
        .validation_layers = &requested_validation_layers,
        .device_extensions = &requested_device_extensions
    };

    renderer.initialize(&renderer_info);
    input_manager.initialize(&renderer.window);

    GuiRenderSystem gui_render_system;
    gui_render_system.initialize(&renderer);
    renderer.add_render_system(&gui_render_system);

    MeshRenderSystem mesh_render_system;
    mesh_render_system.initialize(&renderer);
    renderer.add_render_system(&mesh_render_system);

    static Gui& gui = Gui::get_gui();

    // Create meshes

    //AssetManager::load_mesh_GLTF(&renderer, " ");

    Mesh test_rectangle = PrimitiveShapes::Rectangle(1.0f, 1.0f);
    test_rectangle.upload_to_GPU(&renderer);

    GPUDrawPushConstants push_constants{
        .world_matrix = glm::mat4{ 1.0f },
        .vertex_buffer_address = test_rectangle.gpu_buffers.vertex_buffer_address,
    };

    mesh_render_system.add_renderable(&test_rectangle);

    // Main loop
    while (!renderer.window.window_should_close) {
        input_manager.process_inputs();

        gui_render_system.start_frame();
        gui.add_widget("test", [&](){
            ImGui::Text("testing...");
        });

        mesh_render_system.update_push_constants(&push_constants);

        renderer.draw();

        gui_render_system.end_frame();
        renderer.resize_callback();
    }

    renderer.wait_for_idle();

    test_rectangle.gpu_buffers.cleanup();
    gui_render_system.cleanup();
    mesh_render_system.cleanup();
    renderer.cleanup();

    return 0;
}
