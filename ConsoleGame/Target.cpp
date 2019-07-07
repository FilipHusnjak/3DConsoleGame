#include "ConsoleGame.h"
#include <iostream>

void Target::draw(Object& t, float* depthBuffer, wchar_t* screen) {
	vector vec = getAngleFromPlayer(t.x, t.y);
	if (std::fabs(vec.angle) > fov / 2.0f) return;
	float dist = std::sqrt(vec.vx * vec.vx + vec.vy * vec.vy);
	if (dist < 1) return;
	vec.angle += fov / 2.0f;
	int center = (int)(vec.angle / fov * screenWidth + 0.5f);
	int startX = center - 1;
	if (startX < 0) startX = 0;
	int endX = center + 1;
	if (endX > screenWidth) endX = screenWidth;
	int startY = (int)((float)screenHeight / 2.0f - (float)screenHeight / (dist * 5));
	if (startY < 0) startY = 0;
	int endY = (int)((float)screenHeight / 2.0f + screenHeight / dist);
	if (endY > screenHeight) endY = screenHeight;
	for (int i = startX; i < endX; ++i) {
		for (int j = startY; j < endY; ++j) {
			if (depthBuffer[i] < dist) continue;
			depthBuffer[i] = dist;
			screen[j * screenWidth + i] = '|';
		}
	}
}