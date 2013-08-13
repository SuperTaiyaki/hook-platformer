#include <stdio.h>
#include <iostream>
#include <cmath>

#include "player.h"
#include "world.h"

#include "geometry.h"

#include "tunables.h"

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
		push(rope_vec.x * PULL_FORCE, rope_vec.y * PULL_FORCE);
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
#if 0
	// cases:
	// from 0 -> PLAYER_ACCELERATIION
	// same direction, increasing -> PLAYER_ACCELERATION
	// same direction, decreasing -> PLAYER_BRAKING
	// opposite direction -> PLAYER_BRAKING
	print_vec2("Target velocity", target_velocity);
	//for (int axis = 0; axis < 2; axis++) {
	for (int axis = 0; axis < 1; axis++) {
		if (velocity[axis] == target_velocity[axis]) {
			std::cout << "No speed correction\n";
			continue;
		}
		if (velocity[axis] != 0 && copysign(target_velocity[axis], velocity[axis]) !=
				target_velocity[axis]) {
			// Braking - more force
			std::cout << "Reverse braking\n";
			velocity[axis] += copysign(PLAYER_BRAKING*ts, target_velocity[axis]);
			if (fabs(velocity[axis]) < fabs(target_velocity[axis])) {
				velocity[axis] = target_velocity[axis];
			}
		} else {
			if (fabs(velocity[axis]) < fabs(target_velocity[axis])) {
				std::cout << "standard accel\n";
				velocity[axis] += copysign(PLAYER_ACCELERATION * ts, target_velocity[axis]);
				if (fabs(velocity[axis]) > fabs(target_velocity[axis])) {
					velocity[axis] = target_velocity[axis];
				}
			} else {
				std::cout << "overspeed deceleration\n";
				velocity[axis] -= copysign(PLAYER_BRAKING * ts, velocity[axis]);
				// different sign from above!
				// Urgh, need to account for crossing 0...
				if (fabs(velocity[axis]) < fabs(target_velocity[axis])) {
					velocity[axis] = target_velocity[axis];
				}
			}

		}
	}
#else
// Borrowed from Quake 3
	if (!onground) {
		if (target_velocity.x == 0 && target_velocity.y == 0) {
			// just drift
			return;
		}
		// Yes, diagonal movement is faster.
		velocity += target_velocity * ts;
		return;
	}
	
	// On ground, no vertical control (jump is separate)
	float diff = target_velocity.x - velocity.x;
	if (fabs(diff) > ts * PLAYER_ACCELERATION) {
		// one-dimensional... 
		diff = copysign(ts*PLAYER_ACCELERATION, diff);
	}
	velocity.x += diff;

#endif
}

void Player::update(float ts) {

	merge_movement(ts);

	velocity.y -= GRAVITY * ts;
	velocity += f_accum * ts;
	f_accum.x = f_accum.y = 0;

	if (velocity.x || velocity.y) {
		Vec2 velocity_n = velocity;
		velocity_n.normalize();
		// Collision vector is radius of player + movement for next frame
		// TODO: this doesn't actually work right, because the direction is wrong
		// Maybe project 2 vectors, COLLISION_RANGE apart, parallel to the actual movement vector
		Vec2 next_pos = position + velocity * ts;
		next_pos += velocity_n * COLLISION_RANGE;
		Line movement(position, next_pos);

		// Hrm, this might lead to weirdness if there's a protrusion narrower than COLLISION_RANGE...

		// Issue: Slamming into a surface -> want to position at collision_point - (normal*radius)
		// Moving along a surface -> don't want to sap velocity, so want to be at finishing_point, adjusted to
		// be outside the surface
		Vec2 offset(0, 0);
		std::auto_ptr<std::pair<Line, Vec2> > collision = world.collide_line(movement);
		if (!collision.get()) {
			offset = Vec2(-velocity_n.y, velocity_n.x);
			offset *= COLLISION_RANGE/2.0f;
			collision = world.collide_line(Line(position + offset, next_pos + offset));
		}
		if (!collision.get()) {
			offset *= -1;
			collision = world.collide_line(Line(position + offset, next_pos + offset));
		}

		// Continous collision physics
		onground = 0;
		if (collision.get()) {
			//print_line("Collision ", *collision);
			// Vector rejection -> sliding!
			if (bounce) {
				vec2_bounce(collision->first, velocity);
				// TODO: Cut or cap velocity
				// TODO: adjust position a bit to account for distance covered before and after the
				// bounce
			} else {
				float vy_before = velocity.y;
				// No need to account for offset, because all the vectors are parallel
				Vec2 collision_angle = Vec2(collision->first.y2 - collision->first.y1,
						collision->first.x2 - collision->first.x1);
				velocity = vec2_reject(velocity, collision_angle);

				// Push back from the collision point by COLLISION_RANGE
				Vec2 normal(collision->first.y1 - collision->first.y2,
						collision->first.x2 - collision->first.x1);
				normal.normalize(); //normalized vector normal

				// TODO: 
				position = collision->second - normal * COLLISION_RANGE;
				//position -= offset;
				// Not going to work quite right with non-perpendicular geometry
				if (vy_before < 0 && velocity.y == 0) {
					onground = 1;
				}

				//applies to both ground movement and sliding _into_ blocks
				velocity -= velocity*1*ts;
			}
		}
		position += velocity * ts;

	}
	if (hook.is_active()) {

		// Trim excess nodes first
		if (!release_window && hook_nodes.size() == 2 &&
				dist2(*hook_nodes.begin(), node_2()) < NODE_MIN_DISTANCE) {
			std::cout << "Short range deleted\n";
			hook.deactivate();
			hook_nodes.clear();
		} else {
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
					std::cout << "Node deleted player end\n";
					hook_nodes.erase(iter);
				}
				if (hook_nodes.size() < 2) {
					std::cout << "Deleted last node!\n";
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
					std::cout << "Node deleted hook end\n";
					//hook_nodes.erase(--iter2.base());
					hook_nodes.erase(iter2);
				}
			}

			if (hook_nodes.size() > 2) {
				unwrap_rope();
			}
			wrap_rope();

			hook.update(ts);
			// TODO: confirm if this copies in-place (maybe)
			hook_nodes.front() = position;
			hook_nodes.back() = hook.get_position();
		}
	}
	return;

}

void Player::wrap_rope() {

	std::list<Vec2>::iterator iter = hook_nodes.begin();

	iter++;
	Line segment(hook_nodes.front(), *iter);
	std::auto_ptr<Vec2> collision = world.collide_corner(segment);
	if (collision.get()) {
		hook_nodes.insert(iter, *collision);
		rope_angle_player = node_angle(hook_nodes.begin());
		std::cout << "Collided is now " << hook_nodes.size() << " points\n";
		std::cout << "Angle: " << rope_angle_player << "\n";
	}

	// Geometry isn't mobile, so only the first and last segments can actually move
/*	Vec2 *last_node = NULL;
	for (std::list<Vec2>::iterator iter = hook_nodes.begin();
			iter != hook_nodes.end(); iter++) {
		if (!last_node) {
			last_node = &*iter;
			continue;
		}

		Line segment(*last_node, *iter);
		Vec2 *collision = world.collide_corner(segment);
		if (collision) {
			hook_nodes.insert(iter, *collision);
			delete collision;
		} else {
			last_node = &*iter;
		}

	}
*/
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
	if (onground) {
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
	if (onground) {
		target_velocity.y = 10000;
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
		std::cout << "Retract!\n";
		hook.velocity.x = 0;
		hook.velocity.y = 0;
	} else {
		std::cout << "Fire!\n";
		print_vec2("Player", position);
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

