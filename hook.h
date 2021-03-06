#ifndef __hook_h__
#define __hook_h__

#include "geometry.h"
class World;	

class Hook {
	public:
		Hook(World &w): stuck(0), active(0), world(w) {};
		~Hook() {};

		bool is_active() const;
		void release();
		void launch(const Vec2 &origin, const Vec2 &aim);
		void deactivate();
		void update(float timestep);
		const Vec2 &get_position() const;
	private:
		Vec2 position;
		Vec2 velocity;
		bool stuck;
		bool active;
		bool retracting;

		World &world;

	friend class Player;
};

#endif // __hook_h__

