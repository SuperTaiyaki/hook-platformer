#include <stdio.h>

#include "player.h"

void Player::update(float ts) {

	velocity.x += ts;
	velocity.y -= ts;
	position += velocity * ts;
//	printf("%f %f\n" , position.x, position.y);
	return;

}

const Vec2 &Player::get_position() const{
	return position;
}
