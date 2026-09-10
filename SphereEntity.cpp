#include "SphereEntity.h"

#include "framework.h"
#include <cmath>
#include <GL/gl.h>

void SphereEntity::setPosition(float nextX, float nextY, float nextZ)
{
	x = nextX;
	y = nextY;
	z = nextZ;
}

void SphereEntity::updateRolling(float velocityX, float velocityZ, float dt)
{
	float speed = std::sqrt(velocityX * velocityX + velocityZ * velocityZ);
	if (speed < 0.001f || radius <= 0.0f || dt <= 0.0f) return;

	float axisX = velocityZ / speed;
	float axisY = 0.0f;
	float axisZ = -velocityX / speed;
	float angle = speed * dt / radius;
	float halfAngle = angle * 0.5f;
	float sinHalf = std::sin(halfAngle);
	float deltaX = axisX * sinHalf;
	float deltaY = axisY * sinHalf;
	float deltaZ = axisZ * sinHalf;
	float deltaW = std::cos(halfAngle);

	float nextX = deltaW * qx + deltaX * qw + deltaY * qz - deltaZ * qy;
	float nextY = deltaW * qy - deltaX * qz + deltaY * qw + deltaZ * qx;
	float nextZ = deltaW * qz + deltaX * qy - deltaY * qx + deltaZ * qw;
	float nextW = deltaW * qw - deltaX * qx - deltaY * qy - deltaZ * qz;
	float length = std::sqrt(nextX * nextX + nextY * nextY + nextZ * nextZ + nextW * nextW);
	if (length > 0.0001f) {
		qx = nextX / length;
		qy = nextY / length;
		qz = nextZ / length;
		qw = nextW / length;
	}
}

void SphereEntity::applyOpenGLTransform() const
{
	float xx = qx * qx;
	float yy = qy * qy;
	float zz = qz * qz;
	float xy = qx * qy;
	float xz = qx * qz;
	float yz = qy * qz;
	float wx = qw * qx;
	float wy = qw * qy;
	float wz = qw * qz;

	float matrix[16] = {
		1.0f - 2.0f * (yy + zz), 2.0f * (xy + wz), 2.0f * (xz - wy), 0.0f,
		2.0f * (xy - wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + wx), 0.0f,
		2.0f * (xz + wy), 2.0f * (yz - wx), 1.0f - 2.0f * (xx + yy), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	glTranslatef(x, y + radius, z);
	glMultMatrixf(matrix);
}