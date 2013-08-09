#ifndef __stage_h__
#define __stage_h__

#include <list>
#include <vector>

#include "geometry.h"

class Stage {
	public:
		Stage();
		~Stage();

		const Rect bounds;// = Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT);
		const std::list<Rect*> &get_geometry() const;

		const Rect &get_bounds() const;

		std::auto_ptr<Line> collide_line(const Line &in) const;
		std::auto_ptr<Vec2> collide_corner(const Line &in) const;
	private:

		std::list<Rect*> geometry;

		void generate_border();
		void generate_climb();

		std::auto_ptr<std::vector<Line> > block_borders(Rect &in) const;
};

#endif // __stage_h__

