// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "fg_math.h"

namespace fg {
	vector3 segmentIntersection(const vector3& l0p0, const vector3& l0p1, const vector3& l1p0, const vector3& l1p1) {
		plane pl0 = plane::byTwoPointsAndNormal(l0p0, l0p1, vector3(0.0, 0.0, 1.0));
		bool first = pl0.classify(l1p0) != pl0.classify(l1p1);

		vector3 ll0 = l0p0 - l0p1;
		vector3 ll1 = l1p0 - l1p1;

		if ((ll0 ^ ll1).lengthSq() < FG_EPS) {
			if (pl0.classify(l1p0) == 0) {
				return vector3::Nan();
			}
			return vector3::Inf();
		}
		plane pl1 = plane::byTwoPointsAndNormal(l1p0, l1p1, vector3(0.0, 0.0, 1.0));
		bool second = pl1.classify(l0p0) != pl1.classify(l0p1);
		vector3 tl0 = ll0.getNormalized();
		vector3 nl0 = tl0 ^ vector3(0.0, 0.0, 1.0);
		if (first && second) {
			vector3 v0 = l0p1 - l1p1;
			if (std::fabs(ll1&nl0) < FG_EPS) return vector3::Nan();
			auto point = (v0&nl0)*nl0 + (v0&nl0)*(ll1&tl0) / (ll1&nl0) * tl0 + l1p1;
			return point;
		}
		return vector3::Nan();
	}
}
namespace std {
	fg::vector3 min(const fg::vector3& lhs, const fg::vector3& rhs) {
		return fg::vector3(std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z));
	}
}