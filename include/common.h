#ifndef TRANS_COMMON_HEAD
#define TRANS_COMMON_HEAD

#include <SDL.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>

using namespace std;

#define CLAMP(a, b, x) min(max((a), (x)), (b))

/*	Point : wrapper for SDL_Point	*/
struct Point {
	Point() : x(0), y(0)
	{	}

	Point(int xx, int yy) : x(xx), y(yy)
	{	}

	SDL_Point point() const {
		SDL_Point p = { x, y };
		return p;
	}

	int x;
	int y;
};

struct Vec2 {
	float x, y;

	Vec2() : x(0.0f), y(0.0f)
	{	}

	Vec2(float i, float j) : x(i), y(j)
	{	}

	float magnitude() const {
		return sqrt(x*x + y*y);
	}

	void normalize() {
		if (x == 0.0f && y == 0.0f)
			return;
		register float mag = magnitude();
		set(x/mag, y/mag);
	}

	void set(float i, float j) {
		x = i;
		y = j;
	}
};

Vec2 operator - (const Vec2& a, const Vec2& b);

Vec2 operator - (const Vec2& a, float c);

Vec2 operator - (const Vec2& a);

Vec2 operator -= (Vec2& a, const Vec2& b);

Vec2 operator + (const Vec2& a, const Vec2& b);

Vec2 operator + (const Vec2& a, float c);

Vec2 operator += (Vec2& a, const Vec2& b);

Vec2 operator * (const Vec2& a, float f);

Vec2 operator / (const Vec2& a, float f);

bool operator == (const Vec2& a, const Vec2& b);

float dot_product (const Vec2& a, const Vec2& b);

struct Circle {
	vector<Point> points;

	Circle()
	{	}

	void calculate_points(const Point& centre, unsigned int radius);

	Point* data() {
		return points.data();
	}

	unsigned size() const {
		return points.size();
	}
};

#endif
