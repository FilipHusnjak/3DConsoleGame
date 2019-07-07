#pragma once

#include "Object.h"

class Target : public Object {
public:
	Target(float x, float y) : Object(x, y) {};
	virtual void draw(Object& b, float* depthBuffer, wchar_t* screen);
};