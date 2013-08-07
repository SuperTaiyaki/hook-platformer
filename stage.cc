#include "stdlib.h"
#include <iostream>

#include "stage.h"

#define STAGE_WIDTH 3000
#define STAGE_HEIGHT 5000

Stage::Stage(): bounds(Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT)) {

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

const std::list<Rect*> &Stage::get_geometry() const {
	return geometry;
}

