#include <stdio.h>
#include <iostream>

#include "player.h"
#include "world.h"

#define PLAYER_SPEED_X 100
#define PLAYER_SPEED_Y 100
#define GRAVITY -100

void Player::update(float ts) {

	if (velocity.x || velocity.y) {
		Vec2 next_pos = position + velocity * ts;
		Line movement(position, next_pos);

		Line *collision = world.collide_line(movement);
		if (collision) {
			std::cout << "Collision!\n";
			printf("x1: %f y1: %f x2: %f y2: %f\n",
					collision->x1, collision->y1, collision->x2, collision->y2);
			vec2_bounce(*collision, velocity);
			delete collision;
		}

		position += velocity * ts;
	}
	return;

}

void Player::control(float x, float y) {
	velocity.x += x * PLAYER_SPEED_X;
	velocity.y += y * PLAYER_SPEED_Y;
}

void Player::fire(float x, float y) {
	std::cout << "X: " << x << " Y: " << y << "\n";
	/* position.x = x;
	position.y = y; */
	return;
}

const Vec2 &Player::get_position() const{
	return position;
}

