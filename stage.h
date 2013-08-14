#ifndef __stage_h__
#define __stage_h__

#include <list>
#include <vector>

#include "geometry.h"

class Stage {
	public:
		Stage();
		Stage(const char* filename);
		~Stage();

		const std::list<Rect*> &get_geometry() const;

		const Rect &get_bounds() const;
		const Rect &get_goal() const;
		const Vec2 &get_origin() const;
		int at_goal(const Vec2 &position) const;
		// line impact and collision point
		std::auto_ptr<std::pair<Line, Vec2> > collide_line(const Line &in) const;
		std::auto_ptr<Vec2> collide_corner(const Line &in) const;
	private:

		Rect bounds;
		Rect goal;
		std::list<Rect*> geometry;
		Vec2 origin;

		void generate_border();
		void generate_climb();
		void load_stage(const char*filename);

		std::auto_ptr<std::vector<Line> > block_borders(Rect &in) const;
};

#endif // __stage_h__

