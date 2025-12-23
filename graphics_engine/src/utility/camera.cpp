#include "glm/ext/quaternion_geometric.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/common.hpp"
#include <cmath>
#include <numbers>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "utility/camera.h"
#include <iostream>

void Camera::set_projection_orthographic(float left, float right, float bottom, float top, float near, float far) {
   projection       = glm::mat4{ 1.0f };
   projection[0][0] = 2.f / (right - left);
   projection[3][0] = -(right + left) / (right - left);
   projection[1][1] = 2.f / (bottom - top);
   projection[3][1] = -(bottom + top) / (bottom - top);
   projection[2][2] = 1.0f / (far - near);
   projection[3][2] = far / (far - near);
}

void Camera::set_projection_perspective(float vertical_fov, float aspect_ratio, float near, float far) {
    if (aspect_ratio == 0.0f) {
        return;
    }

    const float fov_radians = vertical_fov * std::numbers::pi / 180.0f;
    const float focal_length = 1.0f / tan(fov_radians * 0.5f);
    const float a = near / (far - near);

    projection       = glm::mat4{ 0.0f };
    projection[0][0] = focal_length / aspect_ratio;
    projection[1][1] = -focal_length;
    projection[2][2] = a;
    projection[2][3] = -1.0f;
    projection[3][2] = a * far;
}

void Camera::set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
    glm::vec3 forward     = glm::normalize(direction);
    glm::vec3 right       = glm::cross(forward, up);
    glm::vec3 relative_up = glm::cross(right, forward);

    view = glm::mat4{ 1.0f };
    view[0][0] = right[0];
    view[0][1] = right[1];
    view[0][2] = right[2];
    view[1][0] = relative_up[0];
    view[1][1] = relative_up[1];
    view[1][2] = relative_up[2];
    view[2][0] = -forward[0];
    view[2][1] = -forward[1];
    view[2][2] = -forward[2];

    glm::vec4 translation_vector = glm::vec4{-position, 0.0f};
    for (int i = 0; i < 4; i++) {
        view[3][i] += glm::dot(glm::row(view, i), translation_vector);
    }
}

void Camera::set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
   set_view_direction(position, target - position, up);
}

void Camera::set_view_euler_yxz(glm::vec3 position, glm::vec3 rotation) {
    // TODO: Understand and implement
}


