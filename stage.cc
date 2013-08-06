#include "stdlib.h"

#include "stage.h"

Stage::Stage(): bounds(Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT)) {

	generate_climb();
	return;
}

void Stage::generate_climb() {
	for (float y = 100;y < STAGE_HEIGHT; y += 300) {
		float x = (random() % 1000 - 500);

		geometry.push_back(new Rect(x, y, x+100, y+100));
	}
}

