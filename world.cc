#include <iostream>
#include "world.h"

#include "tunables.h"

World::World(): 
	display_aspect(4.0f/3.0f),
	zoom(1000.0f),
	viewport(0, 0, 0, 0),
	stage_timer(0),
	goal(0),
	cursor_history(CURSOR_FRAMES),
	cursor_p(0) {
	return;
}

const Rect &World::get_viewport() const{
	return viewport;
}

void World::update(float timestep) {
	if (!goal) {
		stage_timer += timestep;
	}
	// should explode here if player and world aren't set
	player->update(timestep);

	// zoom is actually width
	// Center on the player... or something. Ignoring for now!
	// Make sure the player is visible, an
	const Vec2 &p_player = player->get_position();
	const Rect &border = stage->get_bounds();

	Vec2 cursor_pos(cursor_history[0]);
	// Less typing than using an iterator...
	for (unsigned int i = 1; i < cursor_history.size(); i++) {
		cursor_pos += cursor_history[i];
		// Vec2 wasn't really meant to be used this way, but it still makes sense!
	}
	cursor_pos *= 1.0f/cursor_history.size();
	Vec2 center(p_player.x * 0.75+cursor_pos.x*0.25, p_player.y * 0.75 + cursor_pos.y*0.25);
	if (center.y < border.y1) {
		center.y = border.y1;
	}
	//Vec2 center((p_player.x + cursor_pos.x)/2.0f, (p_player.y + cursor_pos.y)/2.0f);
	viewport.x1 = center.x - zoom/2.0f;
	viewport.x2 = center.x + zoom/2.0f;
	float height = zoom / display_aspect;
	viewport.y1 = center.y - height/2.0f;
	viewport.y2 = center.y + height/2.0f;

	if (p_player.y < border.y1 - 1000)
		reset();

	if (stage->at_goal(p_player) && !goal) {
		goal = 1;
		std::cout << "------Goal in " << stage_timer << "s-----\n";
	}

	return;
}

void World::reset() {
	stage_timer = 0;
	goal = 0;
	player->reset(stage->get_origin());
}

void World::set_focus(float x, float y) {
	cursor_history[cursor_p].x = x;
	cursor_history[cursor_p].y = y;
	cursor_p++;
	cursor_p %= CURSOR_FRAMES;
}

std::auto_ptr<std::pair<Line, Vec2> > World::collide_line(const Line &line) const {
	return stage->collide_line(line);
}

std::auto_ptr<Vec2> World::collide_corner(const Line &line) const {
	return stage->collide_corner(line);
}


void World::set_player(Player *p) {
	player = p;
	// Load the cursor history so it doesn't snap too much when animation starts
	const Vec2 &position = p->get_position();
	for (unsigned int i = 0;i < cursor_history.size();i++) {
		cursor_history[i] = position;
	}
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


float World::get_timer() const {
	return stage_timer;
}

