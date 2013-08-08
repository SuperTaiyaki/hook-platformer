#ifndef __player_h__
#define __player_h__

#include "geometry.h"

class World;

class Player {
	public:
		// Do velocity and f_accum get zeroed...?
		Player(float x, float y, World &w): position(x, y), world(w) {};
		~Player();

		void update(float timestep);

		// Float for analog control... maybe
		void control(float x, float y);
		void fire(float x, float y);

		const Vec2 &get_position() const;
	private:
		Vec2 position;
		Vec2 velocity;
		Vec2 f_accum;
		World &world;
};

#endif // __player_h__
