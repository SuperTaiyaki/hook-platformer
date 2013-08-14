#ifndef __world_h__
#define __world_h__

#include <vector>

#include "stage.h"
#include "player.h"

class World {
	public:
		World();
		~World();

		void update(float ts);
		void set_aspect_ratio(float);

		std::auto_ptr<std::pair<Line, Vec2> > collide_line(const Line &line) const;
		std::auto_ptr<Vec2> collide_corner(const Line &line) const;

		void set_player(Player *p);
		const Player &get_player() const;

		void set_stage(Stage *s);
		const Stage &get_stage() const;

		const Rect &get_viewport() const;

		void set_focus(float x, float y);
	private:
		Player *player;
		Stage *stage;
		float display_aspect;
		float zoom;
		Vec2 cursor;
		Rect viewport;

		std::vector<Vec2> cursor_history;
		int cursor_p; //ring buffer it

};

#endif // world_h
