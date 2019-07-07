#pragma once

#include "Bullet.h"
#include "Target.h"

#define M_PI 3.141592

static constexpr int screenWidth = 240;
static constexpr int screenHeight = 80;

static int mapWidth = 16;
static int mapHeight = 16;

// Player position (X, Y) and angle
static float playerX = 9.0f;
static float playerY = 7.0f;
static float playerA = M_PI;

// Field of view
static float fov = M_PI / 4.0f;

// Raytracing resolution
static float res = 0.1f;

// Max rendering depth
static float dof = 20.0f;

// Player speed
static float playerSpeed = 4.0f;

// Bullet speed
static float bulletSpeed = 5.0f;

// Boundary width
static float bWidth = 0.99995f;

static const float PRECISION = 0.09f;

struct vector {
	float vx;
	float vy;
	float angle;
};

bool checkMapBoundary(float x, float y);

bool findWallCollision(float x, float y);

bool findTargetCollision(const Bullet& b);

vector getAngleFromPlayer(float x, float y);

bool checkWallBoundary(float currX, float currY, float rayX, float rayY);

short getWallShade(float dist);

char getFloorShade(float dist);

void movePlayer(float movX, float movY);