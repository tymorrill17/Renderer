#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/fwd.hpp"
#include "glm/gtx/transform.hpp"
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
    glm::vec3 center;
    float near_plane;
    float far_plane;
    bool perspective;
    float ortho_box[4];
    float rotation;
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
    Camera world_camera;
    CameraParams camera_config{
        .fov = 70.0f,
        .position = glm::vec3{0.0f, 0.0f, -5.0f},
        .center = glm::vec3{0.0f, 0.0f, 0.0f},
        .near_plane = 10000.0f,
        .far_plane = 0.1f,
        .perspective = true,
        .ortho_box = {-10.0f, 10.0f, -5.0f, 5.0f},
        .rotation = 0.0f
    };

    GPUDrawPushConstants push_constants{
        .vertex_buffer_address = test_meshes.value()[2]->GPU_mesh.vertex_buffer_address
    };

    // Main loop
    while (!renderer.window.window_should_close) {
        input_manager.process_inputs();

        gui_render_system.start_frame();
        gui.add_widget("Camera", [&](){
            ImGui::DragFloat("FOV", &camera_config.fov, 0.1f, 1.0f, 300.0f);
            ImGui::DragFloat3("Position", glm::value_ptr(camera_config.position), 0.1f);
            ImGui::DragFloat3("Center", glm::value_ptr(camera_config.center), 0.1f);
            ImGui::DragFloat("Near Plane", &camera_config.near_plane, 0.001);
            ImGui::DragFloat("Far Plane", &camera_config.far_plane, 0.001);
            ImGui::DragFloat4("Orthographic Box", camera_config.ortho_box, 0.01f);
            ImGui::Checkbox("Perspective", &camera_config.perspective);
            ImGui::DragFloat("Rotation", &camera_config.rotation, 0.1f, 0.0f, 360.0f);

        });
        const glm::vec3 up = {0.0f, 1.0f, 0.0f};
        glm::mat4 projection = glm::mat4{1.0f};
        glm::mat4 view = glm::lookAt(camera_config.position, camera_config.center, up);
        if (camera_config.perspective) {
            projection = glm::perspective(glm::radians(camera_config.fov), renderer.window.aspect_ratio, camera_config.near_plane, camera_config.far_plane);
        } else {
            projection = glm::ortho(camera_config.ortho_box[0], camera_config.ortho_box[1], camera_config.ortho_box[2],
                camera_config.ortho_box[3], camera_config.near_plane, camera_config.far_plane);
        }
        projection[1][1] *= -1;
        glm::mat4 model = glm::rotate(camera_config.rotation, up);

        push_constants.world_matrix = projection * view * model;
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
