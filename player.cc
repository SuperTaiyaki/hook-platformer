#include <stdio.h>
#include <iostream>

#include "player.h"
#include "world.h"

#include "geometry.h"

#include "tunables.h"

void Player::update(float ts) {

	if (hook.is_active()) {
		hook.update(ts);
	}

	if (velocity.x || velocity.y) {
		Vec2 next_pos = position + velocity * ts;
		Line movement(position, next_pos);

		std::auto_ptr<Line> collision = world.collide_line(movement);
		if (collision.get()) {
			print_line("Collision ", *collision);
			vec2_bounce(*collision, velocity);
		}

		position += velocity * ts;
	}
	if (hook.is_active()) {
		wrap_rope();

		// TODO: confirm if this copies in-place (maybe)
		hook_nodes.front() = position;
		hook_nodes.back() = hook.get_position();
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

void Player::control(float x, float y) {
	//velocity.x += x * PLAYER_SPEED_X;
	//velocity.y += y * PLAYER_SPEED_Y;
	velocity.y = y * PLAYER_SPEED_X;
	velocity.x = x * PLAYER_SPEED_X;
}

void Player::fire(float x, float y) {
	if (hook.is_active()) {
		hook.release();
		// pull in hook
	} else {
		Vec2 aim(x, y);
		hook.launch(position, aim - position);
		hook_nodes.push_back(position);
		hook_nodes.push_back(position);
	}
	return;
}

const Vec2 &Player::get_position() const{
	return position;
}

const std::list<Vec2> &Player::get_rope_path() const {
	return hook_nodes;
}

