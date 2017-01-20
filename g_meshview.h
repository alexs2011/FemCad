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
			Mesh2::EdgeIndex get(double t) const { return edges.lower_bound(t)->second; }
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
		};
		Mesh2 mesh;
		std::vector<_meshLineView> geometry;
	public:
		/*const */Mesh2& Mesh() /*const */{ return mesh; }
		MeshView2d(const RectView& r) : mesh(r.mesh()) {
			std::map<Mesh2::Edge, Mesh2::EdgeIndex> edges;
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
						geometry.back().force_add(k == 0 ? 0.0 : ps[k-1], edges[std::make_pair(p, pnext)]);
						p = pnext;
					}
				}
			}
		}

	};
}