// Player.cpp
#include "Player.h"
#include <cmath>

void Player::update(float dt, const Map& map, float cameraYawDegrees)
{
	// movement input
	float inX = 0.0f, inZ = 0.0f;
	if (keys['W']) inZ += 1.0f; // forward
	if (keys['S']) inZ -= 1.0f; // backward
	if (keys['A']) inX -= 1.0f; // left
	if (keys['D']) inX += 1.0f; // right

	// combine into movement vector relative to camera yaw
	float len = std::sqrt(inX*inX + inZ*inZ);
	float mvx = 0.0f, mvz = 0.0f;
	if (len > 0.0001f) {
		float nx = inX / len;
		float nz = inZ / len;
		float yaw = cameraYawDegrees * 3.14159265358979323846f / 180.0f;
		// forward vector in XZ
		float fx = std::cos(yaw);
		float fz = std::sin(yaw);
		// right vector
		float rx = -fz;
		float rz = fx;
		// input forward is along +Z in our mapping, so forward * nz + right * nx
		mvx = fx * nz + rx * nx;
		mvz = fz * nz + rz * nx;
	}

	float moveStep = speed * dt;
	float oldX = x;
	float oldZ = z;
	float newX = x + mvx * moveStep;
	float newZ = z + mvz * moveStep;

	// simple collision: check target cell
	int targetRow = (int)std::floor(newZ);
	int targetCol = (int)std::floor(newX);
	if (!map.isWallAt(targetRow, targetCol)) {
		x = newX; z = newZ;
	}

	// gravity
	vy -= 9.8f * dt;
	y += vy * dt;
	if (y <= 0.0f) { y = 0.0f; vy = 0.0f; onGround = true; }
	sphere.setPosition(x, y, z);
	sphere.updateRolling((x - oldX) / dt, (z - oldZ) / dt, dt);
}

void Player::jump()
{
	if (onGround) {
		vy = jumpVelocity;
		onGround = false;
	}
}
