#include "world.h"

World::World() {
	return;
}

Rect *World::get_viewport() {
	return new Rect(0, 0, 100, 100);
}

void World::update(float timestep) {
	return;
}
