#include <cmath>

using namespace std;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define CLAMP(a, b, x) MIN(MAX((a), (x)), (b))

struct Vec2 {
	float x, y;

	Vec2() : x(0.0f), y(0.0f)
	{	}

	Vec2(float i, float j) : x(i), y(j)
	{	}

	float magnitude() const {
		return sqrt(x*x + y*y);
	}
};

Vec2 operator -(const Vec2& a, const Vec2& b) {
	return Vec2(a.x - b.x, a.y - b.y);
}

Vec2 operator -(const Vec2& a, float c) {
	return Vec2(a.x - c, a.y - c);
}

Vec2 operator -(const Vec2& a) {
	return Vec2(-a.x, -a.y);
}

Vec2 operator -=(Vec2& a, const Vec2& b) {
	return a = a-b;
}

Vec2 operator +(const Vec2& a, const Vec2& b) {
	return Vec2(a.x + b.x, a.y + b.y);
}

Vec2 operator +(const Vec2& a, float c) {
	return Vec2(a.x + c, a.y + c);
}

Vec2 operator += (Vec2& a, const Vec2& b) {
	return a = a+b;
}

Vec2 operator *(const Vec2& a, float f) {
	return Vec2(a.x * f, a.y * f);
}

Vec2 operator /(const Vec2& a, float f) {
	return Vec2(a.x / f, a.y / f);
}

bool operator ==(const Vec2& a, const Vec2& b) {
	return a.x == b.x && a.y == b.y;
}

float dot_product(const Vec2& a, const Vec2& b) {
	return (a.x * b.x + a.y * b.y);
}


struct Bounding_Box {
	Vec2 min;	// top left corner
	Vec2 max;	// bottom right corner

	Bounding_Box() : min(), max()
	{	}

	Bounding_Box(Vec2 m1, Vec2 m2) : min(m1), max(m2)
	{	}
};

struct Bounding_Circle {
	Vec2 cpos;	// position of centre
	float rad;	// radius

	Bounding_Circle() : cpos(), rad(0.0f)
	{	}

	Bounding_Circle(Vec2 centre, float radius) : cpos(centre), rad(radius)
	{	}
};

struct Object {
	Vec2 pos;
	Vec2 velocity;
	float inv_mass;
	float restitution;

	Object() : pos(), velocity(), inv_mass(0.0), restitution(1.0)
	{	}

	virtual void move() {
		pos += velocity;
	}
};

struct Circle : public Object, Bounding_Circle {
// keep cpos and pos equal
	Circle(Vec2 position, float radius) : Object(), Bounding_Circle(position, radius)
	{
		pos = position;
	}

	virtual void move() {
		pos += velocity;
		cpos = pos;
	}
};

struct Box : public Object, Bounding_Box {
// pos equals the centre point of the Bounding_Box i.e. (min.x+max.x)/2, (min.y+max.y)/2
	Box()
	{	}

	Box(Vec2 m1, Vec2 m2) : Object(), Bounding_Box(m1, m2)
	{
		pos = Vec2((min.x+max.x)/2, (min.y+max.y)/2);
	}

	virtual void move() {
		const register float ex = (max.x-min.x)/2;
		const register float ey = (max.y-min.y)/2;

		pos += velocity;
		min = Vec2(pos.x-ex, pos.y-ey);
		max = Vec2(pos.x+ex, pos.y+ey);
	}
};

struct Manifold {
	Object* a;
	Object* b;
	float penetration;
	Vec2 normal;

	Manifold() : a(nullptr), b(nullptr), penetration(0.0f), normal()
	{	}

	Manifold(Object* obja, Object* objb): a(obja), b(objb), penetration(0.0f), normal()
	{	}
};

bool collsion_circle_circle(Manifold& m) {
	Circle* a = (Circle*)m.a;
	Circle* b = (Circle*)m.b;

	// get the normal
	Vec2 n = b->pos - a->pos;
	float r = a->rad + b->rad;

	// check collision
	if (dot_product(n, n) > r*r)
		return false;

	float d = n.magnitude();
	if (d != 0) {		// distance b/w circles
		m.penetration = r - d;
		m.normal = n / d;

		return true;
	}
	else {	// concentric circles
		m.penetration = MIN(a->rad, b->rad);
		m.normal = Vec2(1.0f, 0.0f);

		return true;
	}
}

