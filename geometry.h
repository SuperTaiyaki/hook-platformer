#ifndef __geometry_h__
#define __geometry_h__

// Completely transparent class - more like a struct with operators
class Vec2 {
	public:
		Vec2(): x(0), y(0) {};
		Vec2(float ix, float iy): x(ix), y(iy) {};
		union {
			float coords[2];
			struct {
				float x, y;
			};
		};
		// Defining some useful stuff... hopefully not too haphazard
		// There are only getting added as they're needed
		Vec2 &operator+=(const Vec2 &rhs);
};
Vec2 operator*(const Vec2 &lhs, const float rhs);
Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs);

// Convention: bottom left, top right (i.e. lower coords, then higher coords
class Rect {
public:
	Rect(float ix1, float iy1, float ix2, float iy2): x1(ix1), y1(iy1), x2(ix2), y2(iy2){};
	Rect(Vec2 &p1, Vec2 &p2): x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y){};
	union {
		float coords[4];
		struct {
			float x1, y1, x2, y2;
		};
	};
};
typedef Rect Line; // actually a segment

Vec2 *line_collision(const Line &l1, const Line &l2);
// Reflect point in axis
void vec2_bounce(const Line &axis, Vec2 &point);

#endif //__geometry_h__
