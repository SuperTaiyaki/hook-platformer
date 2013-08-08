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

		Line *collide_line(const Rect &in) const;
		Line *collide_corner(const Rect &in) const;
	private:

		std::list<Rect*> geometry;

		void generate_border();
		void generate_climb();

		std::vector<Line> *block_borders(Rect &in) const;
};

#endif // __stage_h__

