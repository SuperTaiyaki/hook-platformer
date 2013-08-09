#include <iostream>
#include "world.h"

World::World(): 
	display_aspect(4.0f/3.0f),
	zoom(1000.0f),
	viewport(0, 0, 0, 0) {
	return;
}

const Rect &World::get_viewport() const{
	return viewport;
}

void World::update(float timestep) {

	player->update(timestep);

	// zoom is actually width
	// Center on the player... or something. Ignoring for now!
	// Got a feeling this is actually wrong... oh well
	const Vec2 &focus = player->get_position();
	viewport.x1 = focus.x - zoom/2.0f;
	viewport.x2 = focus.x + zoom/2.0f;
	float height = zoom / display_aspect;
	viewport.y1 = focus.y - height/2.0f;
	viewport.y2 = focus.y + height/2.0f;

	return;
}

Line *World::collide_line(const Line &line) const {
	return stage->collide_line(line);
}

Vec2 *World::collide_corner(const Line &line) const {
	return stage->collide_corner(line);
}


void World::set_player(Player *p) {
	player = p;
}
const Player &World::get_player() const {
	return *player;
}

void World::set_stage(Stage *st) {
	stage = st;
}
const Stage &World::get_stage() const {
	return *stage;
}

void World::set_aspect_ratio(float aspect) {
	display_aspect = aspect;
}
