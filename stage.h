#ifndef __stage_h__
#define __stage_h__

#include <list>

#include "geometry.h"

#define STAGE_WIDTH 3000
#define STAGE_HEIGHT 5000

class Stage {
	public:
		Stage();
		~Stage();

		const Rect bounds;// = Rect(-STAGE_WIDTH/2.0f, 0.0f, STAGE_WIDTH/2.0f, STAGE_HEIGHT);
		const std::list<Rect*> get_geometry();
	private:

		std::list<Rect*> geometry;


		void generate_climb();
};

#endif // __stage_h__

