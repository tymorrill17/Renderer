#define GLM_ENABLE_EXPERIMENTAL
#include "glm/fwd.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "render_systems/gui_render_system.h"
#include "render_systems/mesh_render_system.h"
#include "renderer/renderer.h"
#include "renderer/mesh.h"
#include "utility/camera.h"
#include "utility/logger.h"
#include "utility/window.h"
#include "utility/input_manager.h"
#include "utility/asset_loading.h"
#include <cstdint>
#include <filesystem>
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

#ifdef ROOT_DIR
const std::string root_directory = std::string(ROOT_DIR);
#endif // ROOT_DIR

struct CameraParams {
    float fov;
    glm::vec3 position;
    glm::vec3 direction;
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

    MeshRenderSystem mesh_render_system;
    mesh_render_system.initialize(&renderer);
    renderer.add_render_system(&mesh_render_system);

    GuiRenderSystem gui_render_system;
    gui_render_system.initialize(&renderer);
    renderer.add_render_system(&gui_render_system);

    static Gui& gui = Gui::get_gui();

    // Create meshes
    auto test_meshes = renderer.asset_manager.load_mesh_GLTF(std::filesystem::absolute(root_directory + "/assets/basicmesh.glb"));
    mesh_render_system.add_renderable(test_meshes.value()[2]);

    // Set up camera
//    Camera world_camera;
//    CameraParams camera_config{
//        .fov = 70.0f,
//        .position = glm::vec3{0.0f, 0.0f, -2.0f},
//        .direction = glm::vec3{0.0f, 0.0f, 1.0f}
//    };
//    world_camera.set_projection_perspective(camera_config.fov, renderer.window.aspect_ratio, 10000.0f, 0.1f);
//    world_camera.set_view_direction(camera_config.position, camera_config.direction);

    //glm::mat4 view = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 0,0,-5 });
    glm::mat4 view = glm::mat4{ 1.0f };
	glm::mat4 projection = glm::perspective(glm::radians(70.f), renderer.window.aspect_ratio, 10000.f, 0.1f);
    projection[1][1] *= -1;

    GPUDrawPushConstants push_constants{
//        .world_matrix = world_camera.projection * world_camera.view,
//        .world_matrix = glm::mat4{ 1.0f },
        .world_matrix = projection * view,
        .vertex_buffer_address = test_meshes.value()[2]->GPU_mesh.vertex_buffer_address
    };

    // Main loop
    while (!renderer.window.window_should_close) {
        input_manager.process_inputs();

        gui_render_system.start_frame();
//        gui.add_widget("Camera", [&](){
//            ImGui::DragFloat("FOV", &camera_config.fov, 1.0f, 70.0f, 120.0f);
//            ImGui::DragFloat3("Position", glm::value_ptr(camera_config.position), 0.1f);
//            ImGui::DragFloat3("Direction", glm::value_ptr(camera_config.direction), 0.1f);
//        });
//        world_camera.set_projection_perspective(camera_config.fov, renderer.window.aspect_ratio, 10000.0f, 0.1f);
//        world_camera.set_view_direction(camera_config.position, camera_config.direction);
//        push_constants.world_matrix = world_camera.projection * world_camera.view;
        mesh_render_system.update_push_constants(&push_constants);

        renderer.draw();

        gui_render_system.end_frame();
        renderer.resize_callback();
    }

    renderer.wait_for_idle();

    for (auto& mesh : test_meshes.value()) mesh->cleanup();
    gui_render_system.cleanup();
    mesh_render_system.cleanup();
    renderer.cleanup();

    return 0;
}
