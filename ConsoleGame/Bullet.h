#pragma once

#include "Object.h"

class Bullet : public Object {
private:
	float vx;
	float vy;
public:
	Bullet(float x, float y, float vx, float vy) : Object(x, y), vx(vx), vy(vy) {};
	bool move(Bullet& b, float elapsed);
	float getVX();
	float getVY();
	virtual void draw(Object& b, float* depthBuffer, wchar_t* screen);
};