/*

	Simple console 3D game.
	Keys:
		W - move forward
		S - move backward
		A - rotate left
		D - rotate right

*/

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <chrono>
#include <vector>
#include <algorithm>

int screenWidth = 120;
int screenHeight = 40;

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


// Player position (X, Y) and angle
float playerX = 9.0f;
float playerY = 9.0f;
float playerA = 3.1415f / 2.0f;

// Field of view
float fov = 3.1415f / 4.0f;

// Raytracing resolution
float res = 0.1f;

// Max rendering depth
float dof = 20.0f;

// Player speed
float speed = 5.0f;

// Boundary width
float bWidth = 0.99995f;

// Checks if the given coordinate is legal
static bool checkMapBoundary(float cord, float maxCord) {
	return cord >= 0 && cord < maxCord;
}

// Checks if the given point collides with wall
static bool checkCollision(float x, float y) {
	return map[(int)y * mapWidth + (int)x] == '#';
}

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

// Determines ASCII character based on the given distance
static short getWallShade(float dist) {
	if (dist <= dof / 4.0f) { return 0x2588; }
	else if (dist < dof / 3.0f) { return 0x2593; }
	else if (dist < dof / 2.0f) { return 0x2592; }
	else if (dist < dof) { return 0x2591; }
	return ' ';
}

static char getFloorShade(float s) {
	if (s < 0.25) return '-';
	else if (s < 0.5) return '.';
	else if (s < 0.75) return 'x';
	return '#';
}

// Moves player in the given direction
static void movePlayer(float movX, float movY) {
	playerX += movX;
	playerY += movY;
	if (!checkMapBoundary(playerX, mapWidth) || !checkMapBoundary(playerY, mapHeight)
		|| checkCollision(playerX, playerY)) {
		playerX -= movX;
		playerY -= movY;
	}
}

int main() {
	// Get and initialize screen buffer
	wchar_t *screen = new wchar_t[screenWidth * screenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	while (true) {
		// Calculate elapsed time
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float elapsed = elapsedTime.count();

		// Rotate player if needed
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			playerA += 0.75f * speed * elapsed;
		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			playerA -= 0.75f * speed * elapsed;
		}

		float movX = cosf(playerA) * speed * elapsed;
		float movY = -sinf(playerA) * speed * elapsed;
		// Move player if needed
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			movePlayer(movX, movY);
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			movePlayer(-movX, -movY);
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
				if (!checkMapBoundary(currX, mapWidth) || !checkMapBoundary(currY, mapHeight)) {
					dist = dof;
					break;
				}
				if (map[currY * mapWidth + currX] == '#') {
					boundary = checkWallBoundary(currX, currY, rayX, rayY);
					break;
				}
			}
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
					char fShade = getFloorShade(s);
					screen[y * screenWidth + x] = fShade;
				} else {
					screen[y * screenWidth + x] = wShade;
				}
			}
		}

		// Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerA, 1.0f / elapsed);

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