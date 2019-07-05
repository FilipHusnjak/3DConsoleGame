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

int screenWidth = 120;
int screenHeight = 40;

int mapWidth = 16;
int mapHeight = 16;

// Map
std::string map =
	"##########...###"
	"#..............#"
	"#..............#"
	"#....###.......#"
	"#..............#"
	"#..............#"
	"#..............#"
	"#..............#"
	"#..............#"
	"#..............#"
	"#..............#"
	"#....####......#"
	"#....#.........#"
	"#....#.........#"
	"#..............#"
	"################";

// Player position (X, Y) and angle
float playerX = 9.0f;
float playerY = 9.0f;
float playerA = 3.1415 / 2;

// Field of view
float fov = 3.1415f / 4.0f;

// Raytracing resolution
float res = 0.1f;

// Max rendering depth
float dof = 20.0f;

// Player speed
float speed = 5.0f;

// Checks if the given coordinate is legal
static bool checkBoundary(float cord, float maxCord) {
	return cord >= 0 && cord < maxCord;
}

// Checks if the given point collides with wall
static bool checkCollision(float x, float y) {
	return map[(int)y * mapWidth + (int)x] == '#';
}

// Determines ASCII character based on the given distance
static short getShade(float dist) {
	if (dist <= dof / 4.0f) { return 0x2588; }
	else if (dist < dof / 3.0f) { return 0x2593; }
	else if (dist < dof / 2.0f) { return 0x2592; }
	else if (dist < dof) { return 0x2591; }
	return ' ';
}

// Moves player in the given direction
static void movePlayer(float movX, float movY) {
	playerX += movX;
	playerY += movY;
	if (!checkBoundary(playerX, mapWidth) || !checkBoundary(playerY, mapHeight)
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
		for (int i = 0; i < screenWidth; ++i) {
			// Calculate ray vector
			float rayA = baseAngle - (float) i / screenWidth * fov;
			float rayX = cosf(rayA);
			float rayY = -sinf(rayA);
			// Calculate distance to the closes wall
			float dist = res;
			for (; dist < dof; dist += res) {
				int currX = (int)(playerX + rayX * dist);
				int currY = (int)(playerY + rayY * dist);
				if (!checkBoundary(currX, mapWidth) || !checkBoundary(currY, mapHeight)) {
					dist = dof;
					break;
				}
				if (map[currY * mapWidth + currX] == '#') break;
			}
			// Get proper ASCII character for calculated distance
			short shade = getShade(dist);
			// Calculate ceiling and floor
			float ceiling = (float)screenHeight / 2 - (float)screenHeight / dist;
			float floor = screenHeight - ceiling;
			// Fill screen buffer
			for (int j = 0; j < screenHeight; ++j) {
				if (j <= ceiling || j >= floor) {
					screen[j * screenWidth + i] = ' ';
				} else {
					screen[j * screenWidth + i] = shade;
				}
			}
		}
		// Draw map
		for (int mx = 0; mx < mapWidth; mx++) {
			for (int my = 0; my < mapHeight; my++) {
				screen[my * screenWidth + mx] = map[my * mapWidth + mx];
			}
		}
		// Get player vector
		float vecX = cosf(playerA);
		float vecY = -sinf(playerA);
		// Draw player angle
		screen[(int)(vecY + (int)playerY + 0.5) * screenWidth + (int)(vecX + (int)playerX + 0.5)] = 'X';
		// Draw player position
		screen[(int)playerY * screenWidth + (int)playerX] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0, 0 }, &dwBytesWritten);
	}
}