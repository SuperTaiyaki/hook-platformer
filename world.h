#ifndef __world_h__
#define __world_h__

#include "stage.h"

class World {
	public:
		World();
		~World();
		void set_stage(Stage s);
//		void set_player(Actor a);
		void update(float ts);

		Rect *get_viewport();

};

#endif // world_h
