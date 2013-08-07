#include <stdio.h>
#include <iostream>

#include "player.h"

#define PLAYER_SPEED_X 50
#define PLAYER_SPEED_Y 50

void Player::update(float ts) {

	position += velocity * ts;
	return;

}

void Player::control(float x, float y) {
	velocity.x = x * PLAYER_SPEED_X;
	velocity.y = y * PLAYER_SPEED_Y;
}

void Player::fire(float x, float y) {
	std::cout << "X: " << x << " Y: " << y << "\n";
	position.x = x;
	position.y = y;
	return;
}

const Vec2 &Player::get_position() const{
	return position;
}

