#include "../include/physics.h"

//	Initialize the ID generator counter to 0
ID ID_gen::next_ID = 0;

void Manifold::resolve_collision () {
	Vec2 rv = b->velocity - a->velocity;	// the relative velocity vector

	register float rv_normal = dot_product (rv, normal);	// calculate the magnitude of relative velocity along the collision normal
	if (rv_normal > 0)
		return;		// doesn't need resolution as both objects are moving in same direction and will get resolved after next step

	register float sum_inv_mass = a->inv_mass + b->inv_mass;

	float impulse_magnitude = (-(1 + min (a->restitution, b->restitution)) * rv_normal / sum_inv_mass);
	Vec2 impulse = normal * impulse_magnitude;

	Vec2 tangent = rv - normal * dot_product(rv, normal);
	tangent.normalize();

	float friction_magnitude = -dot_product(rv, tangent) / sum_inv_mass;
	Vec2 friction_impulse;

	float static_friction = sqrt(a->static_friction * a->static_friction + b->static_friction * b->static_friction);
	if (abs(friction_magnitude) < impulse_magnitude * static_friction) {
		friction_impulse = tangent * friction_magnitude * static_friction;
	}
	else {
		friction_impulse = -tangent * impulse_magnitude * sqrt(a->dynamic_friction * a->dynamic_friction + b->dynamic_friction * b->dynamic_friction);
	}
	if (abs(friction_impulse) < FRICTION_EPSILON)	// don't apply impulse below the limit
		friction_impulse.set (0.0f, 0.0f);
	if (abs(impulse) < IMPULSE_EPSILON)
		impulse.set (0.0f, 0.0f);

/*
	m.a->velocity -= impulse * m.b->inv_mass / sum_inv_mass;
	m.b->velocity += impulse * m.a->inv_mass / sum_inv_mass;
*/
	a->velocity -= (impulse + friction_impulse) * a->inv_mass; 
	b->velocity += (impulse + friction_impulse) * b->inv_mass;
}

void Manifold::positional_correction () {
	const float ratio = 0.6f;
	const float slop = 0.05f;
	Vec2 correction = normal * max (penetration - slop, 0.0f ) / (a->inv_mass + b->inv_mass) * ratio;
	a->pos -= correction * a->inv_mass;
	b->pos += correction * b->inv_mass;
}

void Body::integrate_forces (float dt) {
	velocity += (force * inv_mass + GRAVITY_ACCELERATION * gravity_scale) * (dt / 2.0f);
}

void Body::integrate_velocity (float dt) {
	pos += velocity * dt;
	shape->move();
	integrate_forces(dt);
}

bool collision_circle_circle(Manifold& m) {
	float ra = ((Bounding_Circle*)m.a->shape)->rad;
	float rb = ((Bounding_Circle*)m.b->shape)->rad;

	// get the normal
	Vec2 n = m.b->pos - m.a->pos;
	float r = ra + rb;

	// check collision
	if (dot_product(n, n) >= r*r)
		return false;

	float d = n.magnitude();
	if (d != 0) {		// distance b/w circles
		m.penetration = r - d;
		m.normal = n / d;

		return true;
	}
	else {	// concentric circles
		m.penetration = min(ra, rb);
		m.normal.set(1.0f, 0.0f);

		return true;
	}
}

bool collision_box_box(Manifold& m) {
	Bounding_Box* ba = (Bounding_Box*)(m.a->shape);
	Bounding_Box* bb = (Bounding_Box*)(m.b->shape);

	// get the normal
	Vec2 n = m.b->pos - m.a->pos;

	// side lengths
	float a_extent = (ba->max.x - ba->min.x) / 2;
	float b_extent = (bb->max.x - bb->min.x) / 2;

	// get the overlap length
	float x_overlap = a_extent + b_extent - abs(n.x);
	if (x_overlap > 0) {		// overlapping in x direction
		// get the y-extents
		a_extent = (ba->max.y - ba->min.y) / 2;
		b_extent = (bb->max.y - bb->min.y) / 2;

		float y_overlap = a_extent + b_extent - abs(n.y);
		if (y_overlap > 0) {	// overlapping in both axes
			if (x_overlap < y_overlap) {
				if (n.x < 0)
					m.normal.set(-1.0f, 0.0f);
				else
					m.normal.set(1.0f, 0.0f);
				m.penetration = x_overlap;

				return true;
			}
			else {
				if (n.y < 0)
					m.normal.set(0.0f, -1.0f);
				else
					m.normal.set(0.0f, 1.0f);
				m.penetration = y_overlap;

				return true;
			}
		}
	}
	return false;
}

bool collision_box_circle(Manifold& m) {
	Bounding_Box* ba = (Bounding_Box*)m.a->shape;
	float r = ((Bounding_Circle*)m.b->shape)->rad;

	// get the direction vector
	Vec2 n = m.b->pos - m.a->pos;
	// set the closest point b/w circle to box distance b/w centres for now
	Vec2 closest = n;

	// extents of box (refers to box when centered to origin)
	float x_extent = (ba->max.x - ba->min.x)/2;
	float y_extent = (ba->max.y - ba->min.y)/2;

	// clamp the closest point to within the box extents
	closest.x = CLAMP (-x_extent, x_extent, closest.x);
	closest.y = CLAMP (-y_extent, y_extent, closest.y);

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

	if (d > r*r && !inside)
		return false;

	d = sqrt(d);

	if (inside) {
		m.normal = -normal / d;
		m.penetration = r+d;
	}

	else {
		m.normal = normal / d;
		m.penetration = r-d;
	}

	return true;
}

bool collision_circle_box(Manifold& m) {
	Body* temp = m.a;
	m.a = m.b;
	m.b = temp;
	return collision_box_circle(m);
}

bool (*collision_callback[Shape::Type::SIZE][Shape::Type::SIZE])(Manifold&) = {
	collision_box_box,
	collision_box_circle,
	collision_circle_box,
	collision_circle_circle
};

void Scene::step (float dt) {
	Body_iterator iter;
	Body_iterator iter2;

	for (iter = bodies.begin(); iter != bodies.end(); ++iter) {
		iter->second->integrate_forces(dt);
		iter->second->integrate_velocity(dt);
		iter->second->force.set(0.0f, 0.0f);
	}

	for (iter = bodies.begin(); iter != bodies.end(); ++iter)
		for (iter2 = iter, ++iter2; iter2 != bodies.end(); ++iter2) {
			if (!(iter->second->shape->layer & iter2->second->shape->layer))
				continue;
			
			Manifold m(iter->second, iter2->second);
			if ((*collision_callback [iter->second->shape->type][iter2->second->shape->type])(m)) {
				m.resolve_collision();
				m.positional_correction();
			}
		}
}
