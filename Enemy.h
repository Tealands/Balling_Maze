// Enemy.h
#pragma once
#include "Map.h"
#include "Player.h"
#include "SphereEntity.h"

struct Enemy {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float speed = 2.0f;
	bool active = true;
	SphereEntity sphere;

	void update(float dt, const Map& map, const Player& player);
};
