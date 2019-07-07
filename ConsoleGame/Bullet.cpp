#include "ConsoleGame.h"
#include <iostream>

bool Bullet::move(Bullet& b, float elapsed) {
	b.x += b.getVX() * bulletSpeed * elapsed;
	b.y += b.getVY() * bulletSpeed * elapsed;
	int cx = (int)b.x;
	int cy = (int)b.y;
	return !(b.remove = !checkMapBoundary(cx, cy) || findTargetCollision(b) || findWallCollision(cx, cy));
}

float Bullet::getVX() {
	return vx;
}

float Bullet::getVY() {
	return vy;
}

void Bullet::draw(Object& b, float* depthBuffer, wchar_t* screen) {
	vector vec = getAngleFromPlayer(b.x, b.y);
	// If angle is out of field of view return
	if (std::fabs(vec.angle) > fov / 2.0f) return;
	// Calculate distance from the bullet
	float dist = std::sqrt(vec.vx * vec.vx + vec.vy * vec.vy);
	// Calculate radius based on the given distance
	float radius = screenHeight / (dist * 4);
	// Calculate center x coordinate of the bullet
	vec.angle += fov / 2.0f;
	int center = (int)(vec.angle / fov * screenWidth + 0.5f);
	// Calculate start and end positions for drawing
	int startX = center - 2 * radius;
	if (startX < 0) startX = 0;
	int endX = center + 2 * radius;
	if (endX > screenWidth) endX = screenWidth;
	// Draw the bullet
	for (int i = startX; i < endX; ++i) {
		int offset = i - center;
		float d = std::sqrt(radius * radius - offset * offset);
		int startY = (int)((float)screenHeight / 2 - d);
		if (startY < 0) startY = 0;
		int endY = (int)((float)screenHeight / 2 + d);
		if (endY > screenHeight) endY = screenHeight;
		for (int j = startY; j < endY; ++j) {
			if (depthBuffer[i] < dist) continue;
			depthBuffer[i] = dist;
			screen[j * screenWidth + i] = 176;
		}
	}
}