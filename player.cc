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

		Line *collision = world.collide_line(movement);
		if (collision) {
			print_line("Collision ", *collision);
			vec2_bounce(*collision, velocity);
			delete collision;
		}

		position += velocity * ts;
	}
	if (hook.is_active()) {
		// TODO: confirm if this copies in-place (maybe)
		hook_nodes.front() = position;
		hook_nodes.back() = hook.get_position();
	}
	return;

}

void Player::control(float x, float y) {
	velocity.x += x * PLAYER_SPEED_X;
	velocity.y += y * PLAYER_SPEED_Y;
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

