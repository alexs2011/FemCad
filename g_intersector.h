#pragma once
#include "g_line.h"

namespace fg {
	template<class Element>
	class FEMCADGEOMSHARED_EXPORT Intersector {
	public:
		template<class L0, class L1>
		static IntersectionState intersect(std::vector<vector3>& points_on_l0, std::vector<vector3>& points_on_l1, const L0& l0, const L1& l1) {
			static_assert(std::is_base_of<Element, L0>::value, "l0 must derive Element type");
			static_assert(std::is_base_of<Element, L1>::value, "l1 must derive Element type");

			return _intersect(points_on_l0, points_on_l1, l0, l1);
		}
		static IntersectionState intersect_dynamic(std::vector<vector3>& points_on_l0, std::vector<vector3>& points_on_l1, const Element& l0, const Element& l1) {
			try {
				auto& ll0 = dynamic_cast<const LineSegment&>(l0);
				try {
					auto& ll1 = dynamic_cast<const LineSegment&>(l1);
					return _intersect(points_on_l0, points_on_l1, ll0, ll1);
				}
				catch (...) {
					auto& ll1 = dynamic_cast<const EllipticSegment&>(l1);
					return _intersect(points_on_l0, points_on_l1, ll0, ll1);
				}
			}
			catch (...) {
				auto& ll0 = dynamic_cast<const EllipticSegment&>(l0);
				try {
					auto& ll1 = dynamic_cast<const LineSegment&>(l1);
					return _intersect(points_on_l0, points_on_l1, ll0, ll1);
				}
				catch (...) {
					auto& ll1 = dynamic_cast<const EllipticSegment&>(l1);
					return _intersect(points_on_l0, points_on_l1, ll0, ll1);
				}
			}
		}
		static ClassificationState classify_dynamic(const Element& s, const Element& line) {
			try {
				auto& ll0 = dynamic_cast<const LineSegment&>(s);
				try {
					auto& ll1 = dynamic_cast<const LineSegment&>(line);
					return _classify(ll0, ll1);
				}
				catch (...) {
					auto& ll1 = dynamic_cast<const EllipticSegment&>(line);
					return _classify(ll0, ll1);
				}
			}
			catch (...) {
				auto& ll0 = dynamic_cast<const EllipticSegment&>(s);
				try {
					auto& ll1 = dynamic_cast<const LineSegment&>(line);
					return _classify(ll0, ll1);
				}
				catch (...) {
					auto& ll1 = dynamic_cast<const EllipticSegment&>(line);
					return _classify(ll0, ll1);
				}
			}
		}
		static int intersect_segment(const Element& s, const vector3& l1p0, const vector3& l1p1, std::vector<vector3>& res) {
			try {
				auto& e = dynamic_cast<const LineSegment&>(s);
				return intersect_segment(e, l1p0, l1p1, res);
			}
			catch (...) {
				auto& e = dynamic_cast<const EllipticSegment&>(s);
				return intersect_segment(e, l1p0, l1p1, res);
			}
		}
		static int intersect_segment(const LineSegment& l0, const vector3& l1p0, const vector3& l1p1, std::vector<vector3>& res) {
			auto l0p0 = l0.P0();
			auto l0p1 = l0.P1();

			plane pl0 = plane::byTwoPointsAndNormal(l0p0, l0p1, vector3(0.0, 0.0, 1.0));
			bool first = pl0.classify(l1p0) != pl0.classify(l1p1);
			if ((l0.getTangent() ^ (l1p1 - l1p0)).lengthSq() < FG_EPS)
			{
				if (pl0.classify(l1p0) == 0) {
					if (l0p0 < l0p1) {
						if (l1p0 < l1p1) {
							if (l1p0 < l0p0) {
								if (l1p1 < l0p1) {
									if (l1p1 >= l0p0) { // l1p0, l0p0, l1p1, l0p1
										res.push_back(l0p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l1p0, l1p1, l0p0, l0p1
										return 0;
									}
								}
								else { // l1p0, l0p0, l0p1, l1p1
									res.push_back(l0p0);
									res.push_back(l0p1);
									if (res.back() == res[res.size() - 2]) {
										res.pop_back();
										return 1;
									}
									return 2;
								}
							}
							else {
								if (l1p0 <= l0p1) {
									if (l1p1 < l0p1) { // l0p0, l1p0, l1p1, l0p1
										res.push_back(l1p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l0p0, l1p0, l0p1, l1p1
										res.push_back(l1p0);
										res.push_back(l0p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
								}
								else { // l0p0, l0p1, l1p0, l1p1
									return 0;
								}
							}
						}
						else {
							if (l1p1 < l0p0) {
								if (l1p0 < l0p1) {
									if (l1p0 >= l0p0) { // l1p1, l0p0, l1p0, l0p1
										res.push_back(l0p0);
										res.push_back(l1p0);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l1p1, l1p0, l0p0, l0p1
										return 0;
									}
								}
								else { // l1p1, l0p0, l0p1, l1p0
									res.push_back(l0p0);
									res.push_back(l0p1);
									if (res.back() == res[res.size() - 2]) {
										res.pop_back();
										return 1;
									}
									return 2;
								}
							}
							else {
								if (l1p1 <= l0p1) {
									if (l1p0 < l0p1) { // l0p0, l1p1, l1p0, l0p1
										res.push_back(l1p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l0p0, l1p1, l0p1, l1p0
										res.push_back(l0p1);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
								}
								else { // l0p0, l0p1, l1p1, l1p0
									return 0;
								}
							}
						}
					}
					else {
						if (l1p0 < l1p1) {
							if (l1p0 < l0p1) {
								if (l1p1 < l0p0) {
									if (l1p1 >= l0p1) { // l1p0, l0p1, l1p1, l0p0
										res.push_back(l1p1);
										res.push_back(l0p0);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l1p0, l1p1, l0p1, l0p0
										return 0;
									}
								}
								else { // l1p0, l0p1, l0p0, l1p1
									res.push_back(l0p0);
									res.push_back(l0p1);
									if (res.back() == res[res.size() - 2]) {
										res.pop_back();
										return 1;
									}
									return 2;
								}
							}
							else {
								if (l1p0 <= l0p0) {
									if (l1p1 < l0p0) { // l0p1, l1p0, l1p1, l0p0
										res.push_back(l1p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l0p1, l1p0, l0p0, l1p1
										res.push_back(l0p0);
										res.push_back(l1p0);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
								}
								else { // l0p1, l0p0, l1p0, l1p1
									return 0;
								}
							}
						}
						else {
							if (l1p1 < l0p1) {
								if (l1p0 < l0p0) {
									if (l1p0 >= l0p1) { // l1p1, l0p1, l1p0, l0p0
										res.push_back(l1p0);
										res.push_back(l0p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l1p1, l1p0, l0p1, l0p0
										return 0;
									}
								}
								else { // l1p1, l0p1, l0p0, l1p0
									res.push_back(l0p0);
									res.push_back(l0p1);
									if (res.back() == res[res.size() - 2]) {
										res.pop_back();
										return 1;
									}
									return 2;
								}
							}
							else {
								if (l1p0 <= l0p0) {
									if (l1p1 < l0p0) { // l0p1, l1p1, l1p0, l0p0
										res.push_back(l1p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
									else { // l0p1, l1p1, l0p0, l1p0
										res.push_back(l0p0);
										res.push_back(l1p1);
										if (res.back() == res[res.size() - 2]) {
											res.pop_back();
											return 1;
										}
										return 2;
									}
								}
								else { // l0p1, l0p0, l1p1, l1p0
									return 0;
								}
							}
						}
					}
				}
				return 0;
			}
			plane pl1 = plane::byTwoPointsAndNormal(l1p0, l1p1, vector3(0.0, 0.0, 1.0));
			bool second = pl1.classify(l0p0) != pl1.classify(l0p1);
			vector3 ll0 = l0p0 - l0p1;
			vector3 ll1 = l1p0 - l1p1;
			vector3 tl0 = ll0.getNormalized();
			vector3 nl0 = tl0 ^ vector3(0.0, 0.0, 1.0);
			if (first && second) {
				vector3 v0 = l0p1 - l1p1;
				if (std::fabs(ll1&nl0) < FG_EPS) return 0;
				auto point = (v0&nl0)*nl0 + (v0&nl0)*(ll1&tl0) / (ll1&nl0) * tl0 + l1p1;
				res.push_back(point);
				return 1;
			}
			return 0;
		}
		static int intersect_segment(const EllipticSegment& e, const vector3& l1p0, const vector3& l1p1, std::vector<vector3>& res) {
			auto l0p0 = e.P0();
			auto l0p1 = e.P1();
			auto c = e.Center();
			//auto t = l0.getTangent();
			auto t = l1p1 - l1p0;
			square_curve transformed_l0 = e.getCurve();//l1.curve.getTransformed(l1.getTransform().GetTransform());
			square_curve line_l1 = square_curve::line(l1p0, t);

			bool v;
			std::vector<vector3> pre_intersect;
			transformed_l0.find_intersection(v, line_l1, pre_intersect);
			if (v) {
				return 0;// IntersectionState::Incident;
			}
			auto n0 = l0p0 - c;
			auto n1 = l0p1 - c;
			auto t0 = c.normalToPoint(n0, l0p1);
			auto t1 = c.normalToPoint(n1, l0p0);
			int rc = 0;
			for (size_t i = 0U; i < pre_intersect.size(); i++) {
				auto vec = pre_intersect[i] - l1p0;
				auto sc = (t & vec) / t.lengthSq();
				if (sc >= 0.0 - FG_EPS && sc <= 1.0 + FG_EPS) {
					if ((t0 & (pre_intersect[i] - c)) >= 0.0f - FG_EPS &&
						(t1 & (pre_intersect[i] - c)) >= 0.0f - FG_EPS) {
						res.push_back(pre_intersect[i]);
						rc++;
					}
				}
			}
			return rc;
		}
	protected:
		static inline IntersectionState _intersect(std::vector<vector3>& ppl0, std::vector<vector3>& ppl1, const LineSegment& l0, const LineSegment& l1) {
			auto l0p0 = l0.P0();
			auto l0p1 = l0.P1();
			auto l1p0 = l1.P0();
			auto l1p1 = l1.P1();

			plane pl0 = plane::byTwoPointsAndNormal(l0p0, l0p1, vector3(0.0, 0.0, 1.0));
			bool first = pl0.classify(l1p0) != pl0.classify(l1p1);
			if ((l0.getTangent() ^ l1.getTangent()).lengthSq() < FG_EPS) {
				if (pl0.classify(l1p0) == 0) {
					if (l0p0 < l0p1) {
						if (l1p0 < l1p1) {
							if (l1p0 < l0p0) {
								if (l1p1 < l0p1) {
									if (l1p1 >= l0p0) { // l1p0, l0p0, l1p1, l0p1
										ppl0.insert(ppl0.end(), { l0p0, l1p1, l0p1 });
										ppl1.insert(ppl1.end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p0, l1p1, l0p0, l0p1
										return IntersectionState::Incident;
									}
								}
								else { // l1p0, l0p0, l0p1, l1p1
									ppl0.insert(ppl0.end(), { l0p0, l0p1 });
									ppl1.insert(ppl1.end(), { l1p0, l0p0, l0p1, l1p1 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (l1p0 <= l0p1) {
									if (l1p1 < l0p1) { // l0p0, l1p0, l1p1, l0p1
										ppl0.insert(ppl0.end(), { l0p0, l1p0, l1p1, l0p1 });
										ppl1.insert(ppl1.end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p0, l1p0, l0p1, l1p1
										ppl0.insert(ppl0.end(), { l0p0, l1p0, l0p1 });
										ppl1.insert(ppl1.end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p0, l0p1, l1p0, l1p1
									return IntersectionState::Incident;
								}
							}
						}
						else {
							if (l1p1 < l0p0) {
								if (l1p0 < l0p1) {
									if (l1p0 >= l0p0) { // l1p1, l0p0, l1p0, l0p1
										ppl0.insert(ppl0.end(), { l0p0, l1p0, l0p1 });
										ppl1.insert(ppl1.end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p1, l1p0, l0p0, l0p1
										return IntersectionState::Incident;
									}
								}
								else { // l1p1, l0p0, l0p1, l1p0
									ppl0.insert(ppl0.end(), { l0p0, l0p1 });
									ppl1.insert(ppl1.end(), { l1p1, l0p0, l0p1, l1p0 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (l1p1 <= l0p1) {
									if (l1p0 < l0p1) { // l0p0, l1p1, l1p0, l0p1
										ppl0.insert(ppl0.end(), { l0p0, l1p1, l1p0, l0p1 });
										ppl1.insert(ppl1.end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p0, l1p1, l0p1, l1p0
										ppl0.insert(ppl0.end(), { l0p0, l1p1, l0p1 });
										ppl1.insert(ppl1.end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p0, l0p1, l1p1, l1p0
									return IntersectionState::Incident;
								}
							}
						}
					}
					else {
						if (l1p0 < l1p1) {
							if (l1p0 < l0p1) {
								if (l1p1 < l0p0) {
									if (l1p1 >= l0p1) { // l1p0, l0p1, l1p1, l0p0
										ppl0.insert(ppl0.end(), { l0p1, l1p1, l0p0 });
										ppl1.insert(ppl1.end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p0, l1p1, l0p1, l0p0
										return IntersectionState::Incident;
									}
								}
								else { // l1p0, l0p1, l0p0, l1p1
									ppl0.insert(ppl0.end(), { l0p1, l0p0 });
									ppl1.insert(ppl1.end(), { l1p0, l0p1, l0p0, l1p1 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (l1p0 <= l0p0) {
									if (l1p1 < l0p0) { // l0p1, l1p0, l1p1, l0p0
										ppl0.insert(ppl0.end(), { l0p1, l1p0, l1p1, l0p0 });
										ppl1.insert(ppl1.end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p1, l1p0, l0p0, l1p1
										ppl0.insert(ppl0.end(), { l0p1, l1p0, l0p0 });
										ppl1.insert(ppl1.end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p1, l0p0, l1p0, l1p1
									return IntersectionState::Incident;
								}
							}
						}
						else {
							if (l1p1 < l0p1) {
								if (l1p0 < l0p0) {
									if (l1p0 >= l0p1) { // l1p1, l0p1, l1p0, l0p0
										ppl0.insert(ppl0.end(), { l0p1, l1p0, l0p0 });
										ppl1.insert(ppl1.end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p1, l1p0, l0p1, l0p0
										return IntersectionState::Incident;
									}
								}
								else { // l1p1, l0p1, l0p0, l1p0
									ppl0.insert(ppl0.end(), { l0p1, l0p0 });
									ppl1.insert(ppl1.end(), { l1p1, l0p1, l0p0, l1p0 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (l1p0 <= l0p0) {
									if (l1p1 < l0p0) { // l0p1, l1p1, l1p0, l0p0
										ppl0.insert(ppl0.end(), { l0p1, l1p1, l1p0, l0p0 });
										ppl1.insert(ppl1.end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p1, l1p1, l0p0, l1p0
										ppl0.insert(ppl0.end(), { l0p1, l1p1, l0p0 });
										ppl1.insert(ppl1.end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p1, l0p0, l1p1, l1p0
									return IntersectionState::Incident;
								}
							}
						}
					}
				}
				return IntersectionState::NotIntersect;
			}
			plane pl1 = plane::byTwoPointsAndNormal(l1p0, l1p1, vector3(0.0, 0.0, 1.0));
			bool second = pl1.classify(l0p0) != pl1.classify(l0p1);
			vector3 ll0 = l0p0 - l0p1;
			vector3 ll1 = l1p0 - l1p1;
			vector3 tl0 = ll0.getNormalized();
			vector3 nl0 = tl0 ^ vector3(0.0, 0.0, 1.0);
			if (first && second) {
				vector3 v0 = l0p1 - l1p1;
				if (std::fabs(ll1&nl0) < FG_EPS) return IntersectionState::NotIntersect;
				auto point = (v0&nl0)*nl0 + (v0&nl0)*(ll1&tl0) / (ll1&nl0) * tl0 + l1p1;
				ppl0.push_back(point);
				ppl1.push_back(point);
			}
			else {
				if (first) {
					vector3 a = l1p0 - l0p0;
					vector3 b = l1p1 - l0p0;
					auto x = (nl0 & b) / (nl0 & (b - a));
					if (x > -FG_EPS && x < 1 + FG_EPS)
						ppl1.push_back(l0p0 + x * a + (1.0 - x) * b);
				}
				else if (second) {
					vector3 a = l0p0 - l1p0;
					vector3 b = l0p1 - l1p0;
					vector3 tl1 = ll1.getNormalized();
					vector3 nl1 = tl1 ^ vector3(0.0, 0.0, 1.0);
					auto x = (nl1 & b) / (nl1 & (b - a));
					if (x > -FG_EPS && x < 1 + FG_EPS)
						ppl0.push_back(l1p0 + x * a + (1.0 - x) * b);
				}
			}


			return (IntersectionState)((ppl0.size() ? (int)IntersectionState::IntersectFirst : (int)IntersectionState::NotIntersect) |
				(ppl1.size() ? (int)IntersectionState::IntersectSecond : (int)IntersectionState::NotIntersect));
		}
		static inline IntersectionState _intersect(std::vector<vector3>& ppl0, std::vector<vector3>& ppl1, const LineSegment& l0, const EllipticSegment& l1) {
			auto l0p0 = l0.P0();
			auto l0p1 = l0.P1();
			auto l1p0 = l1.P0();
			auto l1p1 = l1.P1();
			auto c = l1.Center();
			//auto t = l0.getTangent();
			auto t = l0p1 - l0p0;
			square_curve line_l0 = square_curve::line(l0p0, t);
			square_curve transformed_l1 = l1.getCurve();//l1.curve.getTransformed(l1.getTransform().GetTransform());

			bool v;
			std::vector<vector3> pre_intersect;
			transformed_l1.find_intersection(v, line_l0, pre_intersect);
			if (v) {
				return IntersectionState::Incident;
			}
			auto n0 = l1p0 - c;
			auto n1 = l1p1 - c;
			auto t0 = c.normalToPoint(n0, l1p1);
			auto t1 = c.normalToPoint(n1, l1p0);
			for (size_t i = 0U; i < pre_intersect.size(); i++) {
				auto vec = pre_intersect[i] - l0p0;
				auto sc = (t & vec) / t.lengthSq();
				if (sc >= 0.0 - FG_EPS && sc <= 1.0 + FG_EPS) {
					ppl0.push_back(pre_intersect[i]);
				}
				if ((t0 & (pre_intersect[i] - c)) >= 0.0f - FG_EPS &&
					(t1 & (pre_intersect[i] - c)) >= 0.0f - FG_EPS) {
					ppl1.push_back(pre_intersect[i]);
				}
			}
			//        auto on_line = [](const vector3& x, const vector3& t, const vector3& p) { return std::fabs(t.y * (x.x - p.x) + t.x * (x.y - p.y)) < FG_EPS; };

			//        if(on_line(l1p0, t0, l0p0) && on_line(l1p1, t0, l0p0) ){
			//            // we're on line P0
			//            bool sml1p0 = (t0 & (l1p0 - l0p0)) < 0;
			//            bool sml1p1 = (t0 & (l1p1 - l0p0)) < 0;
			//            if(sml1p0 != sml1p1){
			//                if(sml1p0){
			//                    ppl1.push_back(l1p0);
			//                    return IntersectionState::PartialyIncidentSecond;
			//                }
			//                ppl1.push_back(l1p1);
			//                return IntersectionState::PartialyIncidentSecond;
			//            }
			//        }
			//        if(on_line(l1p0, t1, l0p1) && on_line(l1p1, t1, l0p1) ){
			//            // we're on line P0
			//            bool sml1p0 = (t1 & (l1p0 - l0p1)) < 0;
			//            bool sml1p1 = (t1 & (l1p1 - l0p1)) < 0;
			//            if(sml1p0 != sml1p1){
			//                if(sml1p0){
			//                    ppl1.push_back(l1p0);
			//                    return IntersectionState::PartialyIncidentSecond;
			//                }
			//                ppl1.push_back(l1p1);
			//                return IntersectionState::PartialyIncidentSecond;
			//            }
			//        }
			//        auto intersect_ray = [](vector3& r, const vector3 l0, const vector3 l1, const vector3 tn, const vector3 p){
			//            vector3 a = l0 - p;
			//            vector3 b = l1 - p;
			//            vector3 t = tn.getNormalized();
			//            vector3 n = t ^ vector3(0.0,0.0,1.0);
			//            auto x = (n & b) / (n & (b-a));
			//            r = (x * a + (1.0 - x) * b);
			//            return (r & t) > 0 ? false : (r != (r = r + p));
			//        };
			//        vector3 res;
			//        if(intersect_ray(res, l1p0, l1p1, t0, l0p0)){
			//            ppl1.push_back(res);
			//        }
			//        if(intersect_ray(res, l1p0, l1p1, t1, l0p1)){
			//            ppl1.push_back(res);
			//        }

			return IntersectionState((ppl0.size() ? (int)IntersectionState::IntersectFirst : (int)IntersectionState::NotIntersect) |
				(ppl1.size() ? (int)IntersectionState::IntersectSecond : (int)IntersectionState::NotIntersect));
			//auto dd = ((l0p0+l0p1-2.0*l1p0)&n0);
			//if(dd < FG_EPS) {
			//    return (p.size()) ? IntersectionState::IncidentAndIntersect : IntersectionState::Incident;
			//}
			//auto dt = ((l0p1 - l0p0)&t0)+((l0p0 - l0p1)&t0)*((l0p1-l1p0)&n0)/dd;
			//if(dt <= 0) {
			//    p.push_back(dt * t0 / t0.lengthSq() + l1p0);
			//}
			//dd = ((l0p0+l0p1-2.0*l1p1)&n1);
			//if(dd < FG_EPS) {
			//    return (p.size()) ? IntersectionState::IncidentAndIntersect : IntersectionState::Incident;
			//}
			//dt = ((l0p1 - l0p0)&t1)+((l0p0 - l0p1)&t1)*((l0p1-l1p1)&n1)/((l0p0+l0p1-2.0*l1p1)&n1);
			//if(dt <= 0) {
			//    p.push_back(dt * t1 / t1.lengthSq() + l1p1);
			//}

		}
		static inline IntersectionState _intersect(std::vector<vector3>& ppl0, std::vector<vector3>& ppl1, const EllipticSegment& l0, const LineSegment& l1) {
			auto res = _intersect(ppl1, ppl0, l1, l0);
			return (IntersectionState)(((res & IntersectionState::IntersectFirst) != (res & IntersectionState::IntersectSecond))
				? res ^ IntersectionState::Intersect : res);
		}
		static inline IntersectionState _intersect(std::vector<vector3>& ppl0, std::vector<vector3>& ppl1, const EllipticSegment& l0, const EllipticSegment& l1) {
			auto l0p0 = l0.P0();
			auto l0p1 = l0.P1();
			auto l1p0 = l1.P0();
			auto l1p1 = l1.P1();
			auto c0 = l0.Center();
			auto c1 = l1.Center();
			//auto t = l0.getTangent();
			square_curve line_l0 = l0.getCurve();
			square_curve line_l1 = l1.getCurve();//l1.curve.getTransformed(l1.getTransform().GetTransform());

			bool v;
			std::vector<vector3> pre_intersect;
			line_l1.find_intersection(v, line_l0, pre_intersect);

			if (v) {
				auto phil0p0 = atan2(l0p0.y, l0p0.x);
				auto phil0p1 = atan2(l0p1.y, l0p1.x);
				bool f0 = false;
				if (std::fabs(phil0p0 - phil0p1) > PI) f0 = true;
				auto phil1p0 = atan2(l1p0.y, l1p0.x);
				auto phil1p1 = atan2(l1p1.y, l1p1.x);
				bool f1 = false;
				std::vector<vector3>* pppl0 = &ppl0;
				std::vector<vector3>* pppl1 = &ppl1;
				if (std::fabs(phil1p0 - phil1p1) > PI) {
					if (f0 == false) {
						auto tmp = l0p0;  // l0 <-> l1
						l0p0 = l1p0;
						l1p0 = tmp;
						tmp = l0p1;
						l0p1 = l1p1;
						l1p1 = tmp;

						auto tmpp = phil0p0;  // phil0 <-> phil1
						phil0p0 = phil1p0;
						phil1p0 = tmpp;
						tmpp = phil0p1;
						phil0p1 = phil1p1;
						phil1p1 = tmpp;

						auto tmppp = pppl0;  // l0 <-> l1
						pppl0 = pppl1;
						pppl1 = tmppp;

						f0 = true;
						f1 = false;
					}
					else
						f1 = true;
				}

				if (f0 == true) {
					if (f1 == false) {
						if (phil1p0 > phil1p1) {
							if (phil0p0 < phil0p1) {
								if (phil1p0 >= phil0p1) {
									if (phil1p1 >= phil0p1) {
										//p.insert(p.end(), {l0p1, l1p1, l1p0, l0p0}); +
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p1, l0p1, l1p0, l0p0}); +
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								if (phil1p1 < phil0p0) {
									if (phil1p0 < phil0p0) {
										//p.insert(p.end(), {l0p1, l1p1, l1p0, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p1, l1p1, l0p0, l1p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								return IntersectionState::Incident;
							}
							else {
								if (phil1p1 >= phil0p1) {
									if (phil1p0 >= phil0p1) {
										//p.insert(p.end(), {l0p1, l1p0, l1p1, l0p0}); +
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p0, l0p1, l1p1, l0p0}); +
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								if (phil1p0 < phil0p0) {
									if (phil1p1 < phil0p0) {
										//p.insert(p.end(), {l0p1, l1p0, l1p1, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p1, l1p0, l0p0, l1p1});
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								return IntersectionState::Incident;
							}
						}
						else {
							if (phil0p1 < phil0p0) {
								if (phil1p0 >= phil0p0) {
									if (phil1p1 >= phil0p0) {
										//p.insert(p.end(), {l0p0, l1p1, l1p0, l0p1}); +
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p1, l0p0, l1p0, l0p1}); +
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								if (phil1p1 < phil0p1) {
									if (phil1p0 < phil0p1) {
										//p.insert(p.end(), {l0p0, l1p1, l1p0, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p0, l1p1, l0p1, l1p0});
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								return IntersectionState::Incident;
							}
							else {
								if (phil1p1 >= phil0p0) {
									if (phil1p0 >= phil0p0) {
										//p.insert(p.end(), {l0p0, l1p0, l1p1, l0p1}); +
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p0, l0p0, l1p1, l0p1}); +
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								if (phil1p0 < phil0p1) {
									if (phil1p1 < phil0p1) {
										//p.insert(p.end(), {l0p0, l1p0, l1p1, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p0, l1p0, l0p1, l1p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								return IntersectionState::Incident;
							}
						}
					}
					else {
						if (phil0p0 < phil0p1) {
							if (phil1p0 < phil1p1) {
								if (phil0p1 < phil1p1) {
									if (phil0p0 < phil1p0) {
										//p.insert(p.end(), {l0p1, l1p1, l0p0, l1p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p1, l1p1, l1p0, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else {
									if (phil0p0 < phil1p0) {
										//p.insert(p.end(), {l1p1, l0p1, l0p0, l1p0});
										pppl0->insert(pppl0->end(), { l0p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p1, l0p1, l1p0, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
							}
							else {
								if (phil0p1 < phil1p0) {
									if (phil0p0 < phil1p1) {
										//p.insert(p.end(), {l0p1, l1p0, l0p0, l1p1});
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p1, l1p0, l1p1, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else {
									if (phil0p0 < phil1p1) {
										//p.insert(p.end(), {l1p0, l0p1, l0p0, l1p1});
										pppl0->insert(pppl0->end(), { l0p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p0, l0p1, l1p1, l0p0});
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
							}
						}
						else {
							if (phil1p0 < phil1p1) {
								if (phil0p0 < phil1p1) {
									if (phil0p1 < phil1p0) {
										//p.insert(p.end(), {l0p0, l1p1, l0p1, l1p0});
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p0, l1p1, l1p0, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else {
									if (phil0p1 < phil1p0) {
										//p.insert(p.end(), {l1p1, l0p0, l0p1, l1p0});
										pppl0->insert(pppl0->end(), { l0p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p1, l0p0, l1p0, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
							}
							else {
								if (phil0p0 < phil1p0) {
									if (phil0p1 < phil1p1) {
										//p.insert(p.end(), {l0p0, l1p0, l0p1, l1p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l0p0, l1p0, l1p1, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else {
									if (phil0p1 < phil1p1) {
										//p.insert(p.end(), {l1p0, l0p0, l0p1, l1p1});
										pppl0->insert(pppl0->end(), { l0p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else {
										//p.insert(p.end(), {l1p0, l0p0, l1p1, l0p1});
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
							}
						}
					}
				}
				else {

					if (phil0p0 < phil0p1) {
						if (phil1p0 < phil1p1) {
							if (phil1p0 < phil0p0) {
								if (phil1p1 < phil0p1) {
									if (phil1p1 > phil0p0) { // l1p0, l0p0, l1p1, l0p1
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p0, l1p1, l0p0, l0p1
										return IntersectionState::Incident;
									}
								}
								else { // l1p0, l0p0, l0p1, l1p1
									pppl0->insert(pppl0->end(), { l0p0, l0p1 });
									pppl1->insert(pppl1->end(), { l1p0, l0p0, l0p1, l1p1 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (phil1p0 <= phil0p1) {
									if (phil1p1 < phil0p1) { // l0p0, l1p0, l1p1, l0p1
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p0, l1p0, l0p1, l1p1
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p0, l0p1, l1p0, l1p1
									return IntersectionState::Incident;
								}
							}
						}
						else {
							if (phil1p1 < phil0p0) {
								if (phil1p0 < phil0p1) {
									if (phil1p0 > phil0p0) { // l1p1, l0p0, l1p0, l0p1
										pppl0->insert(pppl0->end(), { l0p0, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p1, l1p0, l0p0, l0p1
										return IntersectionState::Incident;
									}
								}
								else { // l1p1, l0p0, l0p1, l1p0
									pppl0->insert(pppl0->end(), { l0p0, l0p1 });
									pppl1->insert(pppl1->end(), { l1p1, l0p0, l0p1, l1p0 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (phil1p1 <= phil0p1) {
									if (phil1p0 < phil0p1) { // l0p0, l1p1, l1p0, l0p1
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l1p0, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p0, l1p1, l0p1, l1p0
										pppl0->insert(pppl0->end(), { l0p0, l1p1, l0p1 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p0, l0p1, l1p1, l1p0
									return IntersectionState::Incident;
								}
							}
						}
					}
					else {
						if (phil1p0 < phil1p1) {
							if (phil1p0 < phil0p1) {
								if (phil1p1 < phil0p0) {
									if (phil1p1 > phil0p1) { // l1p0, l0p1, l1p1, l0p0
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p1, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p0, l1p1, l0p1, l0p0
										return IntersectionState::Incident;
									}
								}
								else { // l1p0, l0p1, l0p0, l1p1
									pppl0->insert(pppl0->end(), { l0p1, l0p0 });
									pppl1->insert(pppl1->end(), { l1p0, l0p1, l0p0, l1p1 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (phil1p0 <= phil0p0) {
									if (phil1p1 < phil0p0) { // l0p1, l1p0, l1p1, l0p0
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p1, l1p0, l0p0, l1p1
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p0, l0p0, l1p1 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p1, l0p0, l1p0, l1p1
									return IntersectionState::Incident;
								}
							}
						}
						else {
							if (phil1p1 < phil0p1) {
								if (phil1p0 < phil0p0) {
									if (phil1p0 > phil0p1) { // l1p1, l0p1, l1p0, l0p0
										pppl0->insert(pppl0->end(), { l0p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l1p1, l1p0, l0p1, l0p0
										return IntersectionState::Incident;
									}
								}
								else { // l1p1, l0p1, l0p0, l1p0
									pppl0->insert(pppl0->end(), { l0p1, l0p0 });
									pppl1->insert(pppl1->end(), { l1p1, l0p1, l0p0, l1p0 });
									return IntersectionState::IncidentAndIntersect;
								}
							}
							else {
								if (phil1p1 <= phil0p0) {
									if (phil1p0 < phil0p0) { // l0p1, l1p1, l1p0, l0p0
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l1p0, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
									else { // l0p1, l1p1, l0p0, l1p0
										pppl0->insert(pppl0->end(), { l0p1, l1p1, l0p0 });
										pppl1->insert(pppl1->end(), { l1p1, l0p0, l1p0 });
										return IntersectionState::IncidentAndIntersect;
									}
								}
								else { // l0p1, l0p0, l1p1, l1p0
									return IntersectionState::Incident;
								}
							}
						}
					}
					//if(std::min(phil0p0, phil0p1) < std::max(phil1p0, phil1p1) + FG_EPS &&
					//   std::min(phil1p0, phil1p1) < std::max(phil0p0, phil0p1) + FG_EPS ){
					//    p.insert(p.end(), {l0p0, l1p0, l0p1, l1p1});
					//    std::sort(p.begin(), p.end(), [](vector3& lhs, vector3& rhs){return atan2(lhs.y, lhs.x) < atan2(rhs.y, rhs.x);});
					//    return IntersectionState::IncidentAndIntersect;
					//}
				}

				return IntersectionState::Incident;
			}

			auto n0p0 = l0p0 - c0;
			auto n0p1 = l0p1 - c0;
			auto n1p0 = l1p0 - c1;
			auto n1p1 = l1p1 - c1;
			auto t0p0 = c0.normalToPoint(n0p0, l0p1);
			auto t0p1 = c0.normalToPoint(n0p1, l0p0);
			auto t1p0 = c1.normalToPoint(n1p0, l1p1);
			auto t1p1 = c1.normalToPoint(n1p1, l1p0);

			for (size_t i = 0U; i < pre_intersect.size(); i++) {
				if ((t1p0 & (pre_intersect[i] - c1)) >= 0.0f - FG_EPS &&
					(t1p1 & (pre_intersect[i] - c1)) >= 0.0f - FG_EPS) {
					ppl1.push_back(pre_intersect[i]);
				}
				if ((t0p0 & (pre_intersect[i] - c0)) >= 0.0f - FG_EPS &&
					(t0p1 & (pre_intersect[i] - c0)) >= 0.0f - FG_EPS) {
					ppl0.push_back(pre_intersect[i]);
				}
			}
			//        {// L0P0
			//            square_curve line = square_curve::line(l0p0, t0p0);
			//            pre_intersect.clear();
			//            line.find_intersection(v, line_l1, pre_intersect);

			//            for(size_t i = 0U; i<pre_intersect.size(); i++){
			//                if((t0p0 & (pre_intersect[i] - l0p0)) <= 0.0+FG_EPS){
			//                    if((t1p0 & (pre_intersect[i]-c1)) >= 0.0 - FG_EPS &&
			//                       (t1p1 & (pre_intersect[i]-c1)) >= 0.0 - FG_EPS)
			//                        ppl1.push_back(pre_intersect[i]);
			//                }
			//            }
			//        }
			//        {// L0P1
			//            square_curve line = square_curve::line(l0p1, t0p1);
			//            pre_intersect.clear();
			//            line.find_intersection(v, line_l1, pre_intersect);

			//            for(int i = 0; i<pre_intersect.size(); i++){
			//                if((t0p1 & (pre_intersect[i] - l0p1)) <= 0.0 + FG_EPS){
			//                    if((t1p0 & (pre_intersect[i]-c1)) >= 0.0 - FG_EPS &&
			//                       (t1p1 & (pre_intersect[i]-c1)) >= 0.0 - FG_EPS)
			//                        ppl1.push_back(pre_intersect[i]);
			//                }
			//            }
			//        }
			//        {// L1P0
			//            square_curve line = square_curve::line(l1p0, t1p0);
			//            pre_intersect.clear();
			//            line.find_intersection(v, line_l0, pre_intersect);

			//            for(int i = 0; i<pre_intersect.size(); i++){
			//                if((t1p0 & (pre_intersect[i] - l1p0)) <= 0.0 + FG_EPS){
			//                    if((t0p0 & (pre_intersect[i]-c0)) >= 0.0 - FG_EPS &&
			//                       (t0p1 & (pre_intersect[i]-c0)) >= 0.0 - FG_EPS)
			//                        ppl0.push_back(pre_intersect[i]);
			//                }
			//            }
			//        }
			//        {// L1P1
			//            square_curve line = square_curve::line(l1p1, t1p1);
			//            pre_intersect.clear();
			//            line.find_intersection(v, line_l0, pre_intersect);

			//            for(int i = 0; i<pre_intersect.size(); i++){
			//                if((t1p1 & (pre_intersect[i] - l1p1)) <= 0.0 + FG_EPS){
			//                    if((t0p0 & (pre_intersect[i]-c0)) >= 0.0 - FG_EPS &&
			//                       (t0p1 & (pre_intersect[i]-c0)) >= 0.0 - FG_EPS)
			//                        ppl0.push_back(pre_intersect[i]);
			//                }
			//            }
			//        }

			return IntersectionState((ppl0.size() ? (int)IntersectionState::IntersectFirst : (int)IntersectionState::NotIntersect) |
				(ppl1.size() ? (int)IntersectionState::IntersectSecond : (int)IntersectionState::NotIntersect));
		}

		static inline ClassificationState _classify(const LineSegment& splitter, const LineSegment& line) {
			int r = splitter.classify(line.P0());
			int rl = splitter.classify(line.P1());
			return (r == -rl) ? (r == 0 ? ClassificationState::Incident : ClassificationState::Cross) : (r == 0 ? (ClassificationState)rl : (ClassificationState)r);
			//return (r == splitter.classify(line.P1())) ? (ClassificationState)r : ClassificationState::Cross;
		}
		static inline ClassificationState _classify(const EllipticSegment& splitter, const LineSegment& line) {
			int r = splitter.classify(line.P0());
			int rl = splitter.classify(line.P1());
			return (r == -rl) ? (r == 0 ? ClassificationState::Incident : ClassificationState::Cross) : (r == 0 ? (ClassificationState)rl : (ClassificationState)r);
			//return (r == splitter.classify(line.P1())) ? (ClassificationState)r : ClassificationState::Cross;
			//int r = splitter.classify(line.P0());
			//return (r == splitter.classify(line.P1())) ? (r ? (ClassificationState)r : ClassificationState::Cross) : ClassificationState::Cross;
		}
		static inline ClassificationState _classify(const LineSegment& splitter, const EllipticSegment& line) {
			auto p0 = line.P0();
			auto p1 = line.P1();
			int r = splitter.classify(p0);
			int rl = splitter.classify(p1);
			if (r == -rl) {
				// middle - это точка, которая лежит на середне дуги между p0 и p1
				if (r == 0) return (ClassificationState)splitter.classify(line.middle());
				return ClassificationState::Cross;
			}
			else {
				// cr - с какой стороны лежит точка пересечения касательных к дуге, проведённых в точках p0 и p1
				int cr = splitter.classify(line.getCurve().control_point(p0, p1));
				return (r == 0) ? (cr == rl ? (ClassificationState)rl : ClassificationState::Cross) : (cr == r ? (ClassificationState)r : ClassificationState::Cross);
			}
		}
		static inline ClassificationState _classify(const EllipticSegment& splitter, const EllipticSegment& line) {
			int r = splitter.classify(line.P0());
			int rl = splitter.classify(line.P1());
			if (r == -rl) {
				if (r == 0) return (ClassificationState)splitter.classify(line.middle());
				return ClassificationState::Cross;
			}
			else {
				// вектор, который перпендикулярен к вектору p1-p0 (не единичной длины) сплиттера
				auto n = splitter.getNormal();
				bool v;
				std::vector<vector3> inter;
				// l проходит через p1 и p0
				auto l = square_curve::line(n, -(n & splitter.P0()));
				// ищутся пересечения l с line, пересечени записываются в inter
				line.getCurve().find_intersection(v, l, inter);
				// если количество пересечений == 2
				if (inter.size() == 2) {
					return (line.pointCast(inter[0]) && line.pointCast(inter[1]) && splitter.classify(inter[0]) == 0 && splitter.classify(inter[1]) == 0) ? ClassificationState::Cross :
						(r == 0 ? (ClassificationState)rl : (ClassificationState)r);
				}
				return (r == 0 ? (ClassificationState)rl : (ClassificationState)r);
			}
		}
	};
}