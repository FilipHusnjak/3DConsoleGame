/*

	Simple console 3D game.
	Keys:
		W - move forward
		S - move backward
		A - rotate left
		D - rotate right
		SPACE - shoot

	NOTE - in order to work properly console size has to be set
		   to width = 240 and height = 80

*/
#ifndef UNICODE
#define UNICODE
#endif
#include "pch.h"
#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <list>
#include <algorithm>
#include "ConsoleGame.h"

#define M_PI 3.141592

constexpr int screenWidth = 240;
constexpr int screenHeight = 80;

int mapWidth = 16;
int mapHeight = 16;

// Map
std::string map =
	"#############..."
	"#..............."
	"#.......########"
	"####...........#"
	"#......##......#"
	"#......###...###"
	"#..............#"
	"#..............#"
	"#..............#"
	"#.#.....###..###"
	"#.#.....#......#"
	"#.#.....#......#"
	"#.#............#"
	"#.####.........."
	"#..............#"
	"################";

std::list<bullet> bullets;

std::list<target> targets = {
	{6.0f, 7.0f, false},
	{12.0f, 13.0f, false}
};


// Player position (X, Y) and angle
float playerX = 9.0f;
float playerY = 7.0f;
float playerA = M_PI;

// Field of view
float fov = M_PI / 4.0f;

// Raytracing resolution
float res = 0.1f;

// Max rendering depth
float dof = 20.0f;

// Player speed
float playerSpeed = 4.0f;

// Bullet speed
float bulletSpeed = 5.0f;

// Boundary width
float bWidth = 0.99995f;

const float PRECISION = 0.07f;

// Checks if the given coordinate is legal
static bool checkMapBoundary(float x, float y) {
	return x >= 0 && x < mapWidth && y >= 0 && y < mapHeight;
}

// Checks if the given point collides with wall
static bool checkCollision(float x, float y) {
	return map[(int)y * mapWidth + (int)x] == '#';
}

static bool findCollisions(const bullet& b) {
	for (target& t : targets) {
		if (std::abs(b.x - t.x) < PRECISION && std::abs(b.y - t.y) < PRECISION) return (t.remove = true);
	}
	return false;
}

// Moves the bullet in its direction
static bool moveBullet(bullet& b, float elapsed) {
	b.x += b.vx * bulletSpeed * elapsed;
	b.y += b.vy * bulletSpeed * elapsed;
	int cx = (int)b.x;
	int cy = (int)b.y;
	return !(b.remove = !checkMapBoundary(cx, cy) || findCollisions(b) || checkCollision(cx, cy));
}

static object_vec getAngleFromPlayer(float x, float y) {
	// Calculate vector from player to the bullet
	float vx = x - playerX;
	float vy = y - playerY;
	// Calculate angle between player and bullet vector
	float angle = playerA - (vy > 0 ? std::atan2(-vy, vx) + 2.0f * M_PI : std::atan2(-vy, vx));
	if (angle < -M_PI) angle += 2.0f * M_PI;
	else if (angle > M_PI) angle -= 2.0f * M_PI;
	return object_vec{ vx, vy, angle };
}

