#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <set>

#include "stage.h"

#define STAGE_WIDTH 3000
#define STAGE_HEIGHT 5000
#define BORDER_WIDTH 100

Stage::Stage(): bounds(Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT)) {

	generate_border();

	generate_climb();
	return;
}

Stage::Stage(const char *filename) {
	load_stage(filename);
}

void Stage::load_stage(const char *filename) {
	std::ifstream file(filename);
	char type;

	file >> type;
	while (type) {
		switch(type) {
			case 'r':
				float x1, y1, x2, y2;
				file >> x1 >> y1 >> x2 >> y2;
				char flags;
				file >> flags;
				switch(flags) {
					case 'b':
						bounds = Rect(x1, y1, x2, y2);
						break;
					case 'g':
						std::cout << "Created goal\n";
						goal = Rect(x1, y1, x2, y2);
						break;
					default:
						geometry.push_back(new Rect(x1, y1, x2, y2));
				}
				break;
			case 'p':
				float x, y;
				file >> x >> y;
				// for now, only origin is available...
				origin = Vec2(x, y);
		}
		file >> type;
		if (file.eof()) {
			break;
		}
	}
	file.close();
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
	// Right
	geometry.push_back(new Rect(bounds.x2, bounds.y1,
				bounds.x2 + BORDER_WIDTH, bounds.y2));
	// Top
	geometry.push_back(new Rect(bounds.x1 - BORDER_WIDTH, bounds.y2,
				bounds.x2 + BORDER_WIDTH, bounds.y2 + BORDER_WIDTH));
}

std::auto_ptr<std::vector<Line> > Stage::block_borders(Rect &in) const{
	std::auto_ptr<std::vector<Line> > ret(new std::vector<Line>);
	ret->reserve(4); //Not using C++11...
	// This is probably not the most efficient way to load the array, but they're fairly small
	// Generate anti-clockwise so normals can be generated consistently
	// Bottom
	ret->push_back(Rect(in.x1, in.y1, in.x2, in.y1));
	// Top
	ret->push_back(Rect(in.x2, in.y2, in.x1, in.y2));

	// Left
	ret->push_back(Rect(in.x1, in.y2, in.x1, in.y1));
	//Right
	ret->push_back(Rect(in.x2, in.y1, in.x2, in.y2));

	return ret;
}

std::auto_ptr<std::pair<Line, Vec2> > Stage::collide_line(const Line &other) const {

	for (std::list<Rect *>::const_iterator iter = geometry.begin();
			iter != geometry.end(); iter++) {
		std::auto_ptr<std::vector<Line> >lines = block_borders(**iter);

		for (std::vector<Line>::const_iterator liter = lines->begin();
				liter != lines->end(); liter++) {
			std::auto_ptr<Vec2> point = line_collision(other, *liter);
			if (point.get()) {
				// Uhh... stick point and line together in a new autoed pair
				return std::auto_ptr<std::pair<Line, Vec2> >
					(new std::pair<Line, Vec2>(*liter, *point));
			}
		}
		lines.reset();
	}
	return std::auto_ptr<std::pair<Line, Vec2> >();
}

// For each block in the geometry, run a line collision
// If there's a hit, store the line
// If there's more than one common point between saved lines, that's the corner
std::auto_ptr<Vec2> Stage::collide_corner(const Line &other) const {
	std::set<Vec2> points;
	std::auto_ptr<Vec2> ret;
	for (std::list<Rect *>::const_iterator iter = geometry.begin();
			iter != geometry.end(); iter++) {
		std::auto_ptr<std::vector<Line> >lines = block_borders(**iter);

		for (std::vector<Line>::const_iterator liter = lines->begin();
				liter != lines->end(); liter++) {
			std::auto_ptr<Vec2> point = line_collision(other, *liter);
			if (point.get()) {
				Vec2 p1(Vec2((*liter).x1, (*liter).y1));
				if (points.find(p1) != points.end()) {
					ret = std::auto_ptr<Vec2>(new Vec2(p1));
					// can't just return here because the fiddling is required
					break;
				} else {
					points.insert(p1);
				}
				Vec2 p2((*liter).x2, (*liter).y2);
				if (points.find(p2) != points.end()) {
					ret = std::auto_ptr<Vec2>(new Vec2(p2));
					break;
				} else {
					points.insert(p2);
				}
			}
		}
		points.clear();
		//delete lines;
		if (ret.get()) {
			// Fiddle the values lightly, to push the collision _outside_ the corner
			// TODO: do the std::min/max thing so this works if vertices are out of order (would break the
			// block generation anyway)
			if (ret->x == (*iter)->x1) {
				ret->x -= 0.1f;
			} else { // it's x2, the right edge
				ret->x += 0.1f;
			}
			if (ret->y == (*iter)->y1) {
				ret->y -= 0.1f;
			} else {
				ret->y += 0.1f;
			}
			break;
		}
		lines.reset();
	}
	return ret;

}
int Stage::at_goal(const Vec2 &position) const {
	return (position.x > goal.x1 && position.x < goal.x2 &&
			position.y > goal.y1 && position.y < goal.y2);
}
const std::list<Rect*> &Stage::get_geometry() const {
	return geometry;
}

const Rect &Stage::get_bounds() const {
	return bounds;
}

const Rect &Stage::get_goal() const {
	return goal;
}


const Vec2 &Stage::get_origin() const {
	return origin;
}
