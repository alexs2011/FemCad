#pragma once
#include "g_meshing.h"
#include "g_line_ext.h"

namespace fg {

	class FEMCADGEOMSHARED_EXPORT MeshView2d {
		struct _meshLineView {
			const ILine& line;
			std::map<Mesh2::EdgeIndex, double> pos;
			std::map<double, Mesh2::EdgeIndex> edges;
			_meshLineView(const ILine& l) : line{ l } {}
			Mesh2::EdgeIndex get(double t) const {
				t += FG_EPS;
				auto s = edges.lower_bound(t);
				s--;
				return s->second;
			}
			double next(double t) {
				auto s = edges.upper_bound(t);
				return s == edges.end() ? 1.0 : s->first;
			}
			void force_add(double t, Mesh2::EdgeIndex e) {
				edges[t] = e;
				pos[e] = t;
			}
			void add(double t, Mesh2::EdgeIndex e0, Mesh2::EdgeIndex e1) {
				if (edges.empty()) {
					edges[0.0] = e0;
					pos[e0] = 0.0;
					edges[t] = e1;
					pos[e1] = t;
				}
				else {
					auto e = edges.lower_bound(t);
					auto w = pos[e->second];
					pos.erase(e->second);
					e->second = e0;
					pos[e0] = e->first;
					edges[t] = e1;
					pos[e1] = t;
				}
			}
			void replace(double t, Mesh2::EdgeIndex e, Mesh2::EdgeIndex e0, Mesh2::EdgeIndex e1) {
				assert(t < 1.0);
				double w = pos[e];
				pos.erase(e);
				pos[e0] = w;
				pos[e1] = t;
				edges[w] = e0;
				edges[t] = e1;
			}
		};
		Mesh2 mesh;
		// коллекция линий геометрии
		std::vector<_meshLineView> geometry;
		// для каждого ребра сетки список индексов линий геометрии, соответствующих ему
		std::vector<std::vector<size_t>> _edgeGeometry;
		const _meshLineView& get_edge_geometry(size_t edge, size_t p) const {
			return geometry[_edgeGeometry[edge][p]];
		}
		void edge_geometry_replace(size_t geom, size_t e, size_t e0, size_t e1) {
			for (size_t i{}; i < _edgeGeometry[e].size(); i++) {
				if (geom == _edgeGeometry[e][i]) {
					_edgeGeometry[e0].push_back(geom);
					_edgeGeometry[e1].push_back(geom);
					std::swap(_edgeGeometry[e][i], _edgeGeometry[e].back());
					_edgeGeometry[e].pop_back();
					break;
				}
			}
		}

	public:
		inline bool isBoundary(Mesh2::EdgeIndex i) const {
			//if (_edgeGeometry.size() <= i) return false;
			return _edgeGeometry[i].size() > 0;
			//return std::any_of(geometry.begin(), geometry.end(), [=](_meshLineView x)->bool {return x.pos.count(i) > 0; });
		}
		const Mesh2& Mesh() const { return mesh; }

