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
