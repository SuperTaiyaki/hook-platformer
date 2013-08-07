#ifndef __world_h__
#define __world_h__

#include "stage.h"
#include "player.h"

class World {
	public:
		World();
		~World();
		void set_stage(Stage &s);
		void set_player(Player *p);

		const Player &get_player();
		Stage &get_stage();

		void update(float ts);
		void set_aspect_ratio(float);

		const Rect &get_viewport() const;
	private:
		Player *player;
		float display_aspect;
		float zoom;
		Rect viewport;
};

#endif // world_h
