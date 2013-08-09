#ifndef __player_h__
#define __player_h__

#include <list>

#include "geometry.h"
#include "hook.h"

class World;

class Player {
	public:
		// Do velocity and f_accum get zeroed...?
		Player(float x, float y, World &w): position(x, y), world(w), hook(w) {};
		~Player();

		void update(float timestep);

		// Float for analog control... maybe
		void control(float x, float y);
		void fire(float x, float y);

		const Vec2 &get_position() const;
		const std::list<Vec2> &get_rope_path() const;
	private:
		Vec2 position;
		Vec2 velocity;
		Vec2 f_accum;
		World &world;

		Hook hook;

		std::list<Vec2> hook_nodes;
};

#endif // __player_h__