		MeshView2d(const RectView& r) : mesh(r.mesh()) 
		{
			std::map<Mesh2::Edge, Mesh2::EdgeIndex> edges;
			_edgeGeometry.resize(mesh.edgesCount());
			for (size_t i{}; i < mesh.edges.size(); i++) {
				auto e = mesh.edges[i];
				edges[e] = i;
				edges[std::make_pair(e.second, e.first)] = i;
			}
			auto p = 0U;
			auto pnext = 0U;
			for (size_t i{}; i < r.lines.size(); i++) {
				for (size_t j{}; j < r.lines[i].lines.size(); j++) {
					geometry.emplace_back(_meshLineView(r.lines[i].lines[j].line));
					auto& ps = r.lines[i].lines[j].points;
					vector3 sample_point = geometry.back().line.sample(0.0);
					auto res = mesh.cast(sample_point, p);
					if (res != GeometryType::Vertex) throw FGException("Incorrect built mesh!");
					for (size_t k{ 0 }; k <= ps.size(); k++) {
						sample_point = geometry.back().line.sample(k == ps.size() ? 1.0 : ps[k]);
						res = mesh.cast(sample_point, pnext);
						if (res != GeometryType::Vertex) throw FGException("Incorrect built mesh!");
						auto e = edges[std::make_pair(p, pnext)];
						_edgeGeometry[e].push_back(geometry.size() - 1);
						geometry.back().force_add(k == 0 ? 0.0 : ps[k - 1], e);

						p = pnext;
					}
				}
			}
		}
		void AddPoint(const vector3& pos, std::vector<Mesh2::EdgeIndex>* added_edges = nullptr) {
			std::tuple<size_t, size_t, size_t> out;
			size_t el;
			auto res = mesh.cast(pos, el);
			//auto res = mesh.insert_point(pos, out, el);

			static std::vector<vector3> int_points;
			static std::vector<size_t> int_index;
			static std::vector<Mesh2::EdgeIndex> new_edges;
			static std::vector<size_t> geoms;


			auto process_edges = [&]() {
				while (!new_edges.empty()) {
					for (auto i : geoms) {
						for (size_t j{}; j < new_edges.size(); j++) {
							if (new_edges[j] == 0xFFFFFFFF) continue;
#ifdef _DEBUG
							std::cout << new_edges[j] << ", ";
#endif
							auto& m = mesh;
							auto count = mesh.intersect(static_cast<const EllipticSegment&>(geometry[i].line), new_edges[j], int_points);
							for (size_t k{}; k < count; k++) int_index.push_back(i);
						}
					}
					new_edges.resize(0);
					res = GeometryType::Vertex;
					while (!int_points.empty()) {
						res = mesh.insert_point(int_points.back(), out, el);

						if (res == GeometryType::Vertex) {
							// Точка int_points.back() лежит на дуге int_index.back()
							int_points.pop_back();
							int_index.pop_back();
							continue;
						}
						assert(res == GeometryType::Edge);
						auto e0 = std::get<1>(out);
						auto e1 = std::get<2>(out);
						//if (std::get<0>(out) != 0xFFFFFFFF) 
						if (std::get<0>(out) >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						if (e0 != 0xFFFFFFFF && e0 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						if (e1 != 0xFFFFFFFF && e1 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						// Дуга с индексом int_index.back() пересеклась с ребром el в точке int_points.back()

						auto& line = geometry[int_index.back()];
						double t = line.line.getParam(int_points.back());
						auto e = line.get(t);
						auto tt = line.pos[e];
						size_t indx;
						mesh.cast(line.line.sample(tt), indx);

						if (mesh.edge(e0).first == indx || mesh.edge(e0).second == indx)
							geometry[int_index.back()].replace(t, e, e0, e1);
						else
							geometry[int_index.back()].replace(t, e, e1, e0);

						edge_geometry_replace(int_index.back(), e, e0, e1);
						
						new_edges.push_back(e0);
						new_edges.push_back(e1);

						if (added_edges) {
							added_edges->push_back(std::get<0>(out));
							added_edges->push_back(std::get<1>(out));
							added_edges->push_back(std::get<2>(out));
						}
						break;
					}
				}

			};
			auto add = [&](size_t edge, size_t tri) {
				//if (edge == 0xFFFFFFFF) return;
				for (size_t i{}; i < _edgeGeometry[edge].size(); ++i) {
					try {
						const EllipticSegment& es = dynamic_cast<const EllipticSegment&>(get_edge_geometry(edge, i).line);
						if (mesh.isLineInsideTriangle(es, edge, tri))
							geoms.push_back(_edgeGeometry[edge][i]);
					}
					catch (...) {}
				}
			};
			if (res == GeometryType::Triangle) {
				int_points.resize(0);
				int_index.resize(0);
				new_edges.resize(0);
				geoms.resize(0);
				auto edges = mesh.triangle(el);
				auto res = mesh.insert_point(pos, out, el);

				auto eo = std::get<0>(out);
				auto e0 = std::get<1>(out);
				auto e1 = std::get<2>(out);

				add(std::get<0>(edges), el);
				add(std::get<1>(edges), el);
				add(std::get<2>(edges), el);
				
				if (eo >= _edgeGeometry.size()) _edgeGeometry.push_back({});
				if (e0 >= _edgeGeometry.size() && e0 != 0xFFFFFFFF) _edgeGeometry.push_back({});
				if (e1 >= _edgeGeometry.size() && e1 != 0xFFFFFFFF) _edgeGeometry.push_back({});


				new_edges.push_back(std::get<0>(out));
				new_edges.push_back(std::get<1>(out));
				new_edges.push_back(std::get<2>(out));

				if (added_edges) {
					added_edges->push_back(std::get<0>(out));
					added_edges->push_back(std::get<1>(out));
					added_edges->push_back(std::get<2>(out));
				}

				process_edges();
			}
			if (res == GeometryType::Edge) {
				auto et = mesh.edge_triangle(el);
				auto tri0 = et.first;
				auto tri1 = et.second;
				auto edges = mesh.triangle(tri0);

				int_points.resize(0);
				int_index.resize(0);
				new_edges.resize(0);
				geoms.resize(0);
				add(std::get<0>(edges), tri0);
				add(std::get<1>(edges), tri0);
				add(std::get<2>(edges), tri0);

				auto res = mesh.insert_point(pos, out, el);
				if (std::get<0>(out) >= _edgeGeometry.size()) _edgeGeometry.push_back({});
				if (std::get<1>(out) >= _edgeGeometry.size() && std::get<1>(out) != 0xFFFFFFFF) _edgeGeometry.push_back({});
				if (std::get<2>(out) >= _edgeGeometry.size() && std::get<2>(out) != 0xFFFFFFFF) _edgeGeometry.push_back({});

				new_edges.push_back(std::get<1>(out));
				if (added_edges) {
					added_edges->push_back(std::get<1>(out));
				}
				process_edges();

				if (tri0 != tri1) {
					int_points.resize(0);
					int_index.resize(0);
					new_edges.resize(0);
					geoms.resize(0);
					add(std::get<0>(edges), tri1);
					add(std::get<1>(edges), tri1);
					add(std::get<2>(edges), tri1);

					auto res = mesh.insert_point(pos, out, el);
					if (std::get<0>(out) >= _edgeGeometry.size()) _edgeGeometry.push_back({});
					if (std::get<1>(out) >= _edgeGeometry.size() && std::get<1>(out) != 0xFFFFFFFF) _edgeGeometry.push_back({});
					if (std::get<2>(out) >= _edgeGeometry.size() && std::get<2>(out) != 0xFFFFFFFF) _edgeGeometry.push_back({});

					new_edges.push_back(std::get<2>(out));
					if (added_edges) {
						added_edges->push_back(std::get<2>(out));
					}
					process_edges();
				}
			}
		}

		void AddLine(const MeshedLine& line) {
			static std::vector<vector3> int0;
			static std::vector<vector3> int1;
			static std::vector<vector3> int_common;
			static std::set<Mesh2::EdgeIndex> triangles_set;
			static std::set<Mesh2::EdgeIndex> edges_set;
			static std::vector<Mesh2::EdgeIndex> edges;
			static std::vector<size_t> geoms;
			int0.resize(0);
			int1.resize(0);
			int_common.resize(0);
			edges_set.clear();
			triangles_set.clear();
			geoms.resize(0);
			for (size_t i{}; i < geometry.size(); i++) {
				Intersector<ILine>::intersect_dynamic(int0, int1, line.line, geometry[i].line);
				for (size_t j{}; j < int0.size(); j++) {
					for (size_t k{ j }; k < int1.size(); k++) {
						if (int0[j] == int1[k]) {
							int_common.push_back(int0[j]);
							geoms.push_back(i);
						}
					}
				}
			}
			AddPoint(line.p0());
			AddPoint(line.p1());
			
			for (size_t i{}; i < int_common.size(); i++) {
				edges.resize(0);
				AddPoint(int_common[i], &edges);
				auto& line = geometry[geoms[i]];
				double t = line.line.getParam(int_common[i]);
				auto e = line.get(t);
				auto tt = line.pos[e];
				if (std::fabs(tt - t) > FG_EPS) {
					size_t edge_beg;
					size_t edge_end;
					mesh.cast(line.line.sample(tt), edge_beg);
					edge_end = mesh.edge(e).first;
					if (edge_end == edge_beg) edge_end = mesh.edge(e).second;
					size_t e0, e1;
					
					for (size_t j{}; j < edges.size(); j++) {
						auto edge_cur = mesh.edge(edges[j]);
						if (edge_cur.first == edge_beg || edge_cur.second == edge_beg) e0 = edges[j];
						if (edge_cur.first == edge_end || edge_cur.second == edge_end) e1 = edges[j];
					}

					geometry[geoms[i]].replace(t, e, e0, e1);
					
					edge_geometry_replace(geoms[i], e, e0, e1);
				}
			}
			edges.resize(0);
			static std::vector<rect> boxes;
			boxes.resize(0);
			if (line.points.size()) {
				boxes.push_back(line.line.getBoundingRect(0, line.points[0]));
				for (size_t i{ 1 }; i < line.points.size(); i++) {
					boxes.push_back(line.line.getBoundingRect(line.points[i - 1], line.points[i]));
				}
				boxes.push_back(line.line.getBoundingRect(1.0, line.points.back()));
			}
			else {
				boxes.push_back(line.line.getBoundingRect());
			}
			for (size_t i{}; i < boxes.size(); ++i) {
				mesh.lookup().get_overlap(boxes[i], triangles_set);
			}
			for (auto i : triangles_set) {
				edges_set.insert(std::get<0>(mesh.triangle(i)));
				edges_set.insert(std::get<1>(mesh.triangle(i)));
				edges_set.insert(std::get<2>(mesh.triangle(i)));
			}
			auto mline = _meshLineView(line.line);
			edges.insert(edges.end(), edges_set.begin(), edges_set.end());
			for (size_t i{}; i < edges.size(); ++i) {
				int_common.resize(0);
				auto e = mesh.edge(edges[i]);
				auto v0 = mesh.point(e.first);
				auto v1 = mesh.point(e.second);
				auto res = Intersector<ILine>::intersect_segment(line.line, v0, v1, int_common);
				switch (res) {
				case 0:
					continue;
				case 1:
					if (int_common[0] == v0 || int_common[0] == v1) {
						continue;
					}
					AddPoint(int_common[0], &edges);
					break;
				case 2:
					if (int_common[0] == v0 && int_common[1] == v1 ||
						int_common[0] == v1 && int_common[1] == v0) {

						double t0 = line.line.getParam(v0);
						double t1 = line.line.getParam(v1);
						mline.force_add(std::min(t0, t1), edges[i]);

						_edgeGeometry[edges[i]].push_back(geometry.size());

						continue;
					}
					AddPoint(int_common[0], &edges);
					AddPoint(int_common[1], &edges);
				}
			}
			geometry.push_back(mline);
			return;
		}
		// OBSOLETE
		void Flip(const Mesh2::EdgeIndex edge) {
			
		}
		// w = 0 -> edges[edge].first
		// w = 1 -> edges[edge].second
		void SubdivideEdge(const Mesh2::EdgeIndex edge, double w = 0.5) {
			if (isBoundary(edge)) {
				for (auto i : _edgeGeometry[edge]) {
					auto t0 = geometry[i].pos[edge];
					auto t1 = geometry[i].next(t0);

					if (mesh.point(mesh.edge(edge).first) == geometry[i].line.sample(t0)) {
						AddPoint(geometry[i].line.sample(t0 * (1.0 - w) + t1 * w));
					}else{
						AddPoint(geometry[i].line.sample(t1 * (1.0 - w) + t0 * w));
					}
				}
			}
			AddPoint(mesh.sample_edge(edge, w));
		}
	protected:
	};
}