#ifndef TRANS_PHYSICS_HEAD
#define TRANS_PHYSICS_HEAD

#include "../include/common.h"

typedef unsigned long ID;
struct ID_gen {
	static ID next_ID;

	static ID get_ID () {
		return next_ID++;
	}
};

struct Shape;

const float GRAVITY_DEFAULT = 1.0f;
const float MASS_EPSILON = 1.0e-10;
const float VELOCITY_EPSILON = 0.5f;

const Vec2 GRAVITY_ACCELERATION = Vec2(0, 10.0f);
const Vec2 FRICTION_EPSILON = Vec2(1.0e-15, 1.0e-15);
const Vec2 IMPULSE_EPSILON = Vec2(1.0e-15, 1.0e-15);

struct Body {
	Body () : id (ID_gen::get_ID ())
	{	}

	ID id;

	Shape *shape;
	float inv_mass;
	float restitution;

	float static_friction;
	float dynamic_friction;
	
	Vec2 pos;
	Vec2 velocity;
	Vec2 force;
	
	float gravity_scale;

	void integrate_forces (float dt);
	void integrate_velocity (float dt);
};

struct Shape {
	enum Type { BOX, CIRCLE, SIZE };

	Type type;
	unsigned layer;
	Body* body;

	Shape() :
		layer(1),
		body(nullptr)
	{	}
	virtual void move () = 0;
};

struct Bounding_Circle : public Shape {
	float rad;

	Bounding_Circle () {
		type = Shape::CIRCLE;
 		rad = 0.0f;
	}

	Bounding_Circle (float radius) {
		type = Shape::CIRCLE;
		rad = radius;
	}

	void move () {	}
};

struct Bounding_Box: public Shape {
	Vec2 min, max;

	Bounding_Box () {
		type = Shape::BOX;
	}

	Bounding_Box (Vec2 top_left, Vec2 bottom_right) {
		type = Shape::BOX;
		min = top_left;
		max = bottom_right;
	}

	void move () {
		const register float ex = (max.x-min.x)/2;
		const register float ey = (max.y-min.y)/2;

		min = Vec2(body->pos.x-ex, body->pos.y-ey);
		max = Vec2(body->pos.x+ex, body->pos.y+ey);
	}

};

struct Manifold {
	Body* a;
	Body* b;
	float penetration;
	Vec2 normal;

	Manifold () : a (nullptr), b (nullptr), penetration (0.0f), normal ()
	{	}

	Manifold (Body* body1, Body* body2): a (body1), b (body2), penetration (0.0f), normal ()
	{	}

	void resolve_collision ();
	void positional_correction ();
};

struct Scene {
	Scene ()
	{	}

	void add_body (Body* b) {
		bodies[b->id] = b;
	}

	void remove_body (Body* b) {
	#ifdef DEBUG_MODE
		if (!bodies.erase(b->id))
			cerr<<"Scene::"__func__<<": trying to erase non-member\n";
	#else
		bodies.erase (b->id);
	#endif
	}

	void step (float dt);

private:

	map<ID, Body*> bodies;
	typedef map<ID, Body*>::iterator Body_iterator;
};

#endif
