#pragma once
#include "stdafx.h"
#include "fg_math.h"
#include "g_intersector.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT RawMesh2 {
	public:
		using Triangle = std::tuple<size_t, size_t, size_t>;
		std::vector<vector3> points;
		std::vector<Triangle> triangles;
		inline RawMesh2(const std::vector<vector3>& points, const std::vector<Triangle>& triangles) :
			points{ points } {
			this->triangles.resize(triangles.size());
			for (size_t i = 0; i < triangles.size(); i++) {
				std::array<size_t, 3> p;
				std::tie(p[0], p[1], p[2]) = triangles[i];
				std::sort(p.begin(), p.end());
				this->triangles[i] = std::make_tuple(p[0], p[1], p[2]);
			}
		}
	};

	enum class GeometryType {
		None = 0x0,
		Vertex = 0x1,
		Edge = 0x2,
		Triangle = 0x3
	};

	class FEMCADGEOMSHARED_EXPORT Mesh2 {
	public:
		using Edge = std::pair<size_t, size_t>;
		using Triangle = std::tuple<size_t, size_t, size_t>;
		using EdgeIndex = size_t;
		using TriangleIndex = size_t;
		using TreeTriangleIndex = size_t;

		inline Mesh2(const RawMesh2& mesh) : points{ mesh.points } {
			std::map<Edge, std::vector<size_t>> all_edges;
			point_edges.resize(points.size());
			std::vector<std::pair<rect, TriangleIndex>> rects(mesh.triangles.size());
			rect brect{};
			triangles.resize(mesh.triangles.size(), EmptyTriangle);
			for (size_t i = 0; i < mesh.triangles.size(); i++) {
				size_t p0, p1, p2;

				std::tie(p0, p1, p2) = mesh.triangles[i];
				all_edges[std::make_pair(p0, p1)].push_back(i);
				all_edges[std::make_pair(p0, p2)].push_back(i);
				all_edges[std::make_pair(p1, p2)].push_back(i);

				rects[i] = std::make_pair(rect(points[p0], points[p1]).add_point(points[p2]), i);
				brect.add_rect(rects[i].first);
			}
			size_t p{};
			edges.resize(all_edges.size());
			edge_triangles.resize(all_edges.size());
			// дерево для поиска тр-ков
			triangle_lookup = std::make_unique<TriangleLookup>(TriangleLookup(16, std::move(rects), brect.Min(), brect.Max()));
			rects.clear();
			for (auto& i : all_edges) {
				edges[p] = i.first;
				auto t0 = i.second.front();
				auto t1 = i.second.back();
				edge_triangles[p] = std::make_pair(i.second.front(), i.second.back());
				point_edges[i.first.first].insert(p);
				point_edges[i.first.second].insert(p);

				auto tt = triangles[t0];
				if (std::get<0>(tt) == 0xFFFFFFFF) std::get<0>(tt) = p;
				else if (std::get<1>(tt) == 0xFFFFFFFF && std::get<0>(tt) != p) std::get<1>(tt) = p;
				else if (std::get<2>(tt) == 0xFFFFFFFF && std::get<0>(tt) != p && std::get<1>(tt) != p) std::get<2>(tt) = p;
				triangles[t0] = tt;
				tt = triangles[t1];
				if (std::get<0>(tt) == 0xFFFFFFFF) std::get<0>(tt) = p;
				else if (std::get<1>(tt) == 0xFFFFFFFF && std::get<0>(tt) != p) std::get<1>(tt) = p;
				else if (std::get<2>(tt) == 0xFFFFFFFF && std::get<0>(tt) != p && std::get<1>(tt) != p) std::get<2>(tt) = p;
				triangles[t1] = tt;

				rects.push_back(std::make_pair(rect{ points[i.first.first], points[i.first.second] }, p));
				p++;
			}
			//edge_lookup = std::make_unique<TriangleLookup>(TriangleLookup(16, std::move(rects), brect.Min(), brect.Max()));
		}

		inline std::array<EdgeIndex, 3> collapseEdge(EdgeIndex edge, double weight = 0.5) {
			auto e = edges[edge];
			auto tris = edge_triangles[edge];
			auto t0 = triangles[tris.first];
			auto t1 = triangles[tris.second];
			size_t et0 = std::get<0>(t0);
			size_t ot0;
			if (et0 == edge) {
				et0 = std::get<1>(t0);
				if (edges[et0].first == e.second || edges[et0].second == e.second) {
					ot0 = std::get<2>(t0);
				}
				else {
					ot0 = et0;
					et0 = std::get<2>(t0);
				}
			}
			else {
				if (edges[et0].first == e.second || edges[et0].second == e.second) {
					ot0 = std::get<2>(t0);
					ot0 = (ot0 == edge) ? std::get<1>(t0) : ot0;
				}
				else {
					ot0 = et0;
					et0 = std::get<2>(t0);
					et0 = (et0 == edge) ? std::get<1>(t0) : et0;
				}
			}
			/*size_t ot0 = std::get<0>(t0) == edge ? (std::get<1>(t0) == et0 ? std::get<2>(t0) : std::get<1>(t0)) :
				(std::get<0>(t0) == et0 ? (std::get<1>(t0) == edge ? std::get<2>(t0) : std::get<1>(t0)) :
				(std::get<1>(t0) == edge ? std::get<0>(t0) : std::get<1>(t0)));*/
			size_t et1 = std::get<0>(t1), ot1;
			if (et1 == edge) {
				et1 = std::get<1>(t1);
				if (edges[et1].first == e.second || edges[et1].second == e.second) {
					ot1 = std::get<2>(t1);
				}
				else {
					ot1 = et1;
					et1 = std::get<2>(t1);
				}
			}
			else {
				if (edges[et1].first == e.second || edges[et1].second == e.second) {
					ot1 = std::get<2>(t1);
					ot1 = (ot1 == edge) ? std::get<1>(t1) : ot1;
				}
				else {
					ot1 = et1;
					et1 = std::get<2>(t1);
					et1 = (et1 == edge) ? std::get<1>(t1) : et1;
				}
			}
			//et1 = (et1 == edge) ? std::get<1>(t1) : et1;
			//et1 = edges[et1].first == e.second || edges[et1].second == e.second ? et1 : std::get<2>(t1);
			//size_t ot1 = std::get<0>(t1) == edge ? (std::get<1>(t1) == et1 ? std::get<2>(t1) : std::get<1>(t1)) :
			//	(std::get<0>(t1) == et1 ? (std::get<1>(t1) == edge ? std::get<2>(t1) : std::get<1>(t1)) :
			//	(std::get<1>(t1) == edge ? std::get<0>(t1) : std::get<1>(t1)));

			if (edge_triangles[et0].first == edge_triangles[et0].second &&
				edge_triangles[ot0].first == edge_triangles[ot0].second &&
				edge_triangles[ot0].first == edge_triangles[et0].second ||
				edge_triangles[et1].first == edge_triangles[et1].second &&
				edge_triangles[ot1].first == edge_triangles[ot1].second &&
				edge_triangles[ot1].first == edge_triangles[et1].second)
				return{ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

			//remove_edge_from_tree(edge);
			//remove_edge_from_tree(et0);
			//if (et0 != et1) remove_edge_from_tree(et1);

			auto tt0i = edge_triangles[et0].first == tris.first ? edge_triangles[et0].second : edge_triangles[et0].first;
			auto& tt0 = triangles[tt0i];
			auto old_t = tt0;
			if (std::get<0>(tt0) == et0) std::get<0>(tt0) = ot0;
			else if (std::get<1>(tt0) == et0) std::get<1>(tt0) = ot0;
			else if (std::get<2>(tt0) == et0) std::get<2>(tt0) = ot0;
			fix_triangle_rect(old_t, tt0i);

			auto tt1i = edge_triangles[et1].first == tris.first ? edge_triangles[et1].second : edge_triangles[et1].first;
			auto& tt1 = triangles[tt1i];
			old_t = tt1;
			if (std::get<0>(tt1) == et1) std::get<0>(tt1) = ot1;
			else if (std::get<1>(tt1) == et1) std::get<1>(tt1) = ot1;
			else if (std::get<2>(tt1) == et1) std::get<2>(tt1) = ot1;
			fix_triangle_rect(old_t, tt1i);

			std::array<size_t, 3> be;
			size_t sz = triangles.size() - 1;
			remove_triangle_from_tree(tris.first);
			if (tris.first != tris.second) {
				remove_triangle_from_tree(tris.second);
			}

			if (tris.first == sz) {
				triangles.pop_back(); sz--;
			}
			else {
				if (tris.second == sz) {
					triangles.pop_back(); sz--;
				}
				if (tris.first == sz) {
					triangles.pop_back(); sz--;
				}
				else {
					std::tie(be[0], be[1], be[2]) = triangles[tris.first] = triangles.back();
					for (size_t i{}; i < 3; i++) {
						if (edge_triangles[be[i]].first == sz) edge_triangles[be[i]].first = tris.first;
						if (edge_triangles[be[i]].second == sz) edge_triangles[be[i]].second = tris.first;
					}
					triangles.pop_back(); sz--;
				}
			}
			if (tris.second == sz) {
				triangles.pop_back(); sz--;
			}
			else if (tris.first != tris.second && tris.second < sz) {
				std::tie(be[0], be[1], be[2]) = triangles[tris.second] = triangles.back();
				for (size_t i{}; i < 3; i++) {
					if (edge_triangles[be[i]].first == sz) edge_triangles[be[i]].first = tris.second;
					if (edge_triangles[be[i]].second == sz) edge_triangles[be[i]].second = tris.second;
				}
				triangles.pop_back();
			}
			for (auto i{ point_edges[e.second].begin() }; i != point_edges[e.second].end(); i++) {
				auto& p = edges[*i];
				if (p.first == e.second) { p.first = e.first; }
				else if (p.second == e.second) { p.second = e.first; }
			}
			point_edges[e.first].insert(std::begin(point_edges[e.second]), std::end(point_edges[e.second]));
			point_edges[e.first].erase(edge);
			point_edges[e.first].erase(et0);
			point_edges[e.first].erase(et1);
			// Old points
			auto pfold = points[e.first];
			auto psold = points[e.second];
			points[e.first] = (weight * points[e.first] + (1.0 - weight) * points[e.second]);
			std::swap(point_edges[e.second], point_edges.back());
			point_edges.pop_back();
			sz = point_edges.size();
			for (auto i{ point_edges[e.second].begin() }; i != point_edges[e.second].end(); i++) {
				auto& p = edges[*i];
				if (p.first == sz) {
					p.first = e.second;
					//fix_edge_rect(rect{ points[p.second], psold }, *i);
				}
				else if (p.second == sz) {
					p.second = e.second;
					//fix_edge_rect(rect{ points[p.first], psold }, *i);
				}
			}

			//for (auto i{ point_edges[e.first].begin() }; i != point_edges[e.first].end(); i++) {
			//	auto& p = edges[*i];
			//	if (p.first == e.first) {
			//		//fix_edge_rect(rect{ points[p.second], pfold }, *i);
			//	}
			//	else if (p.second == e.first) {
			//		//fix_edge_rect(rect{ points[p.first], pfold }, *i);
			//	}
			//}

			std::swap(points[e.second], points.back()); points.pop_back();

			sz = edges.size() - 1;
			be[0] = be[1] = be[2] = 0xFFFFFFFF;
			auto clean_up_last_triangle = [&](EdgeIndex e, size_t siz) {
				auto lt = triangles[edge_triangles[siz].first];
				if (std::get<0>(lt) == siz) std::get<0>(lt) = e;
				if (std::get<1>(lt) == siz) std::get<1>(lt) = e;
				if (std::get<2>(lt) == siz) std::get<2>(lt) = e;
				triangles[edge_triangles[siz].first] = lt;
				lt = triangles[edge_triangles[siz].second];
				if (std::get<0>(lt) == siz) std::get<0>(lt) = e;
				if (std::get<1>(lt) == siz) std::get<1>(lt) = e;
				if (std::get<2>(lt) == siz) std::get<2>(lt) = e;
				triangles[edge_triangles[siz].second] = lt;
			};
			if (edge == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
			else {
				clean_up_last_triangle(edge, sz);
				std::swap(edge_triangles[edge], edge_triangles.back());
				edge_triangles.pop_back();
				std::swap(edges[edge], edges.back());
				edges.pop_back(); sz--; be[0] = edge;
			}
			if (et0 == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
			else {
				clean_up_last_triangle(et0, sz);
				std::swap(edge_triangles[et0], edge_triangles.back());
				edge_triangles.pop_back();
				std::swap(edges[et0], edges.back()); edges.pop_back(); sz--; be[1] = et0;
			}
			if (et1 == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
			else if (et0 != et1) {
				clean_up_last_triangle(et1, sz);
				std::swap(edge_triangles[et1], edge_triangles.back());
				edge_triangles.pop_back();
				std::swap(edges[et1], edges.back()); edges.pop_back(); sz--; be[2] = et1;
			}

			return be;
		}
		inline std::tuple<EdgeIndex, EdgeIndex, EdgeIndex> subdivideEdge(EdgeIndex edge, double weight = 0.5) {
			auto e = edges[edge];
			auto t0i = edge_triangles[edge].first;
			auto t1i = edge_triangles[edge].second;

			auto tni = triangles.size();
			auto eni = edges.size();
			auto e0ni = edges.size() + 1;
			auto pni = points.size();
			points.push_back(weight * points[e.first] + (1.0 - weight) * points[e.second]);
			edges.push_back(std::make_pair(pni, e.second));
			EdgeIndex et0i;
			if (std::get<0>(triangles[t0i]) == edge) {
				et0i = std::get<1>(triangles[t0i]);
				et0i = edges[et0i].first == e.second || edges[et0i].second == e.second ? et0i :
					std::get<2>(triangles[t0i]);
			}
			else {
				et0i = std::get<0>(triangles[t0i]);
				et0i = edges[et0i].first == e.second || edges[et0i].second == e.second ? et0i :
					(std::get<1>(triangles[t0i]) == edge ? std::get<2>(triangles[t0i]) : std::get<1>(triangles[t0i]));
			}
			auto pp0i = edges[et0i].first == e.second ? edges[et0i].second : edges[et0i].first;
			edges.push_back(std::make_pair(pp0i, pni));
			point_edges.push_back({ edge, eni, e0ni });
			point_edges[e.second].erase(edge);
			//rect old_rect{ points[e.first], points[e.second] };
			edges[edge].second = pni;
			//fix_edge_rect(old_rect, edge);

			point_edges[e.second].insert(eni);
			point_edges[pp0i].insert(e0ni);
			if (edge_triangles[et0i].first == t0i)
				edge_triangles[et0i].first = tni;
			if (edge_triangles[et0i].second == t0i)
				edge_triangles[et0i].second = tni;
			edge_triangles.push_back(std::make_pair(tni, tni));
			edge_triangles.push_back(std::make_pair(tni, t0i));

			auto old_t = triangles[t0i];
			if (std::get<0>(old_t) == et0i) std::get<0>(old_t) = e0ni;
			else if (std::get<1>(old_t) == et0i) std::get<1>(old_t) = e0ni;
			else if (std::get<2>(old_t) == et0i) std::get<2>(old_t) = e0ni;
			std::swap(triangles[t0i], old_t);
			fix_triangle_rect(old_t, t0i);

			triangles.push_back(std::make_tuple(eni, e0ni, et0i));
			add_triangle_to_tree(triangles.size() - 1);

			//add_edge_to_tree(eni);
			//add_edge_to_tree(e0ni);
			EdgeIndex e1ni = 0xFFFFFFFF;
			if (t0i != t1i) {
				e1ni = edges.size();
				EdgeIndex et1i;
				tni = triangles.size();
				if (std::get<0>(triangles[t1i]) == edge) {
					et1i = std::get<1>(triangles[t1i]);
					et1i = edges[et1i].first == e.second || edges[et1i].second == e.second ? et1i :
						std::get<2>(triangles[t1i]);
				}
				else {
					et1i = std::get<0>(triangles[t1i]);
					et1i = edges[et1i].first == e.second || edges[et1i].second == e.second ? et1i :
						(std::get<1>(triangles[t1i]) == edge ? std::get<2>(triangles[t1i]) : std::get<1>(triangles[t1i]));
				}
				auto pp1i = edges[et1i].first == e.second ? edges[et1i].second : edges[et1i].first;
				edges.push_back(std::make_pair(pp1i, pni));
				point_edges.back().insert(e1ni);
				point_edges[pp1i].insert(e1ni);
				if (edge_triangles[et1i].first == t1i)
					edge_triangles[et1i].first = tni;
				if (edge_triangles[et1i].second == t1i)
					edge_triangles[et1i].second = tni;
				edge_triangles[edge_triangles.size() - 2].second = tni;
				edge_triangles.push_back(std::make_pair(tni, t1i));

				old_t = triangles[t1i];
				if (std::get<0>(old_t) == et1i) std::get<0>(old_t) = e1ni;
				else if (std::get<1>(old_t) == et1i) std::get<1>(old_t) = e1ni;
				else if (std::get<2>(old_t) == et1i) std::get<2>(old_t) = e1ni;
				std::swap(triangles[t1i], old_t);
				fix_triangle_rect(old_t, t1i);

				triangles.push_back(std::make_tuple(eni, e1ni, et1i));
				add_triangle_to_tree(triangles.size() - 1);
				//add_edge_to_tree(e1ni);
			}
			return std::make_tuple(eni, e0ni, e1ni);
		}
		inline std::tuple<size_t, size_t, size_t> triangleVertices(TriangleIndex t) const {
			auto tr = triangles[t];
			size_t v0, v1, v2;
			v0 = edges[std::get<0>(tr)].first;
			v1 = edges[std::get<0>(tr)].second;
			v2 = (edges[std::get<1>(tr)].first != v0 && edges[std::get<1>(tr)].first != v1) ? edges[std::get<1>(tr)].first : edges[std::get<1>(tr)].second;
			return std::make_tuple(v0, v1, v2);
		}
		inline EdgeIndex edgeIndex(TriangleIndex triangle, size_t v0, size_t v1) const {
			size_t e0, e1, e2;
			std::tie(e0, e1, e2) = triangles[triangle];
			if (edges[e0].first == v0 && edges[e0].second == v1) return e0;
			if (edges[e0].first == v1 && edges[e0].second == v0) return e0;
			if (edges[e1].first == v0 && edges[e1].second == v1) return e1;
			if (edges[e1].first == v1 && edges[e1].second == v0) return e1;
			if (edges[e2].first == v0 && edges[e2].second == v1) return e2;
			if (edges[e2].first == v1 && edges[e2].second == v0) return e2;
			return 0xFFFFFFFF;
		}
		inline matrix4x4 lcoords(TriangleIndex triangle) const {
			matrix4x4 m;
			size_t e0, e1, e2;
			std::tie(e0, e1, e2) = triangleVertices(triangle);

			m.data[0][0] = points[e0].x - points[e2].x;
			m.data[1][0] = points[e0].y - points[e2].y;
			m.data[2][0] = 0.0; // points[e0].z;
			m.data[3][0] = 0.0;
			m.data[0][1] = points[e1].x - points[e2].x;
			m.data[1][1] = points[e1].y - points[e2].y;
			m.data[2][1] = 0.0; // points[e1].z;
			m.data[3][1] = 0.0;
			m.data[0][2] = 0.0; // points[e2].x;
			m.data[1][2] = 0.0; // points[e2].y;
			m.data[2][2] = 1.0; // points[e2].z;
			m.data[3][2] = 0.0;
			m.data[0][3] = points[e2].x;
			m.data[1][3] = points[e2].y;
			m.data[2][3] = 0.0;
			m.data[3][3] = 1.0;
			return m;
		}
		inline matrix4x4 lcoords_inv(TriangleIndex triangle) const {
			return lcoords(triangle).get_inversed();
		}
		inline bool test(TriangleIndex triangle, const vector3& point, vector3& l) const {
			l = lcoords_inv(triangle) * point;
			l.z = 1.0 - l.x - l.y;
			return !(l.x < -FG_EPS || l.y < -FG_EPS || l.z < -FG_EPS);
		}
		inline vector3 cast(const vector3& point, std::tuple<size_t, size_t, size_t>& vertices) const {
			vector3 l;
			std::set<TriangleIndex> overlaps;
			triangle_lookup->get_overlap(rect{ point }, overlaps);
			for (auto i{ overlaps.begin() }; i != overlaps.end(); i++) {
				if (test(*i, point, l)) {
					vertices = triangleVertices(*i);
					return l;
				}
			}
		}
		inline GeometryType cast(const vector3& point, size_t& result) const {
			vector3 l;
			std::set<TriangleIndex> overlaps;
			triangle_lookup->get_overlap(rect{ point }, overlaps);
			for (auto i{ overlaps.begin() }; i != overlaps.end(); i++) {
				if (test(*i, point, l)) {
					size_t e0, e1, e2;
					std::tie(e0, e1, e2) = triangleVertices(*i);
					if (l.x < FG_EPS) {
						if (l.y < FG_EPS) {
							result = e2;
							return GeometryType::Vertex;
						}
						if (l.z < FG_EPS) {
							result = e1;
							return GeometryType::Vertex;
						}
						result = edgeIndex(*i, e1, e2);
						return GeometryType::Edge;
					}
					if (l.y < FG_EPS) {
						if (l.z < FG_EPS) {
							result = e0;
							return GeometryType::Vertex;
						}
						result = edgeIndex(*i, e2, e0);
						return GeometryType::Edge;
					}
					if (l.z < FG_EPS) {
						result = edgeIndex(*i, e0, e1);
						return GeometryType::Edge;
					}
					result = *i;
					return GeometryType::Triangle;
				}
			}
			result = 0xFFFFFFFF;
			return GeometryType::None;
		}
		inline EdgeIndex flip(EdgeIndex e) {
			auto tri = edge_triangles[e];
			if (tri.first == tri.second) return 0xFFFFFFFF;
			auto t0 = triangles[tri.first];
			auto t1 = triangles[tri.second];
			size_t p0, p1;
			size_t pn0, pn1;
			size_t fedge0, fedge1;
			std::tie(p0, p1) = edges[e];
			{
				auto triangle = t0;
				if (std::get<0>(triangle) == e) {
					auto edge_t = (edges[std::get<1>(triangle)]);
					if (edge_t.first == p0) {
						fedge0 = std::get<1>(triangle);
						pn0 = edge_t.second;
					}
					else if (edge_t.second == p0) {
						fedge0 = std::get<1>(triangle);
						pn0 = edge_t.first;
					}
					else {
						fedge0 = std::get<2>(triangle);
						pn0 = edge_t.first == p1 ? edge_t.second : edge_t.first;
					}
				}
				else {
					auto edge_t = (edges[std::get<0>(triangle)]);
					if (edge_t.first == p0) {
						fedge0 = std::get<0>(triangle);
						pn0 = edge_t.second;
					}
					else if (edge_t.second == p0) {
						fedge0 = std::get<0>(triangle);
						pn0 = edge_t.first;
					}
					else {
						fedge0 = std::get<1>(triangle) == e ? std::get<2>(triangle) : std::get<1>(triangle);
						pn0 = edge_t.first == p1 ? edge_t.second : edge_t.first;
					}
				}
			};
			{
				auto triangle = t1;
				if (std::get<0>(triangle) == e) {
					auto edge_t = (edges[std::get<1>(triangle)]);
					if (edge_t.first == p0) {
						fedge1 = std::get<1>(triangle);
						pn1 = edge_t.second;
					}
					else if (edge_t.second == p0) {
						fedge1 = std::get<1>(triangle);
						pn1 = edge_t.first;
					}
					else {
						fedge1 = std::get<2>(triangle);
						pn1 = edge_t.first == p1 ? edge_t.second : edge_t.first;
					}
				}
				else {
					auto edge_t = (edges[std::get<0>(triangle)]);
					if (edge_t.first == p0) {
						fedge1 = std::get<0>(triangle);
						pn1 = edge_t.second;
					}
					else if (edge_t.second == p0) {
						fedge1 = std::get<0>(triangle);
						pn1 = edge_t.first;
					}
					else {
						fedge1 = std::get<1>(triangle) == e ? std::get<2>(triangle) : std::get<1>(triangle);
						pn1 = edge_t.first == p1 ? edge_t.second : edge_t.first;
					}
				}
			};
			if (edge_triangles[fedge0].first == tri.first)
				edge_triangles[fedge0].first = tri.second;
			if (edge_triangles[fedge0].second == tri.first)
				edge_triangles[fedge0].second = tri.second;
			if (edge_triangles[fedge1].first == tri.second)
				edge_triangles[fedge1].first = tri.first;
			if (edge_triangles[fedge1].second == tri.second)
				edge_triangles[fedge1].second = tri.first;
			if (std::get<0>(triangles[tri.first]) == fedge0)
				std::get<0>(triangles[tri.first]) = fedge1;
			else if (std::get<1>(triangles[tri.first]) == fedge0)
				std::get<1>(triangles[tri.first]) = fedge1;
			else if (std::get<2>(triangles[tri.first]) == fedge0)
				std::get<2>(triangles[tri.first]) = fedge1;
			if (std::get<0>(triangles[tri.second]) == fedge1)
				std::get<0>(triangles[tri.second]) = fedge0;
			else if (std::get<1>(triangles[tri.second]) == fedge1)
				std::get<1>(triangles[tri.second]) = fedge0;
			else if (std::get<2>(triangles[tri.second]) == fedge1)
				std::get<2>(triangles[tri.second]) = fedge0;

			point_edges[p0].erase(e);
			point_edges[p1].erase(e);
			point_edges[pn0].insert(e);
			point_edges[pn1].insert(e);

			edges[e] = std::make_pair(pn0, pn1);

			fix_triangle_rect(t0, tri.first);
			fix_triangle_rect(t1, tri.second);
		}
		/*inline void insert_point(const vector3& point) {
			std::tuple<size_t, size_t, size_t> _;
			insert_point(point, _);
		}*/
		inline GeometryType insert_point(const vector3& point, std::tuple<size_t, size_t, size_t>& out, size_t& el) {
			size_t element;
			switch (cast(point, element)) {
			case GeometryType::None:
				el = 0xFFFFFFFF;
				return GeometryType::None;
			case GeometryType::Vertex:
				el = element;
				return GeometryType::Vertex;
			case GeometryType::Edge:
			{
				//points.push_back(point);
				auto p0 = points[edges[element].first];
				auto p1 = points[edges[element].second];
				double w = (point - p1).length() / (p1 - p0).length();
				out = subdivideEdge(element, w);
				el = element;
				return GeometryType::Edge;
			}
			case GeometryType::Triangle:
				auto np = points.size();
				points.push_back(point);

				auto nt0 = element;
				auto nt1 = triangles.size();
				auto nt2 = triangles.size() + 1;

				auto e0 = std::get<0>(triangles[nt0]);
				auto e1 = std::get<1>(triangles[nt0]);
				auto e2 = std::get<2>(triangles[nt0]);

				auto ne0 = edges.size();
				auto ne1 = edges.size() + 1;
				auto ne2 = edges.size() + 2;

				auto p0 = (edges[e0].first == edges[e1].first || edges[e0].second == edges[e1].first) ? edges[e1].second : edges[e1].first;
				auto p1 = (edges[e1].first == edges[e0].first || edges[e1].second == edges[e0].first) ? edges[e0].second : edges[e0].first;
				auto p2 = (edges[e2].first == edges[e1].first || edges[e2].second == edges[e1].first) ? edges[e1].second : edges[e1].first;

				if (edge_triangles[e1].first == nt0) edge_triangles[e1].first = nt1;
				if (edge_triangles[e1].second == nt0) edge_triangles[e1].second = nt1;
				if (edge_triangles[e2].first == nt0) edge_triangles[e2].first = nt2;
				if (edge_triangles[e2].second == nt0) edge_triangles[e2].second = nt2;

				edge_triangles.push_back(std::make_pair(nt1, nt2));
				edge_triangles.push_back(std::make_pair(nt0, nt2));
				edge_triangles.push_back(std::make_pair(nt0, nt1));

				edges.push_back(std::make_pair(p0, np));
				edges.push_back(std::make_pair(p1, np));
				edges.push_back(std::make_pair(p2, np));

				auto old_t = triangles[nt0];
				triangles[nt0] = std::make_tuple(e0, ne2, ne1);
				triangles.push_back(std::make_tuple(e1, ne0, ne2));
				triangles.push_back(std::make_tuple(e2, ne0, ne1));
				fix_triangle_rect(old_t, nt0);
				add_triangle_to_tree(nt1);
				add_triangle_to_tree(nt2);

				point_edges[p0].insert(ne0);
				point_edges[p1].insert(ne1);
				point_edges[p2].insert(ne2);
				point_edges.push_back({ ne0, ne1, ne2 });

				//add_edge_to_tree(ne0);
				//add_edge_to_tree(ne1);
				//add_edge_to_tree(ne2);

				out = std::make_tuple(ne0, ne1, ne2);
				el = element;
				return GeometryType::Triangle;
			}
			return GeometryType::None;
		}
		inline int TrianglesLength() const {
			return triangles.size();
		}
		inline Triangle triangle(size_t index) const {
			return triangles[index];
		}
		inline size_t edgesCount() const {
			return edges.size();
		}
		inline size_t pointsCount() const {
			return points.size();
		}
		inline Edge edge(EdgeIndex i) const {
			return edges[i];
		}
		inline std::pair<TriangleIndex, TriangleIndex> edge_triangle(EdgeIndex i) const {
			return edge_triangles[i];
		}
		inline const std::set<EdgeIndex>& point_edge(size_t point) const {
			return point_edges[point];
		}

		inline vector3 sample_edge(size_t edge, double w) const {
			return points[edges[edge].first] * (1.0 - w) + points[edges[edge].second] * (w);
		}
		inline double edge_length(size_t edge) const {
			return (points[edges[edge].first] - points[edges[edge].second]).length();
		}
		inline double edge_length_sq(size_t edge) const {
			return (points[edges[edge].first] - points[edges[edge].second]).lengthSq();
		}
		inline vector3 edge_tangent(size_t edge) const {
			return (points[edges[edge].first] - points[edges[edge].second]);
		}
		inline vector3 point(size_t index) const {
			return points[index];
		}
		inline vector3 getCoordsByPointIdx(size_t idx) const {
			return points[idx];
		}
		friend class MeshView2d;
		bool isLineInsideTriangle(const ILine& l, Mesh2::EdgeIndex le, Mesh2::TriangleIndex tri) {
			size_t thirdVertexIndex;
			auto edge = edges[le];
			if (std::get<0>(triangles[tri]) == le) {
				auto oe = edges[std::get<1>(triangles[tri])];
				thirdVertexIndex = (oe.first == edge.first || oe.first == edge.second) ? oe.second : oe.first;
			}
			else {
				auto oe = edges[std::get<0>(triangles[tri])];
				thirdVertexIndex = (oe.first == edge.first || oe.first == edge.second) ? oe.second : oe.first;
			}
			auto c = 0.5 * (points[edge.first] + points[edge.second]);
			return l.classify(c) == -l.classify(points[thirdVertexIndex]);
		}
		int intersect(const EllipticSegment& l, EdgeIndex e, std::vector<vector3>& res) {
			return Intersector<ILine>::intersect_segment(l, points[edges[e].first], points[edges[e].second], res);
		}
	protected:
		std::vector<vector3> points;
		std::vector<Edge> edges;
		std::vector<Triangle> triangles;

		std::vector<std::set<EdgeIndex>> point_edges;
		std::vector<std::pair<TriangleIndex, TriangleIndex>> edge_triangles;
		using TriangleLookup = lookup_tree<2, TreeTriangleIndex>;
		using EdgeLookup = lookup_tree<2, EdgeIndex>;

		std::unique_ptr<TriangleLookup> triangle_lookup;
		//std::unique_ptr<EdgeLookup> edge_lookup;
		//std::vector<TriangleIndex> tree_index;

		const Triangle EmptyTriangle = std::make_tuple(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
		inline void remove_triangle_from_tree(TriangleIndex tri) {
			rect r = rect{ points[edges[std::get<0>(triangles[tri])].first],
				points[edges[std::get<0>(triangles[tri])].second] }.
				add_point(points[edges[std::get<1>(triangles[tri])].first]).
				add_point(points[edges[std::get<1>(triangles[tri])].second]);
			auto tp = std::make_pair(r, tri);
			if (tri == triangles.size() - 1) {
				triangle_lookup->remove_element(tp);
				return;
			}
			triangle_lookup->replace_element(tp, triangles.size() - 1);
			rect rb = rect{ points[edges[std::get<0>(triangles.back())].first],
				points[edges[std::get<0>(triangles.back())].second] }.
				add_point(points[edges[std::get<1>(triangles.back())].first]).
				add_point(points[edges[std::get<1>(triangles.back())].second]);
			triangle_lookup->remove_element(std::make_pair(rb, triangles.size() - 1));
		}
		inline void add_triangle_to_tree(TriangleIndex tri) {
			rect r = rect{ points[edges[std::get<0>(triangles[tri])].first],
				points[edges[std::get<0>(triangles[tri])].second] }.
				add_point(points[edges[std::get<1>(triangles[tri])].first]).
				add_point(points[edges[std::get<1>(triangles[tri])].second]);
			triangle_lookup->add_element(std::make_pair(r, tri));
		}
		inline void fix_triangle_rect(Triangle old, TriangleIndex tri) {
			rect ro = rect{ points[edges[std::get<0>(old)].first],
				points[edges[std::get<0>(old)].second] }.
				add_point(points[edges[std::get<1>(old)].first]).
				add_point(points[edges[std::get<1>(old)].second]);
			rect r = rect{ points[edges[std::get<0>(triangles[tri])].first],
				points[edges[std::get<0>(triangles[tri])].second] }.
				add_point(points[edges[std::get<1>(triangles[tri])].first]).
				add_point(points[edges[std::get<1>(triangles[tri])].second]);
			triangle_lookup->replace_element_rect(std::make_pair(ro, tri), r);
		}
		//inline void remove_edge_from_tree(EdgeIndex e) {
		//	rect r = rect{ points[edges[e].first],
		//		points[edges[e].second] };
		//	auto tp = std::make_pair(r, e);
		//	if (e == edges.size() - 1) {
		//		edge_lookup->remove_element(tp);
		//		return;
		//	}
		//	edge_lookup->replace_element(tp, edges.size() - 1);
		//	rect rb = rect{ points[edges.back().first],
		//		points[edges.back().second] };
		//	edge_lookup->remove_element(std::make_pair(rb, triangles.size() - 1));
		//}
		//inline void add_edge_to_tree(EdgeIndex tri) {
		//	rect r = rect{ points[edges[tri].first],
		//		points[edges[tri].second] };
		//	edge_lookup->add_element(std::make_pair(r, tri));
		//}
		//inline void fix_edge_rect(rect ro, EdgeIndex tri) {
		//	/*rect ro = rect{ points[old.first],
		//		points[old.second] };*/
		//	rect r = rect{ points[edges[tri].first],
		//		points[edges[tri].second] };
		//	edge_lookup->replace_element_rect(std::make_pair(ro, tri), r);
		//}
	public:
		const TriangleLookup& lookup() const {
			return *triangle_lookup;
		}
	};

}