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
	void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);

	// @brief Sets the projection matrix member to an orthographic projection
	// @param verticalFOV - vertical field of view, in radians
	// @param aspectRatio - aspect ratio (width/height) of the screen space
	// @param near - near plane
	// @param far - far plane
	void setPerspectiveProjection(float verticalFOV, float aspectRatio, float near, float far);

	// @brief fills the view matrix given a camera position and a direction
	// @param position - position of the camera
	// @param direction - direction the camera is pointing
	// @param up - vector pointing up relative to the camera
	void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});

	// @brief fills the view matrix given a camera position and a target position
	// @param position - position of the camera
	// @param direction - direction the camera is pointing
	// @param up - vector pointing up relative to the camera
	void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.0f, -1.0f, 0.0f });

	// @brief fills the view matrix given a camera position and a set of Euler angles
	// @param position - position of the camera
	// @param rotation - Euler angles of the camera direction in YXZ ordering (pitch, yaw, roll)
	void setViewEulerYXZ(glm::vec3 position, glm::vec3 rotation);

	inline glm::mat4& projectionMatrix() { return _projectionMatrix; }
	inline glm::mat4& viewMatrix() { return _viewMatrix; }

private:

	glm::mat4 _projectionMatrix{ 1.f };
	glm::mat4 _viewMatrix{ 1.f };
};
