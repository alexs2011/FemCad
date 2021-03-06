﻿#pragma once
#include "g_meshing.h"
#include "g_line_ext.h"
#include "g_element_size.h"
namespace fg {
	class IGeometryView {
	public:
		virtual const size_t boundary_size() const = 0;
		virtual MeshedLine boundary(size_t index) const = 0;
		virtual size_t materialId(const vector3& p)const { return 0xFFFFFFFF; }
		virtual std::map<size_t, SETTINGHANDLE> listSettings() const { return{}; }
	};

	class IMeshView
	{
	public:
		size_t dbg_edge_to_be_processed = -1; // индекс ребра, которое будет обработано следующим
	public:
		virtual const Mesh2& mesh() const = 0;
		bool isToBeProcessed(Mesh2::EdgeIndex i) const
		{
			return i == dbg_edge_to_be_processed;
		}
	};

	class IMeshGeometryView : public virtual IGeometryView, public virtual IMeshView {
	public:
		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const = 0;
		virtual size_t materialId(Mesh2::EdgeIndex p)const { return 0xFFFFFFFF; }
	};



	class MeshElementSizeView : public virtual ElementSize, public virtual IMeshGeometryView {
	public:
		using ElementSize::ElementSize;
		template<class S, class... Targs>
		void setIsoSize(Targs... args) {
			static_assert(std::is_convertible<S, MeshElementSize<double>>::value, "Bad iso size parameter");
			_isoSize = std::make_shared<S>(mesh(), args...);
		}
		template<class S, class... Targs>
		void setAnisoSize(Targs... args) {
			static_assert(std::is_convertible<S, MeshElementSize<vector3>>::value, "Bad aniso size parameter");
			_anisoSize = std::make_shared<S>(mesh(), args...);
		}
	};

	class ElementGeometry : public virtual IGeometryView, public virtual ElementSize {

	};

	class RectMeshView : public virtual IMeshGeometryView, public virtual MeshElementSizeView, public virtual ElementGeometry {
		const RectView& rect;
		Mesh2 _mesh;
		std::vector<MeshedLine> _boundary;
	public:
		RectMeshView(const RectView& m) : rect{ m }, _mesh{ m.mesh() }, MeshElementSizeView{ nullptr, nullptr } {
			for (auto i : m.lines) {
				for (auto j : i.lines) {
					_boundary.push_back(j);
				}
			}
		}
		virtual MeshedLine boundary(size_t index) const {
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
	class FEMCADGEOMSHARED_EXPORT MeshView2d : public virtual IMeshGeometryView, public virtual MeshElementSizeView, public virtual ElementGeometry {
		// коллекция линий геометрии
		struct _meshLineView {
			const ILine& line;
			std::map<Mesh2::EdgeIndex, double> pos;
			std::map<double, Mesh2::EdgeIndex> edges;
			_meshLineView(const ILine& l) : line{ l } {}
			Mesh2::EdgeIndex get(double t) const {
				if (edges.upper_bound(FG_EPS) == edges.begin()) {
					throw;
				}
				t += FG_EPS;
				auto s = edges.lower_bound(t);
				s--;
				return s->second;
			}
			double next(double t) {
				t += FG_EPS;
				auto s = edges.upper_bound(t);
				return s == edges.end() ? 1.0 : s->first;
			}
			void force_add(double t, Mesh2::EdgeIndex e) {
				edges[t] = e;
				pos[e] = std::fabs(t);
			}
			void replace_edge(Mesh2::EdgeIndex old, Mesh2::EdgeIndex nw) {
				auto k = pos[old];
				edges[k] = nw;
				pos.erase(old);
				pos[nw] = k;
			}
			void add(double t, Mesh2::EdgeIndex e0, Mesh2::EdgeIndex e1) {
				if (edges.empty()) {
					edges[0.0] = e0;
					pos[e0] = 0.0;
					edges[std::fabs(t)] = e1;
					pos[e1] = t;
				}
				else {
					auto e = edges.lower_bound(t);
					auto w = pos[e->second];
					pos.erase(e->second);
					e->second = e0;
					pos[e0] = e->first;
					edges[t] = e1;
					pos[e1] = std::fabs(t);
				}
			}
			void replace(double t, Mesh2::EdgeIndex e, Mesh2::EdgeIndex e0, Mesh2::EdgeIndex e1) {
				assert(t < 1.0);
				double w = pos[e];
				pos.erase(e);
				pos[e0] = w;
				pos[e1] = std::fabs(t);
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
			void collapse(Mesh2::EdgeIndex e, double point) {
				auto p = pos[e];
				auto next = edges.upper_bound(p+FG_EPS);
				//next++;
				if (next == edges.end()) {
					pos.erase(e);
					edges.erase(p);
					return;
				}
				pos[next->second] = point;
				edges[point] = next->second;
				if (point != p) edges.erase(next->first);
				edges.erase(p);
				pos.erase(e);
			}
		};
#ifndef _DEBUG_
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
			if (t < -FG_EPS || t > 1.0 + FG_EPS) return;
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
			if (_edgeGeometry.size() <= i) {
				//std::cout << "What da " << i << std::endl;
				return false;
			}
			return _edgeGeometry[i].size() > 0;
			//return std::any_of(geometry.begin(), geometry.end(), [=](_meshLineView x)->bool {return x.pos.count(i) > 0; });
		}

		virtual size_t materialId(Mesh2::EdgeIndex p)const {
			if (_edgeGeometry[p].size() > 0) {
				return geometry[_edgeGeometry[p][0]].line.getSetting()->getID();
			}
			return 0xFFFFFFFF;
		}


		MeshView2d(const IMeshGeometryView& r) : MeshElementSizeView{ nullptr, nullptr }, _mesh(r.mesh())
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

#ifdef _DEBUG
			//auto tree = &_mesh.lookup();
			//std::ofstream file("lookup_tree.js", std::ofstream::out);
			//size_t initial_nodes_index = 0;
			//_mesh.printLookupTree(file, tree, initial_nodes_index);
			//file.close();
#endif // _DEBUG

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
							//std::cout << new_edges[j] << ", ";
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
						auto e0 = std::get<0>(out);
						auto e1 = std::get<1>(out);
						auto e2 = std::get<2>(out);
						//if (std::get<0>(out) != 0xFFFFFFFF) 
						if (e0 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						if (e1 != Mesh2::NotAnEdge && e1 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						if (e2 != Mesh2::NotAnEdge && e2 >= _edgeGeometry.size()) _edgeGeometry.push_back({});
						// Дуга с индексом int_index.back() пересеклась с ребром el в точке int_points.back()

						auto& line = geometry[int_index.back()];
						double t = line.line.getParam(int_points.back());
						auto e = line.get(t);
						auto e_edge = _mesh.edge(e);
						auto tt = line.pos[e];
						size_t indx;
						_mesh.cast(line.line.sample(tt), indx);

						e_edge = e_edge.first == indx ? e_edge : std::make_pair(e_edge.second, e_edge.first);

						size_t ee0, ee1;// , vx;
						//vx = _mesh.edge(e0).first == _mesh.edge(e1).second ? _mesh.edge(e0).second :
						//	(_mesh.edge(e0).first == _mesh.edge(e1).first ? _mesh.edge(e0).first :
						//	(_mesh.edge(e0).second == _mesh.edge(e1).second ? _mesh.edge(e0).second : _mesh.edge(e1).first));

						auto fit_edge = [&](size_t edg, size_t& outedg_first, size_t& outedg_second) {
							if (_mesh.edge(edg).first == e_edge.first || _mesh.edge(edg).second == e_edge.first) {
								outedg_first = edg;
							}
							else if (_mesh.edge(edg).first == e_edge.second || _mesh.edge(edg).second == e_edge.second) {
								outedg_second = edg;
							}
						};

						fit_edge(el, ee0, ee1);
						fit_edge(e0, ee0, ee1);
						fit_edge(e1, ee0, ee1);
						fit_edge(e2, ee0, ee1);

						geometry[int_index.back()].replace(t, e, ee0, ee1);

						//if (_mesh.edge(e0).first == indx || _mesh.edge(e0).second == indx)
						//	geometry[int_index.back()].replace(t, e, e0, e1);
						//else
						//	geometry[int_index.back()].replace(t, e, e1, e0);

						edge_geometry_replace(int_index.back(), e, ee0, ee1);

						new_edges.push_back(e0);
						new_edges.push_back(e1);

						if (added_edges) {
							added_edges->push_back(e0);
							added_edges->push_back(e1);
							added_edges->push_back(e2);
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
				if (_DebugTest()) {
					std::cout << "Add point pre insert Triangle\n";
				}
				res = _mesh.insert_point(pos, out, el);
				if (_DebugTest()) {
					std::cout << "Add point post insert Triangle\n";
				}

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

				if (_DebugTest()) {
					std::cout << "Add point pre insert Edge \n";
				}
				res = _mesh.insert_point(pos, out, el);
				if (_DebugTest()) {
					std::cout << "Add point post insert Edge\n";
				}


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

					_DebugTest();
					res = _mesh.insert_point(pos, out, el);
					_DebugTest();
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
				for (size_t i = 0; i < geom.size(); i++) {
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

			if (_DebugTest()) {
				std::cout << "Add line in \n";
			}
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

			if (_DebugTest()) {
				std::cout << "Add line Points added \n";
			}

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
			for (size_t i{}; i < boxes.size(); ++i)
				_mesh.lookup().get_overlap(boxes[i], triangles_set);

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
			if (_DebugTest()) {
				std::cout << "Add line Strange point \n";
			}

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

			if (_DebugTest()) {
				std::cout << "Add line out \n";
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

		void Flip(const Mesh2::EdgeIndex edge, std::vector<size_t>& edges, double r = 1.0) {
			//return;
			if (isBoundary(edge))
				return;
			//try {

				//std::pair<size_t, size_t> t;
				//auto edges = _mesh.get_quadrangle_edges(edge, t);
				//for (size_t i{}; i < edges.size(); ++i) {
				//	auto ti = (i < 2) ? t.first : t.second;
				//	auto ei = edges[i];
				//	if (isBoundary(ei)) {
				//		for (auto j : _edgeGeometry[ei]) {
				//			try {
				//				auto& es = dynamic_cast<const EllipticSegment&>(geometry[j].line);
				//				if (_mesh.isLineInsideTriangle(es, ei, ti))
				//					return;
				//			}
				//			catch (...) {}
				//		}
				//	}
				//}
			//}
			//catch (...) {
			//	return;
			//}

			if (_DebugTest(-100)) {
				std::cout << "Failed at in of Flip " << edge << std::endl;
				throw;
			}
			if (_mesh.flip_force(edge)) {
				auto s = _mesh.get_quadrangle_edges(edge);
				auto e = _mesh.flip(edge);
				edges.insert(edges.end(), s.begin(), s.end());
				return;
			}
			if (_mesh.flippable(edge, r)) {
				std::vector<vector3> int_points;
				auto s = _mesh.get_quadrangle_edges(edge);
				auto ed = _mesh.edge(edge);
				auto te = _mesh.edge(s[0]);
				size_t pp0 = te.first != ed.first && te.first != ed.second ? te.first : te.second;
				te = _mesh.edge(s[2]);
				size_t pp1 = te.first != ed.first && te.first != ed.second ? te.first : te.second;
				auto p0 = _mesh.point(pp0);
				auto p1 = _mesh.point(pp1);
				auto intersect_boundary = [&](size_t bound) -> bool {
					if (edge == bound) return false;
					for (auto i : _edgeGeometry[bound]) {
						const EllipticSegment* el = dynamic_cast<const EllipticSegment*>(&geometry[i].line);
						if (el == nullptr) continue;
						if (Intersector<ILine>::intersect_segment(*el, p0, p1, int_points) > 1)
							return true;
					}
					return false;
				};
				for (auto i : s) {
					if (intersect_boundary(i)) return;
				}

				auto e = _mesh.flip(edge);
				edges.insert(edges.end(), s.begin(), s.end());
			}
			if (_DebugTest(-100)) {
				std::cout << "Failed at end of Flip " << edge << std::endl;
				throw;
			}
		}
		// w = 0 -> edges[edge].first
		// w = 1 -> edges[edge].second
		template<class Collection>
		void SubdivideEdge(const Mesh2::EdgeIndex edge, Collection& result_edges, double w = 0.5) {
			static std::vector<size_t> edges;
			edges.resize(0);
			vector3 pp = _mesh.sample_edge(edge, w);
			if (_DebugTest()) {
				std::cout << "Subdivide in with " << edge;
			}
			if (isBoundary(edge)) {
				for (auto i{ 0U }; i < _edgeGeometry[edge].size(); ++i) {
					edges.resize(0);

					//auto t0 = geometry[_edgeGeometry[edge][i]].line.getParam(_mesh.point(_mesh.edge(edge).first));
					//auto t1 = geometry[_edgeGeometry[edge][i]].line.getParam(_mesh.point(_mesh.edge(edge).second));
					//
					//vector3 p = geometry[_edgeGeometry[edge][i]].line.sample(t0 * (1.0 - w) + t1 * w);
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
				return;
			}
			edges.resize(0);
			if (_DebugTest()) {
				std::cout << "Subdivide after condition with " << edge;
			}

			AddPoint(pp, &edges);
			if (_DebugTest()) {
				std::cout << "Subdivide out with " << edge;
			}
			if (edges.size()) {
				result_edges.insert(result_edges.end(), edges.begin(), edges.end());
			}
		}
		bool _DebugTestIntersection(size_t vx = 0xFFFFFFFF, size_t vt = 0xFFFFFFFF) const {
#ifdef intersection_test
			if (vx != 0xFFFFFFFF && vt != 0xFFFFFFFF) {
				for (auto i : _mesh.point_edges[vx]) {
					auto e0 = _mesh.edges[i];
					if ((e0.first == vx && e0.second == vt) || (e0.first == vt && e0.second == vx)) continue;
					for (auto j : _mesh.point_edges[vt]) {
						auto e1 = _mesh.edges[j];
						if (i == j || (e1.first == vx && e1.second == vt) || (e1.first == vt && e1.second == vx)) continue;
						if ((e1.first == e0.first || e1.second == e0.first) || (e1.first == e0.second || e1.second == e0.second)) continue;
						auto p = segmentIntersection(_mesh.points[e0.first], _mesh.points[e0.second], _mesh.points[e1.first], _mesh.points[e1.second]);
						if (p.isInf() || p.isNan())
							continue;
						std::cout << "Intersection found: " << p << std::endl;
						return true;
						//_mesh.
					}
				}
	}
#endif
			return false;
}
		bool _DebugTest(int val = 0) {
#ifdef TREE_CHECK
			for (size_t i{}; i < _mesh.edgesCount(); ++i) {
				if (isBoundary(i)) {
					auto e = _mesh.edge(i);
					for (auto j : _edgeGeometry[i]) {
						if (geometry[j].line.classify(_mesh.point(e.first)) != 0 ||
							geometry[j].line.classify(_mesh.point(e.second)) != 0) {
							std::cout << "Test 0" << std::endl;
							return true;
						}
					}
				}
			}
			if (!_mesh.triangle_lookup->isCorrect()) {
				std::cout << "Test 3" << std::endl;
				return true;
			}
			if (_mesh.triangle_lookup->traverse_tree([&](const rect& r, size_t index) {return r != _mesh.get_rect(_mesh.triangle(index)); })) {
				std::cout << "Test 2" << std::endl;
				return true;
			}
			if (_mesh.triangle_lookup->traverse_tree([&](const rect& r, size_t index) {return index >= _mesh.TrianglesLength(); })) {
				std::cout << "Test 1" << std::endl;
				return true;
			}
			//for (auto elem : _mesh.lookup().container)
			//{
			//	if (elem.first != _mesh.get_rect(_mesh.triangles[elem.second]))
			//	{
			//		std::cout << "Bad rect";
			//		throw;
			//	}
			//}
			//_mesh.get_rect
			//if (_mesh.triangle_lookup->has_element_pred(_mesh.triangles.size(), [&](size_t l, size_t size) { 
			//	return l >= size; 
			//})) return true;
			//for (size_t i{}; i < _mesh.triangles.size(); ++i) {
			//}
#endif
#ifndef _DEBUG__
//#define _point_edges_test
#ifdef _point_edges_test
			for (size_t i{}; i < _mesh.PointEdges().size(); ++i) {
				for (auto j : _mesh.PointEdges()[i]) {
					if (j >= _mesh.edge_triangles.size()) {
						assert(false);
					}

				}
			}
#endif
			//#define _planar_triangles
#ifdef _planar_triangles
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				auto pts = _mesh.triangleVertices(i);
				auto pp0 = _mesh.points[std::get<0>(pts)];
				auto pp1 = _mesh.points[std::get<1>(pts)];
				auto pp2 = _mesh.points[std::get<2>(pts)];
				try {
					plane::byThreePonts(pp0, pp1, pp2);
				}
				catch (...) {
					std::cout << '"' << i << pp0 << pp1 << pp2 << '"';
					throw;
				}
			}
#endif
//#define _double_edged_triangles_test
#ifdef _double_edged_triangles_test
			std::set<std::pair<size_t, size_t>> p;
			for (size_t i{}; i < _mesh.edgesCount(); ++i) {
				auto pp = _mesh.edge_triangle(i);
				if (pp.first > pp.second)
					pp.first ^= pp.second ^= pp.first ^= pp.second;
				if (p.count(pp) && pp.first != pp.second) {
					std::cout << '+' << i << '+';
					throw "baaad";
				}
				p.insert(pp);

				/*for (size_t j{ i + 1 }; j < _mesh.edgesCount(); ++j) {
					if ((_mesh.edge_triangle(i).first == _mesh.edge_triangle(j).first &&
						_mesh.edge_triangle(i).second == _mesh.edge_triangle(j).second) ||
						(_mesh.edge_triangle(i).first == _mesh.edge_triangle(j).second &&
							_mesh.edge_triangle(i).second == _mesh.edge_triangle(j).first))
				}*/
			}
#endif
//#define _edge_triangles_test
#ifdef _edge_triangles_test
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				auto tr = _mesh.triangle(i);
				if (_mesh.edge_triangles[std::get<0>(tr)].first != i && _mesh.edge_triangles[std::get<0>(tr)].second != i ||
					_mesh.edge_triangles[std::get<1>(tr)].first != i && _mesh.edge_triangles[std::get<1>(tr)].second != i ||
					_mesh.edge_triangles[std::get<2>(tr)].first != i && _mesh.edge_triangles[std::get<2>(tr)].second != i) {
					std::cout << '+' << i;

					throw;
				}
			}
#endif
#ifdef _lookup_test
			if (_mesh.triangle_lookup->has_element_pred(_mesh.TrianglesLength(), [](size_t l, size_t r) {return l >= r; })) {
				assert(false);
			}
#endif
//#define _edge_points_test
#ifdef _edge_points_test
			for (size_t i{}; i < _mesh.edges.size(); ++i) {
				if (_mesh.edges[i].first == _mesh.edges[i].second) {
					assert(false);
				}
			}
#endif
			//#define pretest
#ifdef pretest
			for (size_t i{}; i < _mesh.points.size(); ++i) { //i=857, j=1605
				for (size_t j{}; j < _mesh.triangles.size(); ++j) {
					if (i == 857 && j == 1605)
					{
						vector3 l;
						_mesh.test(j, _mesh.point(i), l);
						if (l.x > FG_EPS && l.y > FG_EPS && l.z > FG_EPS) {
							std::cout << i << ' ' << j << ' ' << l << std::endl;
							throw "Ne Tak";
						}
					}
				}
			}
#endif

#endif
			return false;
		}

		std::vector<Mesh2::EdgeIndex> CollapseEdge(const Mesh2::EdgeIndex edge) {
			if (_edgeGeometry[edge].size() > 1) return{};

			/*if (isBoundary(edge)) {


				return{};
			}*/
			auto g = _edgeGeometry[edge].begin();

			static std::vector<Mesh2::EdgeIndex> result;
			result.clear();

			bool first_is_edge{ false }, second_is_edge{ false };

			auto p = _mesh.edge(edge).first;
			for (auto i : _mesh.PointEdges()[p]) {
				if (i == edge) continue;
				if (isBoundary(i)) {
					if (g != _edgeGeometry[edge].end()) {
						bool skip = true;
						for (auto j : _edgeGeometry[i]) {
							if (*g != j) {
								skip = false;
							}
						}
						if (skip) continue;
					}
					first_is_edge = true;
					break;
				}
			}
			p = _mesh.edge(edge).second;
			for (auto i : _mesh.PointEdges()[p]) {
				if (i == edge) continue;
				if (isBoundary(i)) {
					if (g != _edgeGeometry[edge].end()) {
						bool skip = true;
						for (auto j : _edgeGeometry[i]) {
							if (*g != j) {
								skip = false;
							}
						}
						if (skip) continue;
					}
					second_is_edge = true;
					break;
				}
			}
			if (first_is_edge && second_is_edge) {

				return{};
			}

			//_DebugTest();
			auto ep0 = _mesh.edges[edge].first;
			auto ets = _mesh.oppositeVertices(edge);

			//if (_DebugTestIntersection(ep0, ets.first) || (ets.first != ets.second && _DebugTestIntersection(ep0, ets.second))) {
			//	throw "Sovsem beda";
			//}
			auto pt = _mesh.edge(edge).first;
			if (pt == _mesh.pointsCount() - 1) {
				pt = _mesh.edge(edge).second;
			}
			auto w = _mesh.collapseWeight(edge);
			if (isnan(w.first)) return{};
			double weight = 0.5 * (w.first + w.second);



			auto check_weight_intersection = [&](size_t point, const vector3& wpoint) {
				std::vector<vector3> int_points;
				auto intersect_boundary = [&](size_t ind, size_t bound) -> bool {
					if (ind == bound) return false;
					auto edge_beg = point == _mesh.edge(ind).first ? _mesh.edge(ind).second : _mesh.edge(ind).first;
					for (auto i : _edgeGeometry[bound]) {
						const EllipticSegment* el = dynamic_cast<const EllipticSegment*>(&geometry[i].line);
						if (el == nullptr) continue;
						if (Intersector<ILine>::intersect_segment(*el, _mesh.point(edge_beg), wpoint, int_points) == 1)
							return true;
					}
					return false;
				};
				for (auto i : _mesh.PointEdges()[point]) {
					if (i == edge) continue;
					auto t = _mesh.triangle(_mesh.edge_triangle(i).first);
					if (intersect_boundary(i, std::get<0>(t))) return false;
					if (intersect_boundary(i, std::get<1>(t))) return false;
					if (intersect_boundary(i, std::get<2>(t))) return false;
					t = _mesh.triangle(_mesh.edge_triangle(i).second);
					if (intersect_boundary(i, std::get<0>(t))) return false;
					if (intersect_boundary(i, std::get<1>(t))) return false;
					if (intersect_boundary(i, std::get<2>(t))) return false;
				}
				return true;
			};
			if (first_is_edge) {
				if (w.first <= FG_EPS) {
					weight = 0.0;
					auto wpoint = _mesh.sample_edge(edge, weight);
					if (!check_weight_intersection(_mesh.edge(edge).first, wpoint))
						return{};
				}
				else {
					return{};
				}
			}
			if (second_is_edge) {
				if (w.second >= 1.0 - FG_EPS) {
					weight = 1.0;
					auto wpoint = _mesh.sample_edge(edge, weight);
					if (!check_weight_intersection(_mesh.edge(edge).second, wpoint))
						return{};
				}
				else {
					return{};
				}
			}


			//std::cout << _mesh.point(_mesh.edge(edge).first) << _mesh.point(_mesh.edge(edge).second) << std::endl;
			//std::cout << "w= " << weight << std::endl;

			//if (w != 0.5) return{Mesh2::NotAnEdge,Mesh2::NotAnEdge ,Mesh2::NotAnEdge };
			//if(std::isnan(w)) return{ Mesh2::NotAnEdge,Mesh2::NotAnEdge ,Mesh2::NotAnEdge };
			if (_DebugTest(100)) {
				std::cout << "Failed at start of " << edge << std::endl;
				throw;
			}


			std::map<size_t, size_t> replaces;
			auto replacer = [&](Mesh2::EdgeIndex old, Mesh2::EdgeIndex nw, bool instant) {
				if (instant && _edgeGeometry[nw].size() == 0) {
					_edgeGeometry[nw] = std::move(_edgeGeometry[old]); //std::move(
					for (auto j : _edgeGeometry[nw]) {
						geometry[j].replace_edge(old, nw);
					}
				}
				if (replaces.count(nw) /*|| _edgeGeometry[old].size() == 0*/) return;
				auto theold = old;
				while (replaces.count(theold)) {
					auto preold = theold;
					theold = replaces[theold];
					replaces.erase(preold);
				}
				replaces[nw] = theold;// std::make_pair(std::move(_edgeGeometry[old]), old);
			};

			std::array<size_t, 3> s;
			if(g == _edgeGeometry[edge].end())
				s = _mesh.collapseEdge(edge, replacer, weight);
			else {
				auto ee = _mesh.edge(edge);
				auto p0 = geometry[*g].line.getParam(_mesh.point(ee.first));
				auto p1 = geometry[*g].line.getParam(_mesh.point(ee.second));
				auto ww = p0 * (1.0 - weight) + p1 * weight;
				auto wpoint = geometry[*g].line.sample(ww);
				if (!check_weight_intersection(_mesh.edge(edge).second, wpoint))
					return{};
				geometry[*g].collapse(edge, ww);
				_edgeGeometry[edge].clear();
				s = _mesh.collapseEdge(edge, replacer, wpoint);
			}
			bool collapsed = false;

			auto cur = replaces.begin();

			for (auto i = replaces.begin(); i != replaces.end(); ++i) {
				_edgeGeometry[i->first] = std::move(_edgeGeometry[i->second]); //std::move(
				for (auto j : _edgeGeometry[i->first]) {
					geometry[j].replace_edge(i->second, i->first);
				}
			}

			for (auto i : s) {
				if (i == Mesh2::NotAnEdge) continue;
				collapsed = true;
				_edgeGeometry.pop_back();
				result.push_back(i);
			}
			if (collapsed && _mesh.point_edge(pt).size()) {
				result.insert(result.end(), _mesh.point_edge(pt).begin(), _mesh.point_edge(pt).end());
			}
			if (_DebugTest(-100)) {
				std::cout << "Failed at end of " << edge << std::endl;
				throw;
			}


			if (_DebugTestIntersection(ep0, ets.first) || (ets.first != ets.second && _DebugTestIntersection(ep0, ets.second))) {
				throw "Ne Sovsem beda";
			}
			return std::move(result);
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