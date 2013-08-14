#include <stdio.h>
#include <iostream>
#include <cmath>

#include "player.h"
#include "world.h"

#include "geometry.h"

#include "tunables.h"

void Player::reset(const Vec2 &origin) {

	velocity.x = velocity.y = 0;
	target_velocity.x = target_velocity.y = 0;
	f_accum.x = f_accum.y = 0;
	if (contact_surface) {
		delete contact_surface;
		contact_surface = NULL;
	}
	position = origin;

	hook.deactivate();
	hook_nodes.clear();

	release_window = pull = bounce = 0;

	return;
}

Player::~Player() {
	if (contact_surface) {
		delete contact_surface;
		contact_surface = NULL;
	}
}

const Vec2 &Player::node_2() const {
	std::list<Vec2>::const_iterator iter= hook_nodes.begin();
	iter++;
	return *iter;
}

void Player::rope_retract(float ts) {

	if (hook.stuck) { // player being pulled towards hook
		const Vec2 &pull_point = node_2();
		Vec2 rope_vec = pull_point - position;
		rope_vec.normalize();
		float pull_speed = vec2_project(velocity, rope_vec);
		if (pull_speed < PULL_MAX_SPEED) {
			push(rope_vec.x * PULL_FORCE, rope_vec.y * PULL_FORCE);
		}
		// TODO: cap the max speed
	} else { // hook being retracted
		// Given that this is basically cosmetic, it's a lot of work!
		std::list<Vec2>::const_reverse_iterator iter = hook_nodes.rbegin();
		iter++;
		Vec2 rope_vec = *iter - hook.position;
		//rope_vec points from the hook back at the last node
		rope_vec.normalize();

		Vec2 hook_motion = hook.velocity;

		// Braking
		// Insert player's velocity as outward movement, so the end gets dragged around correctly
		{
			const Vec2 &pull_point = node_2();
			Vec2 pull_vec = position - pull_point;

			// Could probably simplify this by just adding to the other outward_ calculation
			float outward_component = vec2_project(velocity, pull_vec);
			if (outward_component > 0) {
				hook_motion -= rope_vec * outward_component;
			}
		}
		// Actual calculation
		float outward_component = vec2_project(hook_motion, rope_vec);

		if (outward_component < 0) {
			hook.velocity -= rope_vec * outward_component;
		}

		// Completely elimininates flopping around
		const Vec2 &rejection = vec2_reject(hook.velocity, rope_vec);
		hook.velocity -= rejection;

		hook.velocity += rope_vec * RETRACT_FORCE * ts;
	}
}

void Player::rope_brake() {
	const Vec2 &pull_point = node_2();
	Vec2 pull_vec = position - pull_point;

	float outward_component = vec2_project(velocity, pull_vec);
	if (outward_component > 0) {
		pull_vec.normalize();
		velocity += pull_vec * (outward_component * -1);
	}
}

void Player::merge_movement(float ts) {
// Borrowed from Quake 3
	if (!contact_surface) {
		if (target_velocity.x == 0 && target_velocity.y == 0) {
			// just drift
			return;
		}
		// Yes, diagonal movement is faster.
		velocity += target_velocity * ts;
		return;
	}

	// On ground, no vertical control (jump is separate)
	// x is basically vector along the surface, rather than actual x
	// Assuming axis-aligned geometry... don't feel like fixing this up to the general case for now
	float diff = target_velocity.x - velocity.x;
	if (fabs(diff) > ts * PLAYER_ACCELERATION) {
		diff = copysign(ts*PLAYER_ACCELERATION, diff);
	}
	velocity.x += diff;
}

void Player::update(float ts) {
	//std::cout<< "-----FRAME START------";

	merge_movement(ts);

	velocity.y -= GRAVITY * ts;
	// if player is standing on a flat surface, don't let the rope pull them through it
	if (contact_surface && f_accum.y < 0)
		f_accum.y = 0;

	velocity += f_accum * ts;
	f_accum.x = f_accum.y = 0;

	check_collisions(ts);
	position += velocity * ts;

	if (hook.is_active()) {
		update_hook(ts);
	}
	return;

}

