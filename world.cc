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
	float focus[] = {0, 0};
	viewport.x1 = focus[0] - zoom/2.0f;
	viewport.x2 = focus[0] + zoom/2.0f;
	float height = zoom / display_aspect;
	viewport.y1 = focus[1] - height/2.0f;
	viewport.y2 = focus[1] + height/2.0f;
	
	return;
}

void World::set_player(Player *p) {
	player = p;
}
const Player &World::get_player() {
	return *player;
}

void World::set_aspect_ratio(float aspect) {
	display_aspect = aspect;
}
