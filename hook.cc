#include <iostream>

#include "hook.h"
#include "world.h"

#include "tunables.h"

bool Hook::is_active() const {
	return active;
}

void Hook::release() {
	stuck = 0;
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
	return;
}

// Maybe to be merged with player at some point...
void Hook::update(float ts) {
	if (!stuck) {
		velocity.y -= GRAVITY * ts;
		
		Vec2 next_pos = position + velocity * ts;
		Line movement(position, next_pos);

		std::auto_ptr<Line> collision = world.collide_line(movement);
		if (collision.get()) {
			//vec2_bounce(*collision, velocity);
			std::cout << "Hook caught\n";
			stuck = 1;
			velocity.x = velocity.y = 0;
			return;
		}

		position += velocity * ts;
		print_vec2("Hook p", position);
		print_vec2("Hook v", velocity);
	}
}

const Vec2 &Hook::get_position() const {
	return position;
}

