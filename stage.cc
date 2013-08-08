#include "stdlib.h"
#include <iostream>

#include "stage.h"

#define STAGE_WIDTH 3000
#define STAGE_HEIGHT 5000
#define BORDER_WIDTH 100

Stage::Stage(): bounds(Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT)) {

	generate_border();

	generate_climb();
	return;
}
Stage::~Stage() {
	// UNTESTED
	if (!geometry.empty()) {
		for (std::list<Rect *>::const_iterator iterator = geometry.begin();
			       	iterator != geometry.end(); iterator++) {
			delete *iterator;
		}
	}
}

void Stage::generate_climb() {
	for (float y = 100;y < STAGE_HEIGHT; y += 300) {
		float x = (random() % 1000 - 500);

		geometry.push_back(new Rect(x, y, x+100, y+100));
	}

}

void Stage::generate_border() {
	// Bottom
	geometry.push_back(new Rect(bounds.x1 - BORDER_WIDTH, bounds.y1 - BORDER_WIDTH,
				bounds.x2 + BORDER_WIDTH, bounds.y1));
	// Left
	geometry.push_back(new Rect(bounds.x1 - BORDER_WIDTH, bounds.y1,
				bounds.x1, bounds.y2));
	geometry.push_back(new Rect(bounds.x2, bounds.y1,
				bounds.x2 + BORDER_WIDTH, bounds.y2));
	// Top
	geometry.push_back(new Rect(bounds.x1 - BORDER_WIDTH, bounds.y2,
				bounds.x2 + BORDER_WIDTH, bounds.y2 + BORDER_WIDTH));
}

std::vector<Line> *Stage::block_borders(Rect &in) const{
	std::vector<Line> *ret = new std::vector<Line>;
	ret->reserve(4); //Not using C++11...
	// This is probably not the most efficient way to load the array, but they're fairly small
	ret->push_back(Rect(in.x1, in.y1, in.x2, in.y1));
	ret->push_back(Rect(in.x1, in.y2, in.x2, in.y2));

	ret->push_back(Rect(in.x1, in.y1, in.x1, in.y2));
	ret->push_back(Rect(in.x2, in.y1, in.x2, in.y2));

	return ret;
}

Line *Stage::collide_line(const Line &other) const {

	for (std::list<Rect *>::const_iterator iter = geometry.begin();
			iter != geometry.end(); iter++) {
		std::vector<Line> *lines = block_borders(**iter);

		for (std::vector<Line>::const_iterator liter = lines->begin();
				liter != lines->end(); liter++) {
			Vec2 *point = line_collision(other, *liter);
			if (point) {
				delete point;
				return new Line(*liter);
			}
		}
		delete lines;
	}
	return NULL;
}

const std::list<Rect*> &Stage::get_geometry() const {
	return geometry;
}

const Rect &Stage::get_bounds() const {
	return bounds;
}