bool collision_box_box(Manifold& m) {
	Box* a = (Box*)(m.a);
	Box* b = (Box*)(m.b);

	// get the normal
	Vec2 n = b->pos - a->pos;

	// side lengths
	float a_extent = (a->max.x - a->min.x) / 2;
	float b_extent = (b->max.x - b->min.x) / 2;

	// get the overlap length
	float x_overlap = a_extent + b_extent - abs(n.x);
	if (x_overlap > 0) {		// overlapping in x direction
		// get the y-extents
		a_extent = (a->max.y - a->min.y) / 2;
		b_extent = (b->max.y - b->min.y) / 2;

		float y_overlap = a_extent + b_extent - abs(n.y);
		if (y_overlap > 0) {	// overlapping in both axes
			if (x_overlap < y_overlap) {
				if (n.x < 0)
					m.normal = Vec2(-1.0f, 0.0f);
				else
					m.normal = Vec2(1.0f, 0.0f);
				m.penetration = x_overlap;

				return true;
			}
			else {
				if (n.y < 0)
					m.normal = Vec2(0.0f, -1.0f);
				else
					m.normal = Vec2(0.0f, 1.0f);
				m.penetration = y_overlap;

				return true;
			}
		}
	}
	return false;
}

bool collision_box_circle(Manifold& m) {
	Box* a = (Box*)m.a;
	Circle* b = (Circle*)m.b;

	// get the direction vector
	Vec2 n = b->pos - a->pos;
	// set the closest point b/w circle to box distance b/w centres for now
	Vec2 closest = n;

	// extents of box (refers to box when centered to origin)
	float x_extent = (a->max.x - a->min.x)/2;
	float y_extent = (a->max.y - a->min.y)/2;

	// clamp the closest point to within the box extents
	closest.x = CLAMP(-x_extent, x_extent, closest.x);
	closest.y = CLAMP(-y_extent, y_extent, closest.y);

	bool inside = false;

	// closest point is within the box, clamp closest point to nearest edge
	if (closest == n) {
		inside = true;
		if (abs(n.x) > abs(n.y)) {
			closest.x = closest.x > 0 ? x_extent : -x_extent; // clamp to x-axis edge
		}
		else {
			closest.y = closest.y > 0 ? y_extent : -y_extent; // clamp to y-axis edge
		}
	}

	Vec2 normal = n - closest;
	float d = dot_product(normal, normal);
	float r = b->rad;

	if (d > r*r && !inside)
		return false;

	d = sqrt(d);

	if (inside) {
		m.normal = -n;
		m.penetration = r+d;
	}

	else {
		m.normal = n;
		m.penetration = r-d;
	}

	return true;
}

void resolve_collision(Manifold& m) {
	Vec2 rv = m.b->velocity - m.a->velocity;	// the relative velocity vector

	register float rv_normal = dot_product(rv, m.normal);	// calculate the magnitude of relative velocity along the collision normal
	if (rv_normal > 0)
		return;		// doesn't need resolution as both objects are moving in same direction and will get resolved after next step

	register float sum_inv_mass = m.a->inv_mass + m.b->inv_mass;
	Vec2 impulse = m.normal * (-(1 + MIN(m.a->restitution, m.b->restitution)) * rv_normal / sum_inv_mass);
/*
	m.a->velocity -= impulse * m.b->inv_mass / sum_inv_mass;
	m.b->velocity += impulse * m.a->inv_mass / sum_inv_mass;
*/
	m.a->velocity -= impulse * m.a->inv_mass;
	m.b->velocity += impulse * m.b->inv_mass;
}

void positional_correction(Manifold& m)
{
	const float ratio = 0.9;
	const float slop = 0.01;
	Vec2 correction = m.normal * MAX(m.penetration - slop, 0.0f ) / (m.a->inv_mass + m.b->inv_mass) * ratio;
	m.a->pos -= correction * m.a->inv_mass;
	m.b->pos += correction * m.b->inv_mass;
}
