#ifndef __player_h__
#define __player_h__

#include "geometry.h"

class Player {
	public:
		// Do velocity and f_accum get zeroed...?
		Player(float x, float y): position(x, y) {};
		~Player();

		void update(float timestep);

		const Vec2 &get_position() const;
	private:
		Vec2 position;
		Vec2 velocity;
		Vec2 f_accum;
};

#endif // __player_h__