void Player::check_collisions(float ts) {
	// Attempt at surface-tracking physics
	// Collision with a surface with a slope less than 45 degrees off flat -> record to contact_surface
	// Run collision as usual (2- or 3- vector scans). If collision with something collinear with
	// contact_surface -> just follow movement
	// any other surface -> run old-style collision
	Vec2 velocity_n = velocity;
	velocity_n.normalize();

	Vec2 next_pos = position + velocity * ts;
	next_pos += velocity_n * COLLISION_RANGE;
	Line movement(position, next_pos);

	std::auto_ptr<std::pair<Line, Vec2> > collision = world.collide_line(movement);
	Vec2 offset(0, 0);
	if (!collision.get()) {
		offset = Vec2(-velocity_n.y, velocity_n.x);
		offset *= COLLISION_RANGE;
		collision = world.collide_line(Line(position + offset, next_pos + offset));
	}
	if (!collision.get()) {
		offset *= -1;
		collision = world.collide_line(Line(position + offset, next_pos + offset));
	}

	// Another special case that hasn't been dealt with:
	// This only accounts for a single collision per frame
	// running at a wall therefore causes the ground tracking to be lost, and the player pops up a bit

	if (collision.get()) {
		// Special case
		// If the player is sliding upwards or downwards on a vertical
		// surface, the collision will trip as they hit the top or bottom of the edge
		// (i.e. hitting the inside of the block). Smooth it over

		if (collision->first.y1 == collision->first.y2 && (
					collision->second.x == collision->first.x1 || collision->second.x == collision->first.x2)) {
			// Collision? What collision?
			return;
		}

		if (bounce) {
			vec2_bounce(collision->first, velocity);
			// TODO: Cut or cap velocity
			// TODO: adjust position a bit to account for distance covered before and after the bounce
		} else {
			if (contact_surface) {
				//Collinear -> keep, push
				//otherwise -> delete it, fall through

				// Uhh... parallel is close enough to collinear for this work, right?
				Vec2 contact_vec(contact_surface->x2 - contact_surface->x1,
					contact_surface->y2 - contact_surface->y1);

				float m1 = contact_vec.y / contact_vec.x;
				float m2 = (collision->first.y2 - collision->first.y1) /
					(collision->first.x2 - collision->first.x1);

				if (m1 != m2) {
					delete contact_surface;
					contact_surface = NULL;
				} else {
					// axis-aligned cheat...
					if (velocity.y < 0)
						velocity.y = 0;
					//velocity.y += GRAVITY * ts;
				}
			}
			// not _else_ because the above can delete it
			if (!contact_surface) {

				Vec2 collision_angle = Vec2(collision->first.y2 - collision->first.y1,
					collision->first.x2 - collision->first.x1);
				//float vy_before = velocity.y;

				// Vector rejection -> sliding!
				velocity = vec2_reject(velocity, collision_angle);

				// Push back from the collision point by COLLISION_RANGE
				Vec2 normal(collision->first.y1 - collision->first.y2,
						collision->first.x2 - collision->first.x1);
				normal.normalize(); //normalized vector normal

				position = collision->second - normal * COLLISION_RANGE;

				// If collision surface is less than 45 degrees off flat, store it as contact_surface (i.e.
				// walkable surface)
				// _and_ the player is above the contact point (i.e. not smacking something from below)
				if (position.y > collision->second.y && velocity.y <= 0 && 
						fabs(collision->first.y2 - collision->first.y1) < fabs(collision->first.x2 - collision->first.x1)) {
					contact_surface = new Line(collision->first);
				}

				// Not going to work quite right with non-perpendicular geometry
				/*if (vy_before < 0 && velocity.y == 0) {
					onground = 1;
				} */

				//friction applies to both ground movement and sliding _into_ blocks
			}
			velocity -= velocity*1*ts;
		}
	} else {
		if (contact_surface) {
			delete contact_surface;
			contact_surface = NULL;
		}
	}

}

