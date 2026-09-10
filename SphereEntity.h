#pragma once

struct SphereEntity {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float radius = 0.3f;
	float qx = 0.0f;
	float qy = 0.0f;
	float qz = 0.0f;
	float qw = 1.0f;

	void setPosition(float nextX, float nextY, float nextZ);
	void updateRolling(float velocityX, float velocityZ, float dt);
	void applyOpenGLTransform() const;
};