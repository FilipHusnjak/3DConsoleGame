#pragma once

class Object {
public:
	Object(float x, float y) : x(x), y(y) {};
	float x;
	float y;
	bool remove = false;
	virtual void draw(Object& b, float* depthBuffer, wchar_t* screen) = 0;
};