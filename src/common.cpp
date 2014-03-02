#include "../include/common.h"

void Circle::calculate_points(const Point& centre, unsigned int radius) {
	Point p(radius, 0);
	int radius_error = 1 - p.x;

	points.clear();		// clear older points

	while (p.x >= p.y) {
		points.push_back(Point(p.x + centre.x, p.y + centre.y));
		points.push_back(Point(p.y + centre.x, p.x + centre.y));
		points.push_back(Point(-p.x + centre.x, p.y + centre.y));
		points.push_back(Point(-p.y + centre.x, p.x + centre.y));
		points.push_back(Point(-p.x + centre.x, -p.y + centre.y));
		points.push_back(Point(-p.y + centre.x, -p.x + centre.y));
		points.push_back(Point(p.x + centre.x, -p.y + centre.y));
		points.push_back(Point(p.y + centre.x, -p.x + centre.y));

		p.y++;
		if (radius_error < 0) {
			radius_error += (p.y * 2) + 1;
		}
		else {
			p.x--;
			radius_error += ((p.y - p.x + 1) * 2);
		}
	}
}

Vec2 operator - (const Vec2& a, const Vec2& b) {
	return Vec2 (a.x - b.x, a.y - b.y);
}

Vec2 operator - (const Vec2& a, float c) {
	return Vec2 (a.x - c, a.y - c);
}

Vec2 operator - (const Vec2& a) {
	return Vec2 (-a.x, -a.y);
}

Vec2 operator -= (Vec2& a, const Vec2& b) {
	return a = a-b;
}

Vec2 operator + (const Vec2& a, const Vec2& b) {
	return Vec2 (a.x + b.x, a.y + b.y);
}

Vec2 operator + (const Vec2& a, float c) {
	return Vec2 (a.x + c, a.y + c);
}

Vec2 operator += (Vec2& a, const Vec2& b) {
	return a = a+b;
}

Vec2 operator * (const Vec2& a, float f) {
	return Vec2 (a.x * f, a.y * f);
}

Vec2 operator / (const Vec2& a, float f) {
	return Vec2 (a.x / f, a.y / f);
}

bool operator == (const Vec2& a, const Vec2& b) {
	return a.x == b.x && a.y == b.y;
}

float dot_product (const Vec2& a, const Vec2& b) {
	return (a.x * b.x + a.y * b.y);
}
