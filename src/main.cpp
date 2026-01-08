#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "renderer/image.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/fwd.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
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
    float ortho_scale;
    float rotation;
};

struct CameraBuffer {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 model{1.0f};
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

    Buffer global_uniform_buffer = renderer.create_buffer(sizeof(CameraBuffer), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    CameraBuffer camera_buffer{};

    DescriptorSet global_buffer_descriptor = renderer.descriptor_builder
        .add_buffer(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, &global_uniform_buffer)
        .build();

    MeshRenderSystem mesh_render_system;
    mesh_render_system.initialize(&renderer, std::vector<DescriptorSet>{global_buffer_descriptor});
    renderer.add_render_system(&mesh_render_system);

    static Gui& gui = Gui::get_gui();
    gui.initialize(&renderer);

    // Create meshes
    auto test_meshes = renderer.asset_manager.load_mesh_GLTF(std::filesystem::absolute(root_directory + "/assets/basicmesh.glb"));
    mesh_render_system.add_renderable(test_meshes.value()[2]);

    // Set up camera
    Camera world_camera;
    CameraParams camera_config{
        .fov = 70.0f,
        .position = glm::vec3{0.0f, 0.0f, 5.0f},
        .center = glm::vec3{0.0f, 0.0f, 0.0f},
        .near_plane = 0.1f,
        .far_plane = 10000.0f,
        .perspective = true,
        .ortho_scale = 5.0f,
        .rotation = 0.0f
    };

//    GPUDrawPushConstants push_constants{
//        .vertex_buffer_address = test_meshes.value()[2]->GPU_mesh.vertex_buffer_address
//    };

    // Main loop
    while (!renderer.window.window_should_close) {
        input_manager.process_inputs();

        gui.start_frame();
        gui.add_widget("Camera", [&](){
            ImGui::DragFloat("FOV", &camera_config.fov, 0.1f, 1.0f, 300.0f);
            ImGui::DragFloat3("Position", glm::value_ptr(camera_config.position), 0.1f);
            ImGui::DragFloat3("Center", glm::value_ptr(camera_config.center), 0.1f);
            ImGui::DragFloat("Far Plane", &camera_config.far_plane, 1.0f);
            ImGui::DragFloat("Near Plane", &camera_config.near_plane, 0.001f);
            ImGui::Checkbox("Perspective", &camera_config.perspective);
            ImGui::DragFloat("Orthographic Scale", &camera_config.ortho_scale, 0.1f);
            ImGui::DragFloat("Rotation", &camera_config.rotation, 0.1f, 0.0f, 360.0f);

        });
        gui.add_widget("Renderer", [&](){
            ImGui::DragFloat("Render Scale", &renderer.render_scale, 0.001f, 0.3f, 1.0f);
        });
        //glm::mat4 view = glm::lookAt(camera_config.position, camera_config.center, up);
        //world_camera.set_view_direction(camera_config.position, camera_config.center);
        world_camera.set_view_target(camera_config.position, camera_config.center);
        if (camera_config.perspective) {
            world_camera.set_projection_perspective(camera_config.fov, renderer.window.aspect_ratio, camera_config.near_plane, camera_config.far_plane);
        } else {
            world_camera.set_projection_orthographic(-renderer.window.aspect_ratio/2.0f*camera_config.ortho_scale, renderer.window.aspect_ratio/2.0f*camera_config.ortho_scale, -0.5f*camera_config.ortho_scale, 0.5f*camera_config.ortho_scale, camera_config.near_plane, camera_config.far_plane);
        }
        const glm::vec3 up = {0.0f, 1.0f, 0.0f};
        glm::mat4 model = glm::rotate(camera_config.rotation, up);

        camera_buffer.projection = world_camera.projection;
        camera_buffer.view = world_camera.view;
        camera_buffer.model = model;
        global_uniform_buffer.write_data(&camera_buffer);

        renderer.draw();

        gui.end_frame();
        renderer.resize_callback();
    }

    renderer.wait_for_idle();

    global_uniform_buffer.cleanup();
    global_buffer_descriptor.cleanup();
    for (auto& mesh : test_meshes.value()) mesh->cleanup();
    gui.cleanup();
    mesh_render_system.cleanup();
    renderer.cleanup();

    return 0;
}
