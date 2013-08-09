#ifndef __world_h__
#define __world_h__

#include "stage.h"
#include "player.h"

class World {
	public:
		World();
		~World();

		void update(float ts);
		void set_aspect_ratio(float);

		Line *collide_line(const Line &line) const;
		Vec2 *collide_corner(const Line &line) const;

		void set_player(Player *p);
		const Player &get_player() const;

		void set_stage(Stage *s);
		const Stage &get_stage() const;

		const Rect &get_viewport() const;
	private:
		Player *player;
		Stage *stage;
		float display_aspect;
		float zoom;
		Rect viewport;
};

#endif // world_h
