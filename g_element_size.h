#pragma once
#include "g_meshing.h"

namespace fg {
	template<class S>
	class IElementSize {
	public:
		IElementSize() = default;
		IElementSize(const IElementSize&) = default;
		virtual S get_size(const vector3& point) const = 0;
		//virtual vector3 get_size_aniso(const vector3& point) const = 0;
	};

	template<class S>
	class MeshElementSize : public IElementSize<S>{
	protected:
		const Mesh2& _mesh;
	public:
		MeshElementSize(const Mesh2& _mesh) : _mesh{ _mesh } {}
		MeshElementSize(const MeshElementSize&) = default;
		//MeshE
	};

	class MeshElementSizeIsoMaxEdgeLength : public MeshElementSize<double> {
	public:
		using MeshElementSize<double>::MeshElementSize;
		MeshElementSizeIsoMaxEdgeLength(const MeshElementSizeIsoMaxEdgeLength&) = default;
		virtual vector3 get_size_aniso(const vector3& point) const {
			return get_size(point);
		}
		double get_size(const vector3& point) const override {
			std::tuple<size_t, size_t, size_t> verts;
			auto res = _mesh.cast(point, verts);
			return res & vector3{ max_edge(std::get<0>(verts)), max_edge(std::get<1>(verts)), max_edge(std::get<2>(verts)) };
		}
		// ищется максимальное ребро
		double max_edge(size_t vertex) const {
			double res = std::numeric_limits<double>::max();
			for (auto i : _mesh.point_edge(vertex)) {
				res = std::min(res, _mesh.edge_length_sq(i));
			}
			return std::sqrt(res);
		}
	};
	template<class S>
	class ComplexElementSize : public IElementSize<S> {
	protected:
		std::vector<IElementSize<S>> collection;
	public:
		template<class It>
		ComplexElementSize(It _esBegin, It _esEnd) : collection(_esBegin, _esEnd){}
		virtual S get_size(const vector3& point) const = 0;
	};
	template<class S>
	class ComplexElementSizeMinimal : public ComplexElementSize<S> {
	public:
		template<class It>
		ComplexElementSizeMinimal(It _esBegin, It _esEnd) : ComplexElementSize<S>(_esBegin, _esEnd) {}
		virtual S get_size(const vector3& point) const {
			return std::min(collection.begin(), collection.end(), [&](const IElementSize<S>& e) { return e(point); });
		}
	};
	typedef ComplexElementSizeMinimal<double> ComplexElementSizeMinimalIso;
	typedef ComplexElementSizeMinimal<vector3> ComplexElementSizeMinimalAniso;
}

