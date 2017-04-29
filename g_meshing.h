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
		inline RawMesh2() = default;
		inline RawMesh2(const RawMesh2&) = default;
		inline RawMesh2(RawMesh2&&) = default;
		inline RawMesh2& operator =(const RawMesh2&) = default;
		inline RawMesh2& operator =(RawMesh2&&) = default;
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

	// Внутренняя сетка. Состоит из вершин, ребер, треугольников. Треугольники состоят из ребер, а ребра из вершин.
	class FEMCADGEOMSHARED_EXPORT Mesh2
	{
	protected:
		void build_tree() {
			std::vector<std::pair<rect, TriangleIndex>> rects(triangles.size());
			rect brect{};
			for (size_t i{}; i < triangles.size(); i++) {
				//auto t = triangleVertices(i);
				rects[i] = std::make_pair(get_rect(triangles[i]), i);
				//std::make_pair(rect(points[std::get<0>(t)], points[std::get<1>(t)]).add_point(points[std::get<2>(t)]), i);
				brect.add_rect(rects[i].first);
			}
			// дерево для поиска тр-ков
			auto max_depth = 2;
			triangle_lookup = std::make_unique<TriangleLookup>(TriangleLookup(max_depth, 100, std::move(rects), brect.Min(), brect.Max(), max_depth));
		}
	public:
		//#ifdef _DEBUG
		size_t get_tree_debug_info() const {
			return triangle_lookup->counter;
		}
		//#endif
		using Edge = std::pair<size_t, size_t>;
		using Triangle = std::tuple<size_t, size_t, size_t>;
		using EdgeIndex = size_t;
		using TriangleIndex = size_t;
		using TreeTriangleIndex = size_t;
		static const size_t NotAnEdge = 0xFFFFFFFF;
#define NotCollapsed {{ Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge}}
		inline Mesh2() = default;
		// Внутренняя сетка. Состоит из вершин, ребер, треугольников. Треугольники состоят из ребер, а ребра из вершин.
		inline Mesh2(const RawMesh2& _mesh) : points{ _mesh.points } {
			std::map<Edge, std::vector<size_t>> all_edges;
			point_edges.resize(points.size());
			triangles.resize(_mesh.triangles.size(), EmptyTriangle);
			for (size_t i = 0; i < _mesh.triangles.size(); i++) {
				size_t p0, p1, p2;

				std::tie(p0, p1, p2) = _mesh.triangles[i];
				all_edges[std::make_pair(p0, p1)].push_back(i);
				all_edges[std::make_pair(p0, p2)].push_back(i);
				all_edges[std::make_pair(p1, p2)].push_back(i);
			}
			size_t p{};
			edges.resize(all_edges.size());
			edge_triangles.resize(all_edges.size());
			for (auto& i : all_edges) {
				edges[p] = i.first;
				auto t0 = i.second.front();
				auto t1 = i.second.back();
				edge_triangles[p] = std::make_pair(i.second.front(), i.second.back());
				point_edges[i.first.first].insert(p);
				point_edges[i.first.second].insert(p);

				auto tt = triangles[t0];
				if (std::get<0>(tt) == Mesh2::NotAnEdge) std::get<0>(tt) = p;
				else if (std::get<1>(tt) == Mesh2::NotAnEdge && std::get<0>(tt) != p) std::get<1>(tt) = p;
				else if (std::get<2>(tt) == Mesh2::NotAnEdge && std::get<0>(tt) != p && std::get<1>(tt) != p) std::get<2>(tt) = p;
				triangles[t0] = tt;
				tt = triangles[t1];
				if (std::get<0>(tt) == Mesh2::NotAnEdge) std::get<0>(tt) = p;
				else if (std::get<1>(tt) == Mesh2::NotAnEdge && std::get<0>(tt) != p) std::get<1>(tt) = p;
				else if (std::get<2>(tt) == Mesh2::NotAnEdge && std::get<0>(tt) != p && std::get<1>(tt) != p) std::get<2>(tt) = p;
				triangles[t1] = tt;

				//rects.push_back(std::make_pair(rect{ points[i.first.first], points[i.first.second] }, p));
				p++;
			}
			build_tree();
			//edge_lookup = std::make_unique<TriangleLookup>(TriangleLookup(16, std::move(rects), brect.Min(), brect.Max()));
		}
		inline Mesh2(const Mesh2& _mesh) :
			points(_mesh.points),
			edges(_mesh.edges),
			point_edges(_mesh.point_edges),
			triangles(_mesh.triangles),
			edge_triangles(_mesh.edge_triangles) {
			build_tree();
		}
		Mesh2& operator=(const Mesh2& _mesh) {
			points = (_mesh.points);
			edges = (_mesh.edges);
			point_edges = (_mesh.point_edges);
			triangles = (_mesh.triangles);
			edge_triangles = (_mesh.edge_triangles);
			build_tree();
		}
		Mesh2(Mesh2&&) = default;
		Mesh2& operator=(Mesh2&&) = default;
		inline std::pair<double, double> collapseWeight(EdgeIndex edge) const {
			auto t = edge_triangles[edge];
			static std::vector<size_t> segs;
			size_t vx, vo;
			auto tmin = 1.0;
			auto tmax = 1.0;
			auto* limit = &tmax;
			vx = edges[edge].first;
			vo = edges[edge].second;
			for (size_t k{}; k < 2; ++k) {
				segs.clear();
				for (auto i : point_edges[vx]) {
					segs.push_back(edges[i].first == vx ? edges[i].second : edges[i].first);
				}
				std::sort(segs.begin(), segs.end(), [&, vx](size_t l, size_t r) {
					auto sl = points[l] - points[vx], sr = points[r] - points[vx]; return std::atan2(sl.y, sl.x) < std::atan2(sr.y, sr.x); });
				/*auto es = triangles[t.first];
				if (std::get<0>(es) == edge) {
					e0 = std::get<1>(es);
					e1 = std::get<2>(es);
					if (edges[e1].first == edges[edge].first || edges[e1].second == edges[edge].first) {
						std::swap(e0, e1);
					}
					vx = edges[e0].first == edges[edge].first ? edges[e0].second : edges[e0].first;
				}
				else {
					e0 = std::get<0>(es);
					e1 = std::get<1>(es) == edge ? std::get<2>(es) : std::get<1>(es);
					if (edges[e1].first == edges[edge].first || edges[e1].second == edges[edge].first) {
						std::swap(e0, e1);
					}
					vx = edges[e0].first == edges[edge].first ? edges[e0].second : edges[e0].first;
				}
				t0 = edge_triangles[e0].first == t.first ? edge_triangles[e0].second : edge_triangles[e0].first;
				t1 = edge_triangles[e1].first == t.first ? edge_triangles[e1].second : edge_triangles[e1].first;
				if (std::get<0>(triangles[t0]) == e0) {
					et0 = std::get<1>(triangles[t0]);
					if (edges[et0].first != vx && edges[et0].second != vx)
						et0 = std::get<2>(triangles[t0]);
				}
				else {
					et0 = std::get<1>(triangles[t0]);
					if (et0 == e0)
						et0 = std::get<2>(triangles[t0]);
					if (edges[et0].first != vx && edges[et0].second != vx)
						et0 = std::get<0>(triangles[t0]);
				}
				if (std::get<0>(triangles[t1]) == e1) {
					et1 = std::get<1>(triangles[t1]);
					if (edges[et1].first != vx && edges[et1].second != vx)
						et1 = std::get<2>(triangles[t1]);
				}
				else {
					et1 = std::get<1>(triangles[t1]);
					if (et1 == e1)
						et1 = std::get<2>(triangles[t1]);
					if (edges[et1].first != vx && edges[et1].second != vx)
						et1 = std::get<0>(triangles[t1]);
				}*/
				auto gett = [&](const vector3& s0, const vector3& s1, const vector3& p0, const vector3& p1) {
					auto v = p1 - p0;
					auto t = s1 - s0;
					auto qp = p0 - s0;
					auto delta = -t.x * v.y + t.y * v.x;
					if (std::fabs(delta) < FG_EPS) return nan("");
					auto delta_t = -qp.x * v.y + qp.y * v.x;
					return delta_t / delta;
				};
				for (size_t i{}; i < segs.size(); ++i) {
					auto tt = gett(points[vx], points[vo], points[segs[i]], points[segs[(i + 1) % segs.size()]]);
					//std::cout << "\nt = " << tt << '\n';

					if (!std::isnan(tt) && tt > -FG_EPS)
						*limit = std::min(*limit, tt);
				}
				//tt = gett(edges[edge], edges[et0]);
				//if (!std::isnan(tt)) tmax = std::min(tmax, tt);
				/*if (t.first == t.second)
					break;
					t.first = t.second;*/
					//if (tmin >= 0.5 || tmax <= 0.5)
					//	return false;
				vx ^= vo ^= vx ^= vo;
				limit = &tmin;
			}
			tmin = 1.0 - tmin;
			if (tmin >= tmax - 1e-2) {
				return std::make_pair(nan(""), nan(""));
			}
			std::cout << "((((" << tmin << ' ' << tmax << "))))\n";
			return std::make_pair(tmin, tmax); //0.5*(tmin + tmax);
		}
		inline std::array<EdgeIndex, 3> collapseEdge(EdgeIndex edge, std::function<void(EdgeIndex, EdgeIndex)> onEdgeReplace, double weight = 0.5) {
			if (isnan(weight))
				return{ Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge };

			if (weight < 0.0 || weight > 1.0) {
				throw "BEDAAAAA";
			}

			//if (isCorrect() == false) {
			//	throw;
			//}
			auto e = edges[edge];
			auto tris = edge_triangles[edge];
			auto t0 = triangles[tris.first];
			auto t1 = triangles[tris.second];
			size_t et0 = std::get<0>(t0);
			size_t ot0;
			if (et0 == edge) {
				et0 = std::get<1>(t0);
				if (edges[et0].first == e.first || edges[et0].second == e.first) {
					ot0 = std::get<2>(t0);
				}
				else {
					ot0 = et0;
					et0 = std::get<2>(t0);
				}
			}
			else {
				if (edges[et0].first == e.first || edges[et0].second == e.first) {
					ot0 = std::get<2>(t0);
					ot0 = (ot0 == edge) ? std::get<1>(t0) : ot0;
				}
				else {
					ot0 = et0;
					et0 = std::get<2>(t0);
					et0 = (et0 == edge) ? std::get<1>(t0) : et0;
				}
			}
			size_t et1 = std::get<0>(t1);
			size_t ot1;
			if (et1 == edge) {
				et1 = std::get<1>(t1);
				if (edges[et1].first == e.first || edges[et1].second == e.first) {
					ot1 = std::get<2>(t1);
				}
				else {
					ot1 = et1;
					et1 = std::get<2>(t1);
				}
			}
			else {
				if (edges[et1].first == e.first || edges[et1].second == e.first) {
					ot1 = std::get<2>(t1);
					ot1 = (ot1 == edge) ? std::get<1>(t1) : ot1;
				}
				else {
					ot1 = et1;
					et1 = std::get<2>(t1);
					et1 = (et1 == edge) ? std::get<1>(t1) : et1;
				}
			}

			// Проверка угла
			if (edge_triangles[et0].first == edge_triangles[et0].second &&
				edge_triangles[ot0].first == edge_triangles[ot0].second &&
				edge_triangles[ot0].first == edge_triangles[et0].second ||
				edge_triangles[et1].first == edge_triangles[et1].second &&
				edge_triangles[ot1].first == edge_triangles[ot1].second &&
				edge_triangles[ot1].first == edge_triangles[et1].second)
				return{ Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge };

			auto pos = (weight * points[e.second] + (1.0 - weight) * points[e.first]);

			auto tt0i = edge_triangles[et0].first == tris.first ? edge_triangles[et0].second : edge_triangles[et0].first;
			auto ott0i = edge_triangles[ot0].first == tris.first ? edge_triangles[ot0].second : edge_triangles[ot0].first;
			auto t0v = edges[et0].first == e.first ? edges[et0].second : edges[et0].first;

			auto tt1i = edge_triangles[et1].first == tris.second ? edge_triangles[et1].second : edge_triangles[et1].first;
			auto ott1i = edge_triangles[ot1].first == tris.second ? edge_triangles[ot1].second : edge_triangles[ot1].first;
			auto t1v = edges[et1].first == e.first ? edges[et1].second : edges[et1].first;

			// Updating tree
			auto remove_tri_from_tree = [&](size_t tri, size_t last) {
				//auto last = triangles.size() - 1;
				remove_triangle_from_tree(tri);
				if (tri != last) {
					replace_triangle(last, tri);
				}
			};
			std::set<size_t> upd_tris;
			for (auto i : point_edges[e.second]) {
				if (edge_triangles[i].first != tris.first && edge_triangles[i].first != tris.second)
					upd_tris.insert(edge_triangles[i].first);
				if (edge_triangles[i].second != tris.first && edge_triangles[i].second != tris.second)
					upd_tris.insert(edge_triangles[i].second);
			}
			for (auto i : upd_tris) {
				shift_triangle_vertex(i, e.second, pos);
			}
			upd_tris.clear();
			for (auto i : point_edges[e.first]) {
				if (edge_triangles[i].first != tris.first && edge_triangles[i].first != tris.second)
					upd_tris.insert(edge_triangles[i].first);
				if (edge_triangles[i].second != tris.first && edge_triangles[i].second != tris.second)
					upd_tris.insert(edge_triangles[i].second);
			}
			for (auto i : upd_tris) {
				shift_triangle_vertex(i, e.first, pos);
			}

			remove_tri_from_tree(std::max(tris.first, tris.second), triangles.size() - 1);
			if (tris.first != tris.second)
				remove_tri_from_tree(std::min(tris.first, tris.second), triangles.size() - 2);



			// Moving point
			points[e.first] = pos;

			// Update edge points
			for (auto i : point_edges[e.second]) {
				if (i == edge || i == ot0 || i == ot1) continue;
				if (edges[i].first == e.second)
					edges[i].first = e.first;
				else
					edges[i].second = e.first;
			}

			// Updating point_edges
			point_edges[e.first].insert(point_edges[e.second].begin(), point_edges[e.second].end());

			point_edges[t0v].erase(ot0);
			point_edges[t1v].erase(ot1);
			point_edges[e.first].erase(ot0);
			point_edges[e.first].erase(ot1);
			point_edges[e.first].erase(edge);

			// Updating edge_triangles
			edge_triangles[et0] = std::make_pair(tt0i, ott0i);
			edge_triangles[et1] = std::make_pair(tt1i, ott1i);

			auto replace_edge = [&](size_t ot, size_t et, size_t tri) {
				if (std::get<0>(triangles[tri]) == ot)
					std::get<0>(triangles[tri]) = et;
				else if (std::get<1>(triangles[tri]) == ot)
					std::get<1>(triangles[tri]) = et;
				else if (std::get<2>(triangles[tri]) == ot)
					std::get<2>(triangles[tri]) = et;
				onEdgeReplace(ot, et);
			};
			replace_edge(ot0, et0, ott0i);
			replace_edge(ot1, et1, ott1i);

			auto replace_edge_tri = [&](size_t ot, size_t et, size_t edg) {
				if (edge_triangles[edg].first == ot) edge_triangles[edg].first = et;
				if (edge_triangles[edg].second == ot) edge_triangles[edg].second = et;
			};
			auto remove_tri = [&](size_t tri) {
				auto last = triangles.size() - 1;
				if (tri != last) {
					auto last_tri = triangles[last];
					//replace_triangle(triangles.size() - 1, tri);

					replace_edge_tri(last, tri, std::get<0>(last_tri));
					replace_edge_tri(last, tri, std::get<1>(last_tri));
					replace_edge_tri(last, tri, std::get<2>(last_tri));

					triangles[tri] = last_tri;
				}
				triangles.pop_back();
			};
			remove_tri(std::max(tris.first, tris.second));

			if (tris.first != tris.second) remove_tri(std::min(tris.first, tris.second));



			auto remove_edge = [&](size_t edg) {
				auto last = edges.size() - 1;
				if (edg != last) {
					point_edges[edges[last].first].erase(last);
					point_edges[edges[last].first].insert(edg);
					point_edges[edges[last].second].erase(last);
					point_edges[edges[last].second].insert(edg);

					replace_edge(last, edg, edge_triangles[last].first);
					replace_edge(last, edg, edge_triangles[last].second);

					edges[edg] = edges.back();
					edge_triangles[edg] = edge_triangles.back();
				}
				edges.pop_back();
				edge_triangles.pop_back();
			};

			std::array<size_t, 3> result = { NotAnEdge, NotAnEdge, NotAnEdge };
			if (ot0 > ot1) {
				if (edge > ot0) {
					remove_edge(edge);
					remove_edge(ot0);
					remove_edge(ot1);
					result[0] = edge;
					result[1] = ot0;
					result[2] = ot1;
				}
				else {
					if (edge > ot1) {
						remove_edge(ot0);
						remove_edge(edge);
						remove_edge(ot1);
						result[0] = ot0;
						result[1] = edge;
						result[2] = ot1;
					}
					else {
						remove_edge(ot0);
						remove_edge(ot1);
						remove_edge(edge);
						result[0] = ot0;
						result[1] = ot1;
						result[2] = edge;
					}
				}
			}
			else { // ot0 <= ot1
				if (edge < ot0) {
					remove_edge(ot1);
					result[0] = ot1;
					if (ot0 != ot1) {
						remove_edge(ot0);
						result[1] = ot0;
					}
					remove_edge(edge);
					result[2] = edge;
				}
				else { // edge > ot0
					if (edge < ot1) {
						remove_edge(ot1);
						result[0] = ot1;
						remove_edge(edge);
						result[1] = edge;
						if (ot0 != ot1) {
							remove_edge(ot0);
							result[2] = ot0;
						}
					}
					else {
						remove_edge(edge);
						result[0] = edge;
						remove_edge(ot1);
						result[1] = ot1;
						if (ot0 != ot1) {
							remove_edge(ot0);
							result[2] = ot0;
						}
					}
				}
			}

			auto last = points.size() - 1;
			if (e.second != last) {
				for (auto i : point_edges.back()) {
					if (edges[i].first == last)
						edges[i].first = e.second;
					else /*if (edges[i].second == last)*/
						edges[i].second = e.second;
				}
				points[e.second] = points[last];
				point_edges[e.second] = std::move(point_edges[last]);
			}
			points.pop_back();
			point_edges.pop_back();

			if (isCorrect() == false) {
				throw;
			}
			return result;
		}
		inline bool isCorrect() const {
			// проверяеся, что среди ребер, которым принадлежит точка в point_edges нет такого, которому она не принадлежит
			// т.е. проверяем соответствие массивов point_edges и edges
			bool f1 = true;
			bool f2 = true;
			bool f3 = true;
			bool f4 = true;
			for (auto i = 0U; i < point_edges.size(); ++i) {
				for (auto j : point_edges[i]) {
					if (edges[j].first != i && edges[j].second != i) {
						std::cout << "Invalid point_edges at " << i << std::endl;
						f1 = false;
						break;
					}
				}
				if (f1 == false)
					break;
			}
			// проверяеся, что среди треугольников, которым принадлежит ребро в edge_triangles нет такого, которому оно не принадлежит
			// т.е. проверяем соответствие массивов edge_triangles и triangles
			for (size_t i{}; i < triangles.size(); ++i) {
				auto tr = triangles[i];
				if (edge_triangles[std::get<0>(tr)].first != i && edge_triangles[std::get<0>(tr)].second != i ||
					edge_triangles[std::get<1>(tr)].first != i && edge_triangles[std::get<1>(tr)].second != i ||
					edge_triangles[std::get<2>(tr)].first != i && edge_triangles[std::get<2>(tr)].second != i) {
					std::cout << "Invalid edge_triangles at triangle " << i << std::endl;
					f2 = false;
					break;
				}
				if (f2 == false)
					break;
			}

			for (size_t i{}; i < edges.size(); ++i) {
				auto e = edge_triangles[i];
				auto t = triangles[e.first];
				if (std::get<0>(t) != i && std::get<1>(t) != i && std::get<2>(t) != i) {
					std::cout << "Invalid edge_triangles at edge" << i << std::endl;
					f4 = false;
					break;
				}
				t = triangles[e.second];
				if (std::get<0>(t) != i && std::get<1>(t) != i && std::get<2>(t) != i) {
					std::cout << "Invalid edge_triangles at edge" << i << std::endl;
					f4 = false;
					break;
				}
			}


			for (size_t i{}; i < triangles.size(); ++i) {
				auto tr = triangles[i];
				if (std::get<0>(tr) >= edges.size() || std::get<1>(tr) >= edges.size() || std::get<2>(tr) >= edges.size())
				{
					std::cout << "Invalid edges in triangles at " << i << std::endl;
					f3 = false;
					break;
				}
				if (f3 == false)
					break;
			}
			return f1 && f2 && f3 && f4;
		}
		//inline std::array<EdgeIndex, 3> collapseEdge2(EdgeIndex edge, double weight = 0.5) {
		//	if (isnan(weight))
		//		return{ Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge };
		//	auto e = edges[edge];
		//	auto tris = edge_triangles[edge];
		//	auto t0 = triangles[tris.first];
		//	auto t1 = triangles[tris.second];
		//	size_t et0 = std::get<0>(t0);
		//	size_t ot0;
		//	if (et0 == edge) {
		//		et0 = std::get<1>(t0);
		//		if (edges[et0].first == e.second || edges[et0].second == e.second) {
		//			ot0 = std::get<2>(t0);
		//		}
		//		else {
		//			ot0 = et0;
		//			et0 = std::get<2>(t0);
		//		}
		//	}
		//	else {
		//		if (edges[et0].first == e.second || edges[et0].second == e.second) {
		//			ot0 = std::get<2>(t0);
		//			ot0 = (ot0 == edge) ? std::get<1>(t0) : ot0;
		//		}
		//		else {
		//			ot0 = et0;
		//			et0 = std::get<2>(t0);
		//			et0 = (et0 == edge) ? std::get<1>(t0) : et0;
		//		}
		//	}
		//	/*size_t ot0 = std::get<0>(t0) == edge ? (std::get<1>(t0) == et0 ? std::get<2>(t0) : std::get<1>(t0)) :
		//		(std::get<0>(t0) == et0 ? (std::get<1>(t0) == edge ? std::get<2>(t0) : std::get<1>(t0)) :
		//		(std::get<1>(t0) == edge ? std::get<0>(t0) : std::get<1>(t0)));*/
		//	size_t et1 = std::get<0>(t1);
		//	size_t ot1;
		//	if (et1 == edge) {
		//		et1 = std::get<1>(t1);
		//		if (edges[et1].first == e.second || edges[et1].second == e.second) {
		//			ot1 = std::get<2>(t1);
		//		}
		//		else {
		//			ot1 = et1;
		//			et1 = std::get<2>(t1);
		//		}
		//	}
		//	else {
		//		if (edges[et1].first == e.second || edges[et1].second == e.second) {
		//			ot1 = std::get<2>(t1);
		//			ot1 = (ot1 == edge) ? std::get<1>(t1) : ot1;
		//		}
		//		else {
		//			ot1 = et1;
		//			et1 = std::get<2>(t1);
		//			et1 = (et1 == edge) ? std::get<1>(t1) : et1;
		//		}
		//	}
		//	//et1 = (et1 == edge) ? std::get<1>(t1) : et1;
		//	//et1 = edges[et1].first == e.second || edges[et1].second == e.second ? et1 : std::get<2>(t1);
		//	//size_t ot1 = std::get<0>(t1) == edge ? (std::get<1>(t1) == et1 ? std::get<2>(t1) : std::get<1>(t1)) :
		//	//	(std::get<0>(t1) == et1 ? (std::get<1>(t1) == edge ? std::get<2>(t1) : std::get<1>(t1)) :
		//	//	(std::get<1>(t1) == edge ? std::get<0>(t1) : std::get<1>(t1)));
		//	if (edge_triangles[et0].first == edge_triangles[et0].second &&
		//		edge_triangles[ot0].first == edge_triangles[ot0].second &&
		//		edge_triangles[ot0].first == edge_triangles[et0].second ||
		//		edge_triangles[et1].first == edge_triangles[et1].second &&
		//		edge_triangles[ot1].first == edge_triangles[ot1].second &&
		//		edge_triangles[ot1].first == edge_triangles[et1].second)
		//		return{ Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge };
		//	//remove_edge_from_tree(edge);
		//	//remove_edge_from_tree(et0);
		//	//if (et0 != et1) remove_edge_from_tree(et1);
		//	/*if (edge == 1754) {
		//		std::cout << "===" << et0 << ' ' << ot0 << ' ' << et1 << ' ' << ot1 << "===\n";
		//		std::cout << "===" << edges[et0].first << ' ' << edges[et0].second << ' ' << edges[et1].first << ' ' << edges[et1].second << "===";
		//	}*/
		//	point_edges[edges[et0].first].erase(et0);
		//	point_edges[edges[et0].second].erase(et0);
		//	point_edges[edges[et1].first].erase(et1);
		//	point_edges[edges[et1].second].erase(et1);
		//	auto tt0i = edge_triangles[et0].first == tris.first ? edge_triangles[et0].second : edge_triangles[et0].first;
		//	auto& tt0 = triangles[tt0i];
		//	auto old_t = tt0;
		//	auto ott0i = edge_triangles[ot0].first == tris.first ? edge_triangles[ot0].second : edge_triangles[ot0].first;
		//	rect ott0 = get_rect(triangles[ott0i]);
		//	rect ett0 = get_rect(old_t);
		//	if (std::get<0>(tt0) == et0) std::get<0>(tt0) = ot0;
		//	else if (std::get<1>(tt0) == et0) std::get<1>(tt0) = ot0;
		//	else if (std::get<2>(tt0) == et0) std::get<2>(tt0) = ot0;
		//	if (edge_triangles[ot0].first == tris.first) {
		//		edge_triangles[ot0].first = tt0i;
		//	}
		//	else {
		//		edge_triangles[ot0].second = tt0i;
		//	}
		//	auto tt1i = edge_triangles[et1].first == tris.second ? edge_triangles[et1].second : edge_triangles[et1].first;
		//	auto& tt1 = triangles[tt1i];
		//	old_t = tt1;
		//	auto ott1i = edge_triangles[ot1].first == tris.second ? edge_triangles[ot1].second : edge_triangles[ot1].first;
		//	rect ott1 = get_rect(triangles[ott1i]);
		//	rect ett1 = get_rect(old_t);
		//	if (std::get<0>(tt1) == et1) std::get<0>(tt1) = ot1;
		//	else if (std::get<1>(tt1) == et1) std::get<1>(tt1) = ot1;
		//	else if (std::get<2>(tt1) == et1) std::get<2>(tt1) = ot1;
		//	if (edge_triangles[ot1].first == tris.second) {
		//		edge_triangles[ot1].first = tt1i;
		//	}
		//	else {
		//		edge_triangles[ot1].second = tt1i;
		//	}
		//	points[e.first] = (weight * points[e.first] + (1.0 - weight) * points[e.second]);
		//	fix_triangle_rect(ott0, ott0i);
		//	fix_triangle_rect(ott1, ott1i);
		//	fix_triangle_rect(ett0, tt0i);
		//	fix_triangle_rect(ett1, tt1i);
		//	std::array<size_t, 3> be;
		//	size_t sz = triangles.size() - 1;
		//	remove_triangle_from_tree(tris.first);
		//	if (tris.first == sz) {
		//		triangles.pop_back(); sz--;
		//	}
		//	else {
		//		if (tris.second == sz) {
		//			triangles.pop_back(); sz--;
		//		}
		//		if (tris.first == sz) {
		//			triangles.pop_back(); sz--;
		//		}
		//		else {
		//			std::tie(be[0], be[1], be[2]) = triangles[tris.first] = triangles.back();
		//			for (size_t i{}; i < 3; i++) {
		//				if (edge_triangles[be[i]].first == sz) edge_triangles[be[i]].first = tris.first;
		//				if (edge_triangles[be[i]].second == sz) edge_triangles[be[i]].second = tris.first;
		//			}
		//			triangles.pop_back(); sz--;
		//		}
		//	}
		//	if (tris.first != tris.second) {
		//		remove_triangle_from_tree(tris.second);
		//	}
		//	if (tris.second == sz) {
		//		triangles.pop_back(); sz--;
		//	}
		//	else if (tris.first != tris.second && tris.second < sz) {
		//		std::tie(be[0], be[1], be[2]) = triangles[tris.second] = triangles.back();
		//		for (size_t i{}; i < 3; i++) {
		//			if (edge_triangles[be[i]].first == sz) edge_triangles[be[i]].first = tris.second;
		//			if (edge_triangles[be[i]].second == sz) edge_triangles[be[i]].second = tris.second;
		//		}
		//		triangles.pop_back();
		//	}
		//	for (auto i{ point_edges[e.second].begin() }; i != point_edges[e.second].end(); i++) {
		//		auto& p = edges[*i];
		//		if (p.first == e.second) { p.first = e.first; }
		//		else if (p.second == e.second) { p.second = e.first; }
		//	}
		//	point_edges[e.first].insert(std::begin(point_edges[e.second]), std::end(point_edges[e.second]));
		//	point_edges[e.first].erase(edge);
		//	//point_edges[e.first].erase(et0);
		//	//point_edges[e.first].erase(et1);
		//	// Old points
		//	//auto pfold = points[e.first];
		//	//auto psold = points[e.second];
		//	//points[e.first] = (weight * points[e.first] + (1.0 - weight) * points[e.second]);
		//	point_edges[e.second] = std::move(point_edges.back());
		//	point_edges.pop_back();
		//	sz = point_edges.size();
		//	if (e.second < sz) {
		//		for (auto i{ point_edges[e.second].begin() }; i != point_edges[e.second].end(); i++) {
		//			auto& p = edges[*i];
		//			if (p.first == sz) { p.first = e.second; }
		//			else if (p.second == sz) { p.second = e.second; }
		//		}
		//	}
		//	//for (auto i{ point_edges[e.first].begin() }; i != point_edges[e.first].end(); i++) {
		//	//	auto& p = edges[*i];
		//	//	if (p.first == e.first) {
		//	//		//fix_edge_rect(rect{ points[p.second], pfold }, *i);
		//	//	}
		//	//	else if (p.second == e.first) {
		//	//		//fix_edge_rect(rect{ points[p.first], pfold }, *i);
		//	//	}
		//	//}
		//	auto fix_point_edges_for_erased = [&](size_t edge_index, size_t old_index) {
		//		if (edge_index >= edges.size()) return;
		//		if (point_edges.size() > edges[edge_index].first) {
		//			point_edges[edges[edge_index].first].erase(old_index);
		//			point_edges[edges[edge_index].first].insert(edge_index);
		//		}
		//		if (point_edges.size() > edges[edge_index].second) {
		//			point_edges[edges[edge_index].second].erase(old_index);
		//			point_edges[edges[edge_index].second].insert(edge_index);
		//		}
		//	};
		//	std::swap(points[e.second], points.back());
		//	points.pop_back();
		//	sz = edges.size() - 1;
		//	be[0] = be[1] = be[2] = Mesh2::NotAnEdge;
		//	auto clean_up_last_triangle = [&](EdgeIndex e, size_t siz) {
		//		if (edge_triangles[siz].first >= triangles.size()) {
		//			return;
		//		}
		//		auto lt = triangles[edge_triangles[siz].first];
		//		if (std::get<0>(lt) == siz) std::get<0>(lt) = e;
		//		if (std::get<1>(lt) == siz) std::get<1>(lt) = e;
		//		if (std::get<2>(lt) == siz) std::get<2>(lt) = e;
		//		triangles[edge_triangles[siz].first] = lt;
		//		lt = triangles[edge_triangles[siz].second];
		//		if (std::get<0>(lt) == siz) std::get<0>(lt) = e;
		//		if (std::get<1>(lt) == siz) std::get<1>(lt) = e;
		//		if (std::get<2>(lt) == siz) std::get<2>(lt) = e;
		//		triangles[edge_triangles[siz].second] = lt;
		//	};
		//	// удаляем ребро, если оно последнее. Если не последнее, то меняем его с последним и удаляем
		//	auto del_edge = [&](size_t index) {
		//		if (index == sz) {
		//			edges.pop_back();
		//			edge_triangles.pop_back();
		//			sz--;
		//		}
		//		else {
		//			clean_up_last_triangle(index, sz);
		//			std::swap(edge_triangles[index], edge_triangles.back());
		//			edge_triangles.pop_back();
		//			std::swap(edges[index], edges.back());
		//			fix_point_edges_for_erased(index, sz);
		//			edges.pop_back();
		//			sz--;
		//			return index >= edges.size() ? NotAnEdge : index;
		//		}
		//		return NotAnEdge;
		//	};
		//	if (edge < et0) {
		//		if (et0 < et1) {
		//			// edge et0 et1
		//			be[2] = del_edge(et1);
		//			if (et1 != et0) be[1] = del_edge(et0);
		//			else be[1] = be[2];
		//			be[0] = del_edge(edge);
		//		}
		//		else {
		//			if (edge < et1) {
		//				// edge et1 et0
		//				be[1] = del_edge(et0);
		//				if (et1 != et0) be[2] = del_edge(et1);
		//				else be[2] = be[1];
		//				be[0] = del_edge(edge);
		//			}
		//			else {
		//				// et1 edge et0
		//				be[1] = del_edge(et0);
		//				be[0] = del_edge(edge);
		//				if (et1 != et0) be[2] = del_edge(et1);
		//				else be[2] = be[1];
		//			}
		//		}
		//	}
		//	else {
		//		if (et0 > et1) {
		//			// et1 et0 edge
		//			be[0] = del_edge(edge);
		//			be[1] = del_edge(et0);
		//			if (et1 != et0) be[2] = del_edge(et1);
		//			else be[2] = be[1];
		//		}
		//		else {
		//			if (edge < et1) {
		//				// et0 edge et1
		//				be[2] = del_edge(et1);
		//				be[0] = del_edge(edge);
		//				if (et1 != et0) be[1] = del_edge(et0);
		//				else be[1] = be[2];
		//			}
		//			else {
		//				// et0 et1 edge
		//				be[0] = del_edge(edge);
		//				be[2] = del_edge(et1);
		//				if (et1 != et0) be[1] = del_edge(et0);
		//				else be[1] = be[2];
		//			}
		//		}
		//	}
		//	/*
		//				if (edge == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
		//				else {
		//					clean_up_last_triangle(edge, sz);
		//					std::swap(edge_triangles[edge], edge_triangles.back());
		//					edge_triangles.pop_back();
		//					std::swap(edges[edge], edges.back());
		//					fix_point_edges_for_erased(edge, sz);
		//					edges.pop_back(); sz--; be[0] = edge >= edges.size() ? NotAnEdge : edge;
		//				}
		//				if (et0 == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
		//				else {
		//					clean_up_last_triangle(et0, sz);
		//					std::swap(edge_triangles[et0], edge_triangles.back());
		//					edge_triangles.pop_back();
		//					std::swap(edges[et0], edges.back());
		//					fix_point_edges_for_erased(et0, sz);
		//					edges.pop_back(); sz--; be[1] = et0 >= edges.size() ? NotAnEdge : et0;
		//				}
		//				if (et1 == sz) { edges.pop_back(); edge_triangles.pop_back(); sz--; }
		//				else if (et0 != et1) {
		//					clean_up_last_triangle(et1, sz);
		//					std::swap(edge_triangles[et1], edge_triangles.back());
		//					edge_triangles.pop_back();
		//					std::swap(edges[et1], edges.back());
		//					fix_point_edges_for_erased(et1, sz);
		//					edges.pop_back(); sz--; be[2] = et1 >= edges.size() ? NotAnEdge : et1;
		//				}
		//	*/
		//	return be;
		//}
		inline std::tuple<EdgeIndex, EdgeIndex, EdgeIndex> subdivideEdge(EdgeIndex edge, double weight = 0.5) {
			auto e = edges[edge];
			auto t0i = edge_triangles[edge].first;
			auto t1i = edge_triangles[edge].second;

			auto or0 = get_rect(triangles[t0i]);
			auto or1 = get_rect(triangles[t1i]);

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
			fix_triangle_rect(or0, t0i);

			triangles.push_back(std::make_tuple(eni, e0ni, et0i));
			add_triangle_to_tree(triangles.size() - 1);

			//add_edge_to_tree(eni);
			//add_edge_to_tree(e0ni);
			EdgeIndex e1ni = Mesh2::NotAnEdge;
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
				fix_triangle_rect(or1, t1i);

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

			if (edge_triangles[std::get<0>(tr)].first != t && edge_triangles[std::get<0>(tr)].second != t ||
				edge_triangles[std::get<1>(tr)].first != t && edge_triangles[std::get<1>(tr)].second != t ||
				edge_triangles[std::get<2>(tr)].first != t && edge_triangles[std::get<2>(tr)].second != t) {
				std::cout << v0 << ' ' << v1 << ' ' << v2 << " ______ " << t << '\n';
				throw;
			}

			return std::make_tuple(v0, v1, v2);
		}
		inline std::pair<size_t, size_t> oppositeVertices(size_t edge) const {
			std::pair<size_t, size_t> result;
			auto trs = edge_triangles[edge];
			auto tr = triangles[trs.first];
			auto et0 = std::get<0>(tr) == edge ? std::get<1>(tr) : std::get<0>(tr);
			result.first = edges[et0].first == edges[edge].first || edges[et0].first == edges[edge].second ? edges[et0].second : edges[et0].first;
			tr = triangles[trs.second];
			et0 = std::get<0>(tr) == edge ? std::get<1>(tr) : std::get<0>(tr);
			result.second = edges[et0].first == edges[edge].first || edges[et0].first == edges[edge].second ? edges[et0].second : edges[et0].first;

			return result;
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
			return Mesh2::NotAnEdge;
		}


		// l-координаты: http://dolivanov.ru/sites/default/files/_13.pdf ???
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

		inline matrix4x4 lcoords_inv(TriangleIndex triangle) const
		{
			return lcoords(triangle).get_inversed();
		}

		inline bool test(TriangleIndex triangle, const vector3& point, vector3& l) const
		{
			l = lcoords_inv(triangle) * point;
			l.z = 1.0 - l.x - l.y;
			return !(l.x < -FG_EPS || l.y < -FG_EPS || l.z < -FG_EPS);
		}

		// находит первый треугольник, на который попадает точка point
		inline vector3 cast(const vector3& point, std::tuple<size_t, size_t, size_t>& vertices) const
		{
			vector3 l;
			static std::vector<TriangleIndex> overlaps;
			overlaps.resize(0);
			// находим все треугольники, в ограничивающий объект которых попала точка
			triangle_lookup->get_overlap(point, overlaps);
			// из всех найденных трегольников выбираем только тот треугольник, в который попала точка
			for (auto i{ overlaps.begin() }; i != overlaps.end(); i++)
				if (test(*i, point, l))
				{
					vertices = triangleVertices(*i);
					return l;
				}
			return std::numeric_limits<double>::infinity();
		}

		// определяет, в какой элемент геометрии попала точка
		inline GeometryType cast(const vector3& point, size_t& result) const {
			vector3 l;
			static std::vector<TriangleIndex> overlaps;
			overlaps.resize(0);
			triangle_lookup->get_overlap(point, overlaps);
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
			result = Mesh2::NotAnEdge;
			return GeometryType::None;
		}
		inline EdgeIndex flip(EdgeIndex e) {
			auto tri = edge_triangles[e];
			if (tri.first == tri.second) return Mesh2::NotAnEdge;
			auto t0 = triangles[tri.first];
			auto t1 = triangles[tri.second];

			auto or0 = get_rect(t0);
			auto or1 = get_rect(t1);

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
					if (edge_t.first == p1) {
						fedge1 = std::get<1>(triangle);
						pn1 = edge_t.second;
					}
					else if (edge_t.second == p1) {
						fedge1 = std::get<1>(triangle);
						pn1 = edge_t.first;
					}
					else {
						fedge1 = std::get<2>(triangle);
						pn1 = edge_t.first == p0 ? edge_t.second : edge_t.first;
					}
				}
				else {
					auto edge_t = (edges[std::get<0>(triangle)]);
					if (edge_t.first == p1) {
						fedge1 = std::get<0>(triangle);
						pn1 = edge_t.second;
					}
					else if (edge_t.second == p1) {
						fedge1 = std::get<0>(triangle);
						pn1 = edge_t.first;
					}
					else {
						fedge1 = std::get<1>(triangle) == e ? std::get<2>(triangle) : std::get<1>(triangle);
						pn1 = edge_t.first == p0 ? edge_t.second : edge_t.first;
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

			fix_triangle_rect(or0, tri.first);
			fix_triangle_rect(or1, tri.second);

			return e;
		}

		inline bool flippable(EdgeIndex e, double r = 1.0) const {
			auto pp0 = points[edges[e].first];
			auto pp1 = points[edges[e].second];

			auto t = edge_triangles[e];
			if (t.first == t.second) return false;
			size_t i0, i1;
			auto ed = std::get<0>(triangles[t.first]);
			if (ed == e) {
				ed = std::get<1>(triangles[t.first]);
			}
			if (edges[ed].first == edges[e].first || edges[ed].first == edges[e].second)
				i0 = edges[ed].second;
			else
				i0 = edges[ed].first;
			ed = std::get<0>(triangles[t.second]);
			if (ed == e) {
				ed = std::get<1>(triangles[t.second]);
			}
			if (edges[ed].first == edges[e].first || edges[ed].first == edges[e].second)
				i1 = edges[ed].second;
			else
				i1 = edges[ed].first;

			auto pi0 = points[i0];
			auto pi1 = points[i1];

			plane p = plane::byThreePonts(pi0, pi1, pi0 + vector3::Z());


			if (p.classify(pp0) == -p.classify(pp1)) {
				if (p.classify(pp0) == 0) {
					return false;
				}
				auto a = pi0 - pp0;
				auto b = pp1 - pi1;

				auto d0 = (pp0 - pp1);
				auto d1 = (pi0 - pi1);

				double el0 = (d0 & a);
				el0 = el0 * el0 / a.lengthSq() / d0.lengthSq();

				double el1 = (d1 & b);
				el1 = el1 * el1 / b.lengthSq() / d1.lengthSq();
				return el0 > el1 * r * r;
			}
			return false;
		}

		/*inline void insert_point(const vector3& point) {
			std::tuple<size_t, size_t, size_t> _;
			insert_point(point, _);
		}*/
		inline GeometryType insert_point(const vector3& point, std::tuple<size_t, size_t, size_t>& out, size_t& el) {
			size_t element;
			switch (cast(point, element)) {
			case GeometryType::None:
				el = Mesh2::NotAnEdge;
				return GeometryType::None;
			case GeometryType::Vertex:
				el = element;
				return GeometryType::Vertex;
				// если точка попала в ребро, то подразбиваем его на два
			case GeometryType::Edge:
			{
				//for (auto elem : this->lookup().container)
				//{
				//	if (elem.first != this->get_rect(this->triangles[elem.second]))
				//	{
				//		std::cout << "Bad rect";
				//		throw;
				//	}
				//}


				//points.push_back(point);
				auto p0 = points[edges[element].first];
				auto p1 = points[edges[element].second];
				double w = (point - p1).length() / (p1 - p0).length();
				out = subdivideEdge(element, w);
				el = element;
				return GeometryType::Edge;
			}
			// если точка попала в треугольник, то он разбивается на три треугольника
			case GeometryType::Triangle:
				auto np = points.size();
				points.push_back(point);

				auto nt0 = element;

				auto or = get_rect(triangles[nt0]);

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
				fix_triangle_rect(or , nt0);
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
		// возвращает хэндлеры ребер тр-ка
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
		// возвращает ребра, принадлежащие точке
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
		bool isLineInsideTriangle(const ILine& l, Mesh2::EdgeIndex le, Mesh2::TriangleIndex tri)
		{
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

		inline std::array<EdgeIndex, 4> get_quadrangle_edges(EdgeIndex e, std::pair<TriangleIndex, TriangleIndex>& t) {
			t = edge_triangles[e];
			if (t.first == t.second) throw;
			std::array<EdgeIndex, 4> res;
			size_t i = 0;
			auto addif = [&i, e, &res](size_t ind) {if (ind != e && ind != res[0] && ind != res[1] && ind != res[2] && ind != res[3]) res[i++] = ind; };
			auto tri = triangles[t.first];
			addif(std::get<0>(tri));
			addif(std::get<1>(tri));
			addif(std::get<2>(tri));

			tri = triangles[t.second];
			addif(std::get<0>(tri));
			addif(std::get<1>(tri));
			addif(std::get<2>(tri));

			return res;
		}
		inline std::array<EdgeIndex, 4> get_quadrangle_edges(EdgeIndex e) {
			std::pair<TriangleIndex, TriangleIndex> _fake;
			return get_quadrangle_edges(e, _fake);
		}
#ifndef _DEBUG_
	protected:
#endif
		std::vector<vector3> points;
		std::vector<Edge> edges;
		std::vector<Triangle> triangles;

		// все ребра, которым принадлежит точка
		std::vector<std::set<EdgeIndex>> point_edges;
		// все треугольники, которым принадлежит ребро (их может быть всего 2)
		std::vector<std::pair<TriangleIndex, TriangleIndex>> edge_triangles;
		using TriangleLookup = lookup_tree<2, TreeTriangleIndex>;
		//using EdgeLookup = lookup_tree<2, EdgeIndex>;

		std::unique_ptr<TriangleLookup> triangle_lookup;
		//std::unique_ptr<EdgeLookup> edge_lookup;
		//std::vector<TriangleIndex> tree_index;

		const Triangle EmptyTriangle = std::make_tuple(Mesh2::NotAnEdge, Mesh2::NotAnEdge, Mesh2::NotAnEdge);
		inline void remove_triangle_from_tree(TriangleIndex tri) {
			rect r = get_rect(triangles[tri]);
			//rect r = rect{ points[edges[std::get<0>(triangles[tri])].first],
			//	points[edges[std::get<0>(triangles[tri])].second] }.
			//	add_point(points[edges[std::get<1>(triangles[tri])].first]).
			//	add_point(points[edges[std::get<1>(triangles[tri])].second]);
			auto tp = std::make_pair(r, tri);
			//if (tri == triangles.size() - 1) {
			triangle_lookup->remove_element(tp);
			//return;
		//}
		//triangle_lookup->replace_element(tp, triangles.size() - 1);
			//rect rb = get_rect(triangles.back());
			//rect rb = rect{ points[edges[std::get<0>(triangles.back())].first],
			//	points[edges[std::get<0>(triangles.back())].second] }.
			//	add_point(points[edges[std::get<1>(triangles.back())].first]).
			//	add_point(points[edges[std::get<1>(triangles.back())].second]);
			//triangle_lookup->remove_element(std::make_pair(rb, triangles.size() - 1));
			//triangle_lookup->add_element(std::make_pair(rb, tri));
		}
		inline void add_triangle_to_tree(TriangleIndex tri) {
			rect r = get_rect(triangles[tri]);
			triangle_lookup->add_element(std::make_pair(r, tri));
		}
		inline void replace_triangle(TriangleIndex old, TriangleIndex nw) {
			rect r = get_rect(triangles[old]);
			triangle_lookup->replace_element(std::make_pair(r, old), nw);
		}
		//inline void fix_triangle_rect(const rect& ro, TriangleIndex tri) {
		//	rect r = get_rect(triangles[tri]);
		//	triangle_lookup->replace_element_rect(std::make_pair(ro, tri), r);
		//}
		inline void shift_triangle_vertex(TriangleIndex tri, size_t vert_index, const vector3& pos) {
			auto t = triangles[tri];
			rect r = get_rect(t);
			rect nr{ edges[std::get<0>(t)].first != vert_index ? points[edges[std::get<0>(t)].first] : pos,
				edges[std::get<0>(t)].second != vert_index ? points[edges[std::get<0>(t)].second] : pos };
			nr.add_point(edges[std::get<1>(t)].first != vert_index ? points[edges[std::get<1>(t)].first] : pos).
				add_point(edges[std::get<1>(t)].second != vert_index ? points[edges[std::get<1>(t)].second] : pos);

			triangle_lookup->replace_element_rect(std::make_pair(r, tri), nr);
		}
		inline void fix_triangle_rect(rect ro, TriangleIndex tri) {
			/*rect ro = get_rect(old);*/
			rect r = get_rect(triangles[tri]);
			//rect r = rect{ points[edges[std::get<0>(triangles[tri])].first],
			//	points[edges[std::get<0>(triangles[tri])].second] }.
			//	add_point(points[edges[std::get<1>(triangles[tri])].first]).
			//	add_point(points[edges[std::get<1>(triangles[tri])].second]);
			triangle_lookup->replace_element_rect(std::make_pair(ro, tri), r);
		}
		inline rect get_rect(Triangle t) const {
			return rect{ points[edges[std::get<0>(t)].first],
						 points[edges[std::get<0>(t)].second] }.
				add_point(points[edges[std::get<1>(t)].first]).
				add_point(points[edges[std::get<1>(t)].second]);
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
		const std::vector<std::set<EdgeIndex>>& PointEdges() const {
			return point_edges;
		}

		template<int D, class T, int plane = axis::AXIS_X>
		void printLookupTree(std::ofstream &f, const lookup_tree<D, T, plane> *tree, size_t &index, size_t parent = 0)
		{
			f << "v = " << *tree;
			//if (tree == nullptr) {
			//	f << "Node: NULL - " << parent << std::endl;
			//	f << "	Plane: " << plane << std::endl;
			//	return;
			//}
			//// индекс текущего узла
			//index++;
			//size_t old_idx = index;
			//f << "Node: " << old_idx << " - " << parent << std::endl;
			//f << "	Plane: " << plane << std::endl;
			//f << "	Minimum: " << tree->minimum << std::endl;
			//f << "	Maximum : " << tree->maximum << std::endl;
			//f << "	Mx: " << tree->Mx << ", counter: " << tree->counter << std::endl;
			//f << "	Collection: " << std::endl;
			//for (auto elem : tree->container)
			//	f << "	rect.Min: " << elem.first.Min() << ", rect.Max: " << elem.first.Max() << ", second: " << elem.second << std::endl;
			////std::array<lookup_tree<D, T, (plane + 1) % D>*, 2> s;
			//printLookupTree(f, tree->s[0], index, old_idx);
			//printLookupTree(f, tree->s[1], index, old_idx);
		}
	};

}