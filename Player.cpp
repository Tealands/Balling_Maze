// Player.cpp
#include "Player.h"
#include <cmath>

void Player::update(float dt, const Map& map)
{
	// movement input
	float dx = 0.0f, dz = 0.0f;
	if (keys['W']) dz -= 1.0f;
	if (keys['S']) dz += 1.0f;
	if (keys['A']) dx -= 1.0f;
	if (keys['D']) dx += 1.0f;
	// normalize
	float len = std::sqrt(dx*dx + dz*dz);
	if (len > 0.0001f) { dx /= len; dz /= len; }

	float moveStep = speed * dt;
	float newX = x + dx * moveStep;
	float newZ = z + dz * moveStep;

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
}

void Player::jump()
{
	if (onGround) {
		vy = jumpVelocity;
		onGround = false;
	}
}