void Player::update_hook(float ts) {
	// Trim excess nodes first
	if (!release_window && hook_nodes.size() == 2 &&
			dist2(*hook_nodes.begin(), node_2()) < NODE_MIN_DISTANCE) {
		hook.deactivate();
		hook_nodes.clear();
		return;
	}

	if (pull) {
		rope_retract(ts);
	}
	if (hook.stuck) {
		rope_brake();
	}
	std::list<Vec2>::iterator iter = hook_nodes.begin();
	iter++;
	if (dist2(hook_nodes.front(), *iter) < NODE_MIN_DISTANCE) {
		if (!release_window) {
			hook_nodes.erase(iter);
		}
		if (hook_nodes.size() < 2) {
			std::cout << "Deleted last node!\n";
			// actually an error condition
		}
	} else {
		release_window = 0;
	}

	if (!hook.stuck) {
		// reverse iterators are confusing
		//std::list<Vec2>::reverse_iterator iter2 = hook_nodes.rbegin();
		//iter2++; // .back()
		std::list<Vec2>::iterator iter2 = hook_nodes.end();
		iter2--;
		iter2--;
		if (!release_window && dist2(*iter2, hook_nodes.back()) < NODE_MIN_DISTANCE) {
			//hook_nodes.erase(--iter2.base());
			hook_nodes.erase(iter2);
		}
	}

	if (hook_nodes.size() > 2) {
		unwrap_rope();
	}
	wrap_rope();

	hook.update(ts);
	hook_nodes.front() = position;
	hook_nodes.back() = hook.get_position();

	// Length limiting
	float length = 0;

	std::list<Vec2>::const_iterator i = hook_nodes.begin();
	const Vec2 *last = &(*i);
	for (;i != hook_nodes.end();++i) {
		length += hypot(*i - *last);
		last = &(*i);
	}
	if (length > ROPE_LENGTH) {
		// requires that the hook is still active
		// The only deactivation condition is running out of length, which is checked first off
		trigger(0, 0);
	}
}

void Player::wrap_rope() {

	std::list<Vec2>::iterator iter = hook_nodes.begin();

	iter++;
	Line segment(hook_nodes.front(), *iter);
	std::auto_ptr<Vec2> collision = world.collide_corner(segment);
	if (collision.get()) {
		hook_nodes.insert(iter, *collision);
		rope_angle_player = node_angle(hook_nodes.begin());
		/*std::cout << "Collided is now " << hook_nodes.size() << " points\n";
		std::cout << "Angle: " << rope_angle_player << "\n";
		*/
	}
}

// Calculate the angle between the next 3 nodes under the iterator
float Player::node_angle(std::list<Vec2>::const_iterator iter) const {
	const Vec2 &node1 = *iter++;
	const Vec2 &node2 = *iter++;
	const Vec2 &node3 = *iter;

	Vec2 seg1 = node1 - node2;
	Vec2 seg2 = node3 - node2;

	return angle_diff(seg1, seg2);
}

void Player::unwrap_rope() {
	// Player end
	std::list<Vec2>::iterator iter = hook_nodes.begin();
	float angle = node_angle(iter);

	if (copysign(rope_angle_player, angle) != rope_angle_player) {
		// unwrap!
		iter++;
		hook_nodes.erase(iter);
		if (hook_nodes.size() > 2) {
			rope_angle_player = node_angle(hook_nodes.begin());
		}
	}
	// Rope retraction is now perfectly linear, so no unwrapping on the other end
}

void Player::control(float x, float y) {
#if 1
	if (contact_surface) {
		target_velocity.y = 0;
		target_velocity.x = x * PLAYER_GROUND_SPEED;
	} else {
		target_velocity.x = x * PLAYER_AIR_SPEED;
		target_velocity.y = y * PLAYER_AIR_SPEED;
	}

#elif 0
	velocity.x += x * PLAYER_SPEED_X;
	velocity.y += y * PLAYER_SPEED_Y;
#else
	velocity.y = y * PLAYER_SPEED_X;
	velocity.x = x * PLAYER_SPEED_X;
#endif
}

void Player::jump(int value) {
	if (value && contact_surface) {
		//Axis-aligned surfaces only. Should actually be surface normal
		velocity.y = 300;
	} else {
		// I'm kind of surprised gcc doesn't say anything about the type mismatch...
		bounce = value;
	}
}

void Player::trigger(float x, float y) {
	if (hook.is_active()) {
		hook.release();
		// pull in hook
		pull = 1;
		hook.velocity.x = 0;
		hook.velocity.y = 0;
	} else {
		Vec2 aim(x, y);
		hook.launch(position, aim - position);
		hook_nodes.push_back(position);
		hook_nodes.push_back(position);
		pull = 0;
		release_window = 1;
	}
	return;
}

void Player::retract(int value) {
	if (hook.stuck) {
		pull = value;
	}
}

void Player::push(float x, float y) {
	f_accum.x += x;
	f_accum.y += y;
}

const Vec2 &Player::get_position() const{
	return position;
}

const std::list<Vec2> &Player::get_rope_path() const {
	return hook_nodes;
}

