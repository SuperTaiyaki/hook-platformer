#include <math.h> //isinf is only C++11
#include <cmath>
#include <algorithm>
#include <iostream>

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

Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) {
	Vec2 ret = lhs;
	ret.x += rhs.x;
	ret.y += rhs.y;
	return ret;
}

Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs) {
	Vec2 ret = lhs;
	ret.x -= rhs.x;
	ret.y -= rhs.y;
	return ret;
}
bool operator<(const Vec2 &lhs, const Vec2 &rhs) {
	if (lhs.x == rhs.x)
		return lhs.y < rhs.y;
	return lhs.x < rhs.x;
}

float gradient(const Line &line) {
	// No need to worry about infinites here?
	return (line.y2 - line.y1) / (line.x2 - line.x1);
}

Vec2 *line_collision(const Line &l1, const Line &l2) {
	float m1 = gradient(l1);
	float m2 = gradient(l2);

	if (isinf(m1) and isinf(m2)) {
		if (l1.x1 != l2.x1) {
			return NULL;
		}
		// TODO: parallel check
		return NULL;
	}

	float c1, c2;
	float collision_x, collision_y;

	// New references, so that l1 and l2 can be swapped around
	Line const *rl1 = &l1;
	Line const *rl2 = &l2;
	if (isinf(m2)) {
		rl1 = &l2;
		rl2 = &l1;
		float tmp = m2;
		m2 = m1;
		m1 = tmp;
	}
	if (isinf(m1)) {
		// l1 is vertical
		collision_x = rl1->x1;
		if (collision_x < std::min(rl2->x1, rl2->x2) ||
				collision_x > std::max(rl2->x1, rl2->x2)) {
			return NULL;
		}

		c2 = rl2->y1 - m2 * rl2->x1;
		collision_y = c2 + collision_x * m2;
		if (collision_y < std::min(rl1->y1, rl1->y2) || 
				collision_y > std::max(rl1->y1, rl1->y2)) {
				return NULL;
		}
		return new Vec2(collision_x, collision_y);
	}

	c1 = rl1->y1 - m1 * rl1->x1;
	c2 = rl2->y1 - m2 * rl2->x1;
	if (m1 == m2) {
		if (c1 != c2) {
			return NULL;
		}
		// TODO: More parallel collision junk...
		return NULL;
	}

	// Special cases for straight lines - float error makes this awkward otherwise
	if (m2 == 0) {
		collision_y = rl2->y1;
		collision_x = (collision_y - c1) / m1;
	} else if (m1 == 0) {
		collision_y = rl1->y1;
		collision_x = (collision_y - c2) / m2;
	} else {
		collision_x = (c2 - c1) / (m1 - m2);
		collision_y = collision_x * m1 + c1;
	}

	float min_x = std::max(std::min(rl1->x1, rl1->x2), std::min(rl2->x1, rl2->x2));
	float max_x = std::min(std::max(rl1->x1, rl1->x2), std::max(rl2->x1, rl2->x2));
	float min_y = std::max(std::min(rl1->y1, rl1->y2), std::min(rl2->y1, rl2->y2));
	float max_y = std::min(std::max(rl1->y1, rl1->y2), std::max(rl2->y1, rl2->y2));

	if (collision_x >= min_x && collision_x <= max_x &&
			collision_y >= min_y && collision_y <= max_y) {
		return new Vec2(collision_x, collision_y);
	}

	return NULL;
}

void vec2_bounce(const Line &axis, Vec2 &point) {
	// [cos2t  sin2t][x]
	// [sin2t -cos2t][y]

	float angle = std::atan2(axis.y2 - axis.y1, axis.x2 - axis.x1);
	float c2t = std::cos(2*angle);
	float s2t = std::sin(2*angle);
	float nx = c2t * point.x + s2t * point.y;
	float ny = s2t * point.x - c2t * point.y;
	point.x = nx;
	point.y = ny;
}

float hypot(const float x, const float y) {
	return std::sqrt(x*x+y*y);
}
float hypot(const Vec2 &segment) {
	return hypot(segment.x, segment.y);
}
float hypot(const Line &segment) {
	return hypot(segment.x2 - segment.x1, segment.y2 - segment.y1);
}

//TODO: rewrite this as a.^b? (shorter scalar projection definition)
float vec2_project(const Vec2 &src, const Vec2 &dst) {
	float angle = angle_diff(dst, src);
	return std::cos(angle) * hypot(dst.x, dst.y);
}

float angle_diff(const Vec2 &a1, const Vec2 &a2) {
	float dir_a1 = std::atan2(a1.y, a1.x);
	float dir_a2 = std::atan2(a2.y, a2.x);
	float angle = std::fmod(dir_a1 - dir_a2 + M_PI*2, M_PI*2);
	if (angle > M_PI) {
		angle -= M_PI * 2;
	}
	return angle;
}

void print_vec2(const std::string prefix, const Vec2 &in) {
	std::cout << prefix << " X: " << in.x << " Y: " << in.y << "\n";
}

void print_line(const std::string prefix, const Line &in) {
	std::cout << prefix << " 1: (" << in.x1 << "," << in.y1 <<
		") 2: (" << in.x2 << "," << in.y2 << ")\n";
}