// Draws bullet on the screen
static void drawBullet(bullet& b, float* depthBuffer, wchar_t* screen) {
	object_vec vec = getAngleFromPlayer(b.x, b.y);
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

static void drawTarget(target& t, float* depthBuffer, wchar_t* screen) {
	object_vec vec = getAngleFromPlayer(t.x, t.y);
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

// Checks whether the given coordinate belongs to wall boundary
static bool checkWallBoundary(float currX, float currY, float rayX, float rayY) {
	for (int i = 0; i < 2; i++) {
		for (float j = 0; j < 2; ++j) {
			// Calculate vector from player to the perfect edge
			float vx = currX + i - playerX;
			float vy = currY + j - playerY;
			float mag = std::sqrt(vx * vx + vy * vy);
			// Calculate vector from perfect edge to the center of the wall block
			float cx = 0.5 - i;
			float cy = 0.5 - j;
			float cm = std::sqrt(cx * cx + cy * cy);
			// Calculate cosine of the angle between last 2 vectors
			float cos = (cx * vx + cy * vy) / (mag * cm);
			// If the angle is smaller than -45 degrees the boundary cannot be seen,
			// cos(45) = -0.6675...
			if (cos < -0.6675) continue;
			// If the boundary can be seen find out if current ray is close enough to
			// the checked boundary
			float dot = (vx * rayX + vy * rayY) / mag;
			if (dot > bWidth) return true;
		}
	}
	return false;
}

// Determines ASCII character for wall based on the given distance
static short getWallShade(float dist) {
	if (dist <= dof / 4.0f) { return 0x2588; }
	else if (dist < dof / 3.0f) { return 0x2593; }
	else if (dist < dof / 2.0f) { return 0x2592; }
	else if (dist < dof) { return 0x2591; }
	return ' ';
}

// Determines ASCII character for floor based on the given distance
static char getFloorShade(float dist) {
	if (dist < 0.25) return '-';
	else if (dist < 0.5) return '.';
	else if (dist < 0.75) return 'x';
	return '#';
}

// Moves player in the given direction
static void movePlayer(float movX, float movY) {
	playerX += movX;
	playerY += movY;
	if (!checkMapBoundary(playerX, playerY) || checkCollision(playerX, playerY)) {
		playerX -= movX;
		playerY -= movY;
	}
}

int main(void) {
	// Get and initialize screen buffer
	wchar_t *screen = new wchar_t[screenWidth * screenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	float depthBuffer[screenWidth];

	while (true) {
		// Calculate elapsed time
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float elapsed = elapsedTime.count();

		// Rotate player if needed
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			playerA += 0.3f * playerSpeed * elapsed;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			playerA -= 0.3f * playerSpeed * elapsed;
		}
		playerA -= (int)(playerA / (2.0f * M_PI)) * 2.0f * M_PI;
		float movX = cosf(playerA) * playerSpeed * elapsed;
		float movY = -sinf(playerA) * playerSpeed * elapsed;
		// Move player if needed
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			movePlayer(movX, movY);
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			movePlayer(-movX, -movY);
		}

		float eyeX = cosf(playerA);
		float eyeY = -sinf(playerA);
		// Fire bullet
		if (GetAsyncKeyState(VK_SPACE) & 1) {
			bullet b{ playerX + eyeX, playerY + eyeY, eyeX, eyeY, false };
			bullets.push_back(b);
		}

		float baseAngle = playerA + fov / 2;
		for (int x = 0; x < screenWidth; ++x) {
			// Calculate ray vector
			float rayA = baseAngle - (float) x / screenWidth * fov;
			float rayX = cosf(rayA);
			float rayY = -sinf(rayA);
			// Calculate distance to the closes wall
			float dist = res;
			bool boundary = false;
			for (dist = res; dist < dof; dist += res) {
				int currX = (int)(playerX + rayX * dist);
				int currY = (int)(playerY + rayY * dist);
				if (!checkMapBoundary(currX, currY)) {
					dist = dof;
					break;
				}
				if (map[currY * mapWidth + currX] == '#') {
					boundary = checkWallBoundary(currX, currY, rayX, rayY);
					break;
				}
			}
			depthBuffer[x] = dist;
			// Calculate ceiling and floor
			float ceiling = (float)screenHeight / 2 - (float)screenHeight / dist;
			float floor = screenHeight - ceiling;
			// Get proper ASCII character for calculated distance
			short wShade = boundary ? ' ' : getWallShade(dist);
			// Fill screen buffer
			for (int y = 0; y < screenHeight; ++y) {
				if (y <= ceiling) {
					screen[y * screenWidth + x] = ' ';
				} else if (y >= floor) {
					float s = (y - (float)screenHeight / 2) / ((float)screenHeight / 2);
					screen[y * screenWidth + x] = getFloorShade(s);
				} else {
					screen[y * screenWidth + x] = wShade;
				}
			}
		}
		
		for (target& t : targets) {
			drawTarget(t, depthBuffer, screen);
		}
		// Draw bullets
		for (bullet& b : bullets) {
			if (!moveBullet(b, elapsed)) continue;
			drawBullet(b, depthBuffer, screen);
		}

		// Remove bullets that are out of bounds or that hit the wall
		bullets.remove_if([](bullet& b) { return b.remove; });

		targets.remove_if([](target& t) { return t.remove; });

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f", playerX, playerY, playerA, 1.0f / elapsed);

		// Draw map
		for (int mx = 0; mx < mapWidth; mx++) {
			for (int my = 0; my < mapHeight; my++) {
				screen[(my + 1) * screenWidth + mx] = map[my * mapWidth + mx];
			}
		}
		// Get player vector
		float vecX = cosf(playerA);
		float vecY = -sinf(playerA);
		// Draw player angle
		screen[(int)(vecY + (int)playerY + 1.5) * screenWidth + (int)(vecX + (int)playerX + 0.5)] = 'X';
		// Draw player position
		screen[(int)(playerY + 1) * screenWidth + (int)playerX] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0, 0 }, &dwBytesWritten);
	}

}