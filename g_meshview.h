#pragma once
#include "g_meshing.h"
#include "g_line_ext.h"

namespace fg {
	class IMeshView {
	public:
		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const = 0;
		virtual const size_t boundary_size() const = 0;
		virtual const Mesh2& mesh() const = 0;
		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const = 0;
	};
	class RectMeshView : public virtual IMeshView{
		const RectView& rect;
		Mesh2 _mesh;
		std::vector<MeshedLine> _boundary;
	public:
		RectMeshView(const RectView& m) : rect{ m }, _mesh{ m.mesh() } {
			for (auto i : m.lines) {
				for (auto j : i.lines) {
					_boundary.push_back(j);
				}
			}
		}
		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const {
			return _boundary[index];
		}
		virtual const size_t boundary_size() const {
			return _boundary.size();
		}
		virtual const Mesh2& mesh() const {
			return _mesh;
		}

		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const {
			return false;
		}
	};
	// базовая сетка, в которую будут добавляться вершины, линии, другие сетки
	class FEMCADGEOMSHARED_EXPORT MeshView2d : public virtual IMeshView {
		// коллекция линий геометрии
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
			void replace(Mesh2::EdgeIndex e, std::vector<std::pair<double, Mesh2::EdgeIndex>>& e_new) {
				double w = pos[e];
				pos.erase(e);
				for (auto i : e_new) {
					pos[i.second] = i.first;
					edges[i.first] = i.second;
				}
			}
		};
#ifdef _DEBUG
	public:
#endif
		// Внутренняя сетка. Состоит из вершин, ребер, треугольников. Треугольники состоят из ребер, а ребра из вершин.
		Mesh2 _mesh;
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
		template<class It>
		void edge_geometry_replace(size_t geom, size_t e, It e_begin, It e_end) {
			for (size_t i{}; i < _edgeGeometry[e].size(); i++) {
				if (geom == _edgeGeometry[e][i]) {
					for (auto j{ e_begin }; j != e_end; ++j) {
						_edgeGeometry[*j].push_back(geom);
					}
					_edgeGeometry[e][i] = _edgeGeometry[e].back();
					_edgeGeometry[e].pop_back();
					break;
				}
			}
		}

		inline void update_line(size_t geometry_index, vector3 point, std::vector<size_t>& edges) {
			auto& line = geometry[geometry_index];
			// локальная координата точки пересечения на линии line
			double t = line.line.getParam(point);
			// ребро сетки, лежащее на line, соответсвующее координате t
			auto e = line.get(t);
			// координата начала ребра e
			auto tt = line.pos[e];
			// если не попали в начало ребра, тогда нужно обновить границу
			if (std::fabs(tt - t) > FG_EPS) {
				size_t edge_beg;
				size_t edge_end;
				// находим начало и конец ребра
				_mesh.cast(line.line.sample(tt), edge_beg);
				edge_end = _mesh.edge(e).first;
				if (edge_end == edge_beg) edge_end = _mesh.edge(e).second;
				size_t e0, e1;

				static std::vector<std::pair<double, Mesh2::EdgeIndex>> add_edges;
				add_edges.resize(0);
				for (size_t j{}; j < edges.size(); j++) {
					auto edge_cur = _mesh.edge(edges[j]);
					if (line.line.classify(_mesh.point(edge_cur.first)) || line.line.classify(_mesh.point(edge_cur.second))) {
						if (j < edges.size() - 1) {
							edges[j] = edges.back();
							j--;
						}
						edges.pop_back();
					}
					else {
						double tt0 = std::min(line.line.getParam(_mesh.point(edge_cur.first)), line.line.getParam(_mesh.point(edge_cur.second)));
						if (std::fabs(tt0 - t) < FG_EPS) tt0 = t;
						add_edges.push_back(std::make_pair(tt0, edges[j]));
					}
				}
				edge_geometry_replace(geometry_index, e, edges.begin(), edges.end());
				geometry[geometry_index].replace(e, add_edges);
			}
		}

		bool intersectBoundary(vector3 p, std::vector<size_t>& geom) {
			for (size_t i{}; i < geometry.size(); ++i) {
				if (geometry[i].line.classify(p) == 0) {
					geom.push_back(i);
				}
			}
			return geom.size();
		}
	public:
		inline bool isBoundary(Mesh2::EdgeIndex i) const {
			//if (_edgeGeometry.size() <= i) return false;
			return _edgeGeometry[i].size() > 0;
			//return std::any_of(geometry.begin(), geometry.end(), [=](_meshLineView x)->bool {return x.pos.count(i) > 0; });
		}

