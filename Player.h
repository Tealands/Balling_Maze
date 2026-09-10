// Player.h
#pragma once
#include <array>
#include "Map.h"

struct Player {
	float x = 0.0f; // world X (column)
	float y = 0.0f; // height
	float z = 0.0f; // world Z (row)
	float vy = 0.0f;
	bool onGround = false;
	bool keys[256] = {0};
	float speed = 5.0f; // units per second
	float jumpVelocity = 8.0f;

	void update(float dt, const Map& map);
	void jump();
};
