#ifndef __player_h__
#define __player_h__

#include <list>

#include "geometry.h"
#include "hook.h"

class World;

class Player {
	public:
		// Do velocity and f_accum get zeroed...?
		Player(float x, float y, World &w): position(x, y), world(w), hook(w), pull(0) {};
		~Player();

		void update(float timestep);
		void push(float x, float y);

		// Float for analog control... maybe
		void control(float x, float y);
		void trigger(float x, float y);
		void retract(int value);

		const Vec2 &get_position() const;
		const std::list<Vec2> &get_rope_path() const;
	private:
		Vec2 position;
		Vec2 velocity;
		Vec2 f_accum;
		World &world;

		Hook hook;
		int release_window;
		int pull;

		std::list<Vec2> hook_nodes;
		float rope_angle_player;
		float rope_angle_hook;

		float node_angle(std::list<Vec2>::const_iterator iter) const;

		void wrap_rope();
		void unwrap_rope();
		void rope_retract(float ts);
		void rope_brake();

		const Vec2 &node_2() const; // 2nd node in the list
};

#endif // __player_h__
