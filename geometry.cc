#include "geometry.h"

Vec2 &Vec2::operator+=(const Vec2 &rhs) {
	x += rhs.x;
	y += rhs.y;

	return *this;
}

Vec2 operator*(const Vec2 &lhs, const float rhs) {
	Vec2 ret = lhs;
	ret.x *= rhs;
	ret.y *= rhs;
	return ret;
}

