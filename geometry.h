#ifndef __geometry_h__
#define __geometry_h__

//typedef float[2] Vertex;
class Vertex {
	union {
		float coords[2];
		struct {
			float x, y;
		};
	};
};

class Rect {
public:
	Rect(float ix1, float iy1, float ix2, float iy2): x1(ix1), y1(iy1), x2(ix1), y2(iy2){};
	union {
		float coords[4];
		struct {
			float x1, y1, x2, y2;
		};
	};
};

#endif //__geometry_h__
