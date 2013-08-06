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

		Rect *get_viewport();
	private:
		Player *player;

};

#endif // world_h
