#include <iostream>

#include "hook.h"
#include "world.h"

#include "tunables.h"

bool Hook::is_active() const {
	return active;
}

void Hook::release() {
	stuck = 0;
	retracting = 1;
}

void Hook::deactivate() {
	active = 0;
}

void Hook::launch(const Vec2 &origin, const Vec2 &aim) {
	float len = hypot(aim);
	velocity = aim * (HOOK_SPEED/len);
	position = origin;
	active = 1;
	stuck = 0;
	retracting = 0;
	return;
}

// Maybe to be merged with player at some point...
void Hook::update(float ts) {
	if (!stuck) {
		velocity.y -= GRAVITY * ts;

		Vec2 next_pos = position + velocity * ts;
		Line movement(position, next_pos);

		std::auto_ptr<std::pair<Line, Vec2> > collision = world.collide_line(movement);
		if (!retracting && collision.get()) {
			// TODO: low angle hook bounce, etc.
			//vec2_bounce(*collision, velocity);
			stuck = 1;
			velocity.x = velocity.y = 0;
			// 0.001 is arbitrary, the coordinate needs to be outside the surface
			position = collision->second - velocity * 0.001;
			return;
		}

		position += velocity * ts;
	}
}

const Vec2 &Hook::get_position() const {
	return position;
}