		MeshView2d(const IMeshView& r) : _mesh(r.mesh())
		{
			std::map<Mesh2::Edge, Mesh2::EdgeIndex> edges;
			_edgeGeometry.resize(_mesh.edgesCount());
			for (size_t i{}; i < _mesh.edges.size(); i++) {
				auto e = _mesh.edges[i];
				edges[e] = i;
				edges[std::make_pair(e.second, e.first)] = i;
			}
			auto p = 0U;
			auto pnext = 0U;
			//const auto& rlines = r.boundary();
			for (size_t i{}; i < r.boundary_size(); i++) {
				geometry.emplace_back(_meshLineView(r.boundary(i).line));
				auto& ps = r.boundary(i).points;
				vector3 sample_point = geometry.back().line.sample(0.0);
				auto res = _mesh.cast(sample_point, p);
				if (res != GeometryType::Vertex) throw FGException("Incorrect built mesh!");
				for (size_t k{ 0 }; k <= ps.size(); k++) {
					sample_point = geometry.back().line.sample(k == ps.size() ? 1.0 : ps[k]);
					res = _mesh.cast(sample_point, pnext);
					if (res != GeometryType::Vertex) throw FGException("Incorrect built mesh!");
					auto e = edges[std::make_pair(p, pnext)];
					_edgeGeometry[e].push_back(geometry.size() - 1);
					geometry.back().force_add(k == 0 ? 0.0 : ps[k - 1], e);

					p = pnext;
				}
			}
		}
		void _addPoint(const vector3& pos, std::vector<Mesh2::EdgeIndex>* added_edges = nullptr) {
			std::tuple<size_t, size_t, size_t> out;
			// хэндлер элемента геометрии, куда попала добавляемая точка
			size_t el;
			// определим, в какой тип элемента геометрии попала добавляемая точка и его хэндлер
			auto res = _mesh.cast(pos, el);
			//auto res = mesh.insert_point(pos, out, el);

			static std::vector<vector3> int_points;
			static std::vector<size_t> int_index;
			static std::vector<Mesh2::EdgeIndex> new_edges;
			static std::vector<size_t> geoms;


			auto process_edges = [&]() {
				while (!new_edges.empty()) {
					for (auto i : geoms) {
						for (size_t j{}; j < new_edges.size(); j++) {
							if (new_edges[j] == Mesh2::NotAnEdge) continue;
#ifdef _DEBUG
							std::cout << new_edges[j] << ", ";
#endif
							auto& m = _mesh;
							auto count = _mesh.intersect(static_cast<const EllipticSegment&>(geometry[i].line), new_edges[j], int_points);
							for (size_t k{}; k < count; k++) int_index.push_back(i);
						}
					}
					new_edges.resize(0);
					res = GeometryType::Vertex;
					while (!int_points.empty()) {
						res = _mesh.insert_point(int_points.back(), out, el);

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
						if (e0 != Mesh2::NotAnEdge && e0 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						if (e1 != Mesh2::NotAnEdge && e1 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						// Дуга с индексом int_index.back() пересеклась с ребром el в точке int_points.back()

						auto& line = geometry[int_index.back()];
						double t = line.line.getParam(int_points.back());
						auto e = line.get(t);
						auto tt = line.pos[e];
						size_t indx;
						_mesh.cast(line.line.sample(tt), indx);

						if (_mesh.edge(e0).first == indx || _mesh.edge(e0).second == indx)
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
						if (_mesh.isLineInsideTriangle(es, edge, tri))
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
				// вернулись индексы ребер тр-ка, в который попала добавляемая точка
				auto edges = _mesh.triangle(el);
				// добавим точку в тр-к и вернем индексы трех добавленых ребер в out
				auto res = _mesh.insert_point(pos, out, el);

				// вытащим ребра из out
				auto eo = std::get<0>(out);
				auto e0 = std::get<1>(out);
				auto e1 = std::get<2>(out);

				add(std::get<0>(edges), el);
				add(std::get<1>(edges), el);
				add(std::get<2>(edges), el);

				if (eo >= _edgeGeometry.size()) _edgeGeometry.push_back({});
				if (e0 >= _edgeGeometry.size() && e0 != Mesh2::NotAnEdge) _edgeGeometry.push_back({});
				if (e1 >= _edgeGeometry.size() && e1 != Mesh2::NotAnEdge) _edgeGeometry.push_back({});


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
				auto et = _mesh.edge_triangle(el);
				auto tri0 = et.first;
				auto tri1 = et.second;
				auto edges = _mesh.triangle(tri0);

				int_points.resize(0);
				int_index.resize(0);
				new_edges.resize(0);
				geoms.resize(0);
				add(std::get<0>(edges), tri0);
				add(std::get<1>(edges), tri0);
				add(std::get<2>(edges), tri0);

				auto res = _mesh.insert_point(pos, out, el);
				if (std::get<0>(out) >= _edgeGeometry.size()) _edgeGeometry.push_back({});
				if (std::get<1>(out) >= _edgeGeometry.size() && std::get<1>(out) != Mesh2::NotAnEdge) _edgeGeometry.push_back({});
				if (std::get<2>(out) >= _edgeGeometry.size() && std::get<2>(out) != Mesh2::NotAnEdge) _edgeGeometry.push_back({});

				new_edges.push_back(std::get<1>(out));
				if (added_edges) {
					added_edges->push_back(el);
					added_edges->push_back(std::get<0>(out));
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

					auto res = _mesh.insert_point(pos, out, el);
					if (std::get<0>(out) >= _edgeGeometry.size()) _edgeGeometry.push_back({});
					if (std::get<1>(out) >= _edgeGeometry.size() && std::get<1>(out) != Mesh2::NotAnEdge) _edgeGeometry.push_back({});
					if (std::get<2>(out) >= _edgeGeometry.size() && std::get<2>(out) != Mesh2::NotAnEdge) _edgeGeometry.push_back({});

					new_edges.push_back(std::get<2>(out));
					if (added_edges) {
						added_edges->push_back(std::get<2>(out));
					}
					process_edges();
				}
			}
		}

		void AddPoint(const vector3& pos, std::vector<Mesh2::EdgeIndex>* added_edges = nullptr) {
			static std::vector<size_t> geom;
			static std::vector<size_t> edges;
			geom.resize(0);
			edges.resize(0);
			_addPoint(pos, &edges);
			if (added_edges) {
				added_edges->insert(added_edges->end(), edges.begin(), edges.end());
			}
			if (edges.size() && intersectBoundary(pos, geom)) {
				for (size_t i = 0; i < geom.size(); i++){
					update_line(geom[i], pos, edges);
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
			// вектор хэндлов линий геометрии, с которыми пересеклась добавляемая линия
			static std::vector<size_t> geoms;
			int0.resize(0);
			int1.resize(0);
			int_common.resize(0);
			edges_set.clear();
			triangles_set.clear();
			geoms.resize(0);
			// для всех линий геометрии базовой сетки
			for (size_t i{}; i < geometry.size(); i++) {
				// ищем пересечение линии геометрии базовой сетки и добавляемой линии
				Intersector<ILine>::intersect_dynamic(int0, int1, line.line, geometry[i].line);
				// добавляем найденные точки пересечения в int_common и линию геометрии, с которой пересеклись в geoms
				for (size_t j{}; j < int0.size(); j++) {
					for (size_t k{ j }; k < int1.size(); k++) {
						if (int0[j] == int1[k]) {
							int_common.push_back(int0[j]);
							geoms.push_back(i);
						}
					}
				}
			}
			
			// для всех точек пересечения ???
			for (size_t i{}; i < int_common.size(); i++) {
				//edges.resize(0);
				// добавили точку пересечения в базовую сетку
				AddPoint(int_common[i], &edges);
				//if(edges.size())
				//	update_line(geoms[i], int_common[i], edges);
			}
			// добавили вершины начала и конца добавляемой линии в базовую сетку
			AddPoint(line.p0());
			AddPoint(line.p1());

			// формирование ограничивающих объемов подсегментов добавляемого сегмента
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
			// Поиск треугольников пересекающихся с ограничивающими объемами
			for (size_t i{}; i < boxes.size(); ++i) {
				_mesh.lookup().get_overlap(boxes[i], triangles_set);
			}

			// Формирование списка ребер сетки потенциально пересекающихся с добавляемым сегментом
			for (auto i : triangles_set) {
				edges_set.insert(std::get<0>(_mesh.triangle(i)));
				edges_set.insert(std::get<1>(_mesh.triangle(i)));
				edges_set.insert(std::get<2>(_mesh.triangle(i)));
			}
			auto mline = _meshLineView(line.line);
			edges.resize(0);
			edges.insert(edges.end(), edges_set.begin(), edges_set.end());
			size_t sz = geometry.size();
			geoms.clear();
			for (size_t i{}; i < edges.size(); ++i) {
				int_common.resize(0);
				auto e = _mesh.edge(edges[i]);
				auto v0 = _mesh.point(e.first);
				auto v1 = _mesh.point(e.second);
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
						geoms.push_back(edges[i]);
						//_edgeGeometry[].push_back(sz);
					
						continue;
					}
					AddPoint(int_common[0], &edges);
					AddPoint(int_common[1], &edges);
					break;
				}
			}
			
			for (auto i : geoms) {
				if (mline.get(mline.pos[i]) != i) {
					mline.pos.erase(i);
					continue;
				}
				_edgeGeometry[i].push_back(sz);
			}
			geometry.push_back(mline);
			return;
		}
		// OBSOLETE
		void Flip(const Mesh2::EdgeIndex edge) {

		}
		// w = 0 -> edges[edge].first
		// w = 1 -> edges[edge].second
		template<class Collection>
		void SubdivideEdge(const Mesh2::EdgeIndex edge, Collection& result_edges, double w = 0.5) {
			static std::vector<size_t> edges;
			edges.resize(0);
			vector3 pp = _mesh.sample_edge(edge, w);
			if (isBoundary(edge)) {
				for (auto i{ 0U }; i < _edgeGeometry[edge].size(); ++i) {
					edges.resize(0);
					auto t0 = geometry[_edgeGeometry[edge][i]].pos[edge];
					auto t1 = geometry[_edgeGeometry[edge][i]].next(t0);

					vector3 p;
					if (_mesh.point(_mesh.edge(edge).first) == geometry[_edgeGeometry[edge][i]].line.sample(t0)) {
						p = geometry[_edgeGeometry[edge][i]].line.sample(t0 * (1.0 - w) + t1 * w);
					}
					else {
						p = geometry[_edgeGeometry[edge][i]].line.sample(t1 * (1.0 - w) + t0 * w);
					}
					AddPoint(p, &edges);
					if (edges.size()) {
						result_edges.insert(result_edges.end(), edges.begin(), edges.end());
						//update_line(_edgeGeometry[edge][i], p, edges);
					}
				}
			}
			edges.resize(0);
			AddPoint(pp, &edges);
			if (edges.size())
				result_edges.insert(result_edges.end(), edges.begin(), edges.end());
		}

		std::array<Mesh2::EdgeIndex, 3> CollapseEdge(const Mesh2::EdgeIndex edge) {
			
			for (auto i : _mesh.PointEdges()[_mesh.edge(edge).first]) {
				auto t = _mesh.triangle(_mesh.edge_triangle(i).first);
				if (isBoundary(std::get<0>(t))) return NotCollapsed;
				if (isBoundary(std::get<1>(t))) return NotCollapsed;
				if (isBoundary(std::get<2>(t))) return NotCollapsed;
				t = _mesh.triangle(_mesh.edge_triangle(i).second);
				if (isBoundary(std::get<0>(t))) return NotCollapsed;
				if (isBoundary(std::get<1>(t))) return NotCollapsed;
				if (isBoundary(std::get<2>(t))) return NotCollapsed;
			}
			for (auto i : _mesh.PointEdges()[_mesh.edge(edge).second]) {
				auto t = _mesh.triangle(_mesh.edge_triangle(i).first);
				if (isBoundary(std::get<0>(t))) return NotCollapsed;
				if (isBoundary(std::get<1>(t))) return NotCollapsed;
				if (isBoundary(std::get<2>(t))) return NotCollapsed;
				t = _mesh.triangle(_mesh.edge_triangle(i).second);
				if (isBoundary(std::get<0>(t))) return NotCollapsed;
				if (isBoundary(std::get<1>(t))) return NotCollapsed;
				if (isBoundary(std::get<2>(t))) return NotCollapsed;
			}
#ifdef _DEBUG
			for (size_t i{}; i < _mesh.PointEdges().size(); ++i) {
				for (auto j : _mesh.PointEdges()[i]) {
					if (j >= _mesh.edge_triangles.size()) {
						throw "beda";
					}

				}
			}
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				auto tr = _mesh.triangle(i);
				if (_mesh.edge_triangles[std::get<0>(tr)].first != i && _mesh.edge_triangles[std::get<0>(tr)].second != i ||
					_mesh.edge_triangles[std::get<1>(tr)].first != i && _mesh.edge_triangles[std::get<1>(tr)].second != i ||
					_mesh.edge_triangles[std::get<2>(tr)].first != i && _mesh.edge_triangles[std::get<2>(tr)].second != i)
					throw;
			}
#endif
			auto w = _mesh.collapseWeight(edge);
			auto s = _mesh.collapseEdge(edge, w);

#ifdef _DEBUG
			for (size_t i{}; i < _mesh.PointEdges().size(); ++i) {
				for (auto j : _mesh.PointEdges()[i]) {
					if (j >= _mesh.edge_triangles.size()) {
						throw "beda";
					}

				}
			}
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				auto tr = _mesh.triangle(i);
				if (_mesh.edge_triangles[std::get<0>(tr)].first != i && _mesh.edge_triangles[std::get<0>(tr)].second != i ||
					_mesh.edge_triangles[std::get<1>(tr)].first != i && _mesh.edge_triangles[std::get<1>(tr)].second != i ||
					_mesh.edge_triangles[std::get<2>(tr)].first != i && _mesh.edge_triangles[std::get<2>(tr)].second != i)
					throw;
			}
#endif

			return s;
		}

		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const {
			return MeshedLine(geometry[index].line);
		}
		virtual const size_t boundary_size() const {
			return geometry.size();
		}
		virtual const Mesh2& mesh() const { return _mesh; }
	protected:
	};
}