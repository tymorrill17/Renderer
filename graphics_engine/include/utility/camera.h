#pragma once

#include "glm/glm.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class Camera {
public:
	// @brief Sets the projection matrix member to an orthographic projection
	// @param left - left plane
	// @param right - right plane
	// @param top - top plane
	// @param bottom - bottom plane
	// @param near - near plane
	// @param far - far plane
	void set_projection_orthographic(float left, float right, float bottom, float top, float near, float far);

	// @brief Sets the projection matrix member to perspective projection
	// @param verticalFOV - vertical field of view, in radians
	// @param aspectRatio - aspect ratio (width/height) of the screen space
	// @param near - near plane
	// @param far - far plane
	void set_projection_perspective(float vertical_fov, float aspect_ratio, float near, float far);

	// @brief fills the view matrix given a camera position and a direction
	// @param position - position of the camera
	// @param direction - direction the camera is pointing
	// @param up - vector pointing up relative to the camera
	void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});

	// @brief fills the view matrix given a camera position and a target position
	// @param position - position of the camera
	// @param direction - direction the camera is pointing
	// @param up - vector pointing up relative to the camera
	void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });

	// @brief fills the view matrix given a camera position and a set of Euler angles
	// @param position - position of the camera
	// @param rotation - Euler angles of the camera direction in YXZ ordering (pitch, yaw, roll)
	void set_view_euler_yxz(glm::vec3 position, glm::vec3 rotation);

	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
};
