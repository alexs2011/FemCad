#pragma once
#include "g_meshing.h"

namespace fg {
	template<class S>
	class IElementSize {
	public:
		virtual S get_size(const vector3& point) const = 0;
		//virtual vector3 get_size_aniso(const vector3& point) const = 0;
	};

	template<class S>
	class MeshElementSize : public IElementSize<S>{
	protected:
		const Mesh2& mesh;
	public:
		MeshElementSize(const Mesh2& mesh) : mesh{ mesh } {}
		//MeshE
	};

	class MeshElementSizeIsoMaxEdgeLength : public MeshElementSize<double> {
	public:
		using MeshElementSize<double>::MeshElementSize;
		virtual vector3 get_size_aniso(const vector3& point) const {
			return get_size(point);
		}
		double get_size(const vector3& point) const override {
			std::tuple<size_t, size_t, size_t> verts;
			auto res = mesh.cast(point, verts);
			return res & vector3{ max_edge(std::get<0>(verts)), max_edge(std::get<0>(verts)), max_edge(std::get<0>(verts)) };
		}
		double max_edge(size_t vertex) const {
			double res = 0.0;
			for (auto i : mesh.point_edge(vertex)) {
				res = std::min(res, mesh.edge_length_sq(i));
			}
			return std::sqrt(res);
		}
	};
}

