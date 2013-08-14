#ifndef __geometry_h__
#define __geometry_h__

#include <memory>

// Completely transparent class - more like a struct with operators
class Vec2 {
	public:
		Vec2(): x(0), y(0) {};
		Vec2(float ix, float iy): x(ix), y(iy) {};
		float x, y;
		// Defining some useful stuff... hopefully not too haphazard
		// There are only getting added as they're needed
		Vec2 &operator+=(const Vec2 &rhs);
		Vec2 &operator-=(const Vec2 &rhs);

		Vec2 &operator*=(const float rhs);
		float &operator[](unsigned i) {
			if (i == 0)
				return x;
			else if (i == 1)
				return y;
			// explode!
			throw 0;
		}
		void normalize();
};
Vec2 operator*(const Vec2 &lhs, const float rhs);
Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs);
Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs);
bool operator<(const Vec2 &lhs, const Vec2 &rhs); // Only for container use!

// Convention: bottom left, top right (i.e. lower coords, then higher coords
class Rect {
public:
	Rect(float ix1, float iy1, float ix2, float iy2): x1(ix1), y1(iy1), x2(ix2), y2(iy2){};
	Rect(const Vec2 &p1, const Vec2 &p2): x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y){};
	Rect(){};
	float x1, y1, x2, y2;
	// Apparently unioning this to an array isn't valid C++ - it's never passed around as an array anyway.
};
typedef Rect Line; // actually a segment

std::auto_ptr<Vec2> line_collision(const Line &l1, const Line &l2);
// Reflect point in axis
void vec2_bounce(const Line &axis, Vec2 &point);

// project src onto dst
float vec2_project(const Vec2 &src, const Vec2 &dst);
Vec2 vec2_reject(const Vec2 &src, const Vec2 &dst);

// regular cross product
//Vec2 vec2_cross(const Vec2 &a, const Vec2 &b);

// difference between 2 angles, adjusted to be [-pi <= x < pi]
float angle_diff(const Vec2 &a1, const Vec2 &a2);

// sqrt(x^2 + y^2) - std::hypot only available in c++11
// Doesn't do the error-minimising stuff
float hypot(const float x, const float y);
float hypot(const Vec2 &segment);
float hypot(const Line &segment);

// Distance between points squared
float dist2(const Vec2 &a, const Vec2 &b);

void print_vec2(std::string prefix, const Vec2 &in);
void print_line(std::string prefix, const Line &in);

#endif //__geometry_h__
