#pragma once

#include "render2d_types.h"

struct Camera2D
{
	Vec2 position = { 0.0f, 0.0f };
	f32 zoom = 1.0f;
	f32 rotation = 0.0f;
	Vec2 cameraVelocity = { 0.0f, 0.0f };
	const float cameraDamping = 10.0f; // Higher = more responsive

	Mat4 GetViewMatrix() const {
		Mat4 transform = Mat4(1.0f);
		transform = glm::translate(transform, Vec3(-position, 0.0f)); // Negative for proper camera movement
		transform = glm::rotate(transform, glm::radians(-rotation), Vec3(0, 0, 1)); // Negative for proper rotation
		transform = glm::scale(transform, Vec3(zoom, zoom, 1.0f));
		return glm::inverse(transform); // Return inverse for proper view matrix
	}

	Mat4 GetProjectionMatrix(f32 width, f32 height) const {
		const f32 halfHeight = height * 0.5f;
		const f32 halfWidth = width * 0.5f;
		constexpr f32 zNear = -1.0f;
		constexpr f32 zFar = 1.0f;
		return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, zNear, zFar);
	}
};
