// Enemy.cpp
#include "Enemy.h"
#include <cmath>

void Enemy::update(float dt, const Map& map, const Player& player)
{
	if (!active) return;
	float dx = player.x - x;
	float dz = player.z - z;
	float dist = std::sqrt(dx*dx + dz*dz);
	if (dist < 0.001f) return;
	float ndx = dx / dist;
	float ndz = dz / dist;

	// check adjacent walls to apply speed multiplier
	int curRow = (int)std::floor(z);
	int curCol = (int)std::floor(x);
	bool adjacentWall = false;
	const int dr[4] = {-1,1,0,0};
	const int dc[4] = {0,0,-1,1};
	for (int i=0;i<4;i++) {
		if (map.isWallAt(curRow+dr[i], curCol+dc[i])) { adjacentWall = true; break; }
	}

	float sp = speed * (adjacentWall ? 3.0f : 1.0f);
	float oldX = x;
	float oldZ = z;
	float nx = x + ndx * sp * dt;
	float nz = z + ndz * sp * dt;

	// simple collision: don't enter wall cell
	int targetRow = (int)std::floor(nz);
	int targetCol = (int)std::floor(nx);
	if (!map.isWallAt(targetRow, targetCol)) {
		x = nx; z = nz;
	}
	sphere.setPosition(x, y, z);
	sphere.updateRolling((x - oldX) / dt, (z - oldZ) / dt, dt);
}
