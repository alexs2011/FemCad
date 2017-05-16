#pragma once
#include "g_meshing.h"

namespace fg {
	template<class S>
	class IElementSize
	{
	public:
		IElementSize() = default;
		IElementSize(const IElementSize&) = default;
		virtual S get_size(const vector3& point) const = 0;
		//virtual vector3 get_size_aniso(const vector3& point) const = 0;
	};

	template<class SizeType>
	class IElementSizeView : public IElementSize<SizeType> {
	public:
		virtual SizeType get_size(const vector3& p) const {
			return getSize(p, SizeType{});
		}
		virtual SizeType getSize(const vector3& p, SizeType x) const = 0;
	};

	class ElementSize : public IElementSizeView<double>, public IElementSizeView<vector3> {
	protected:
		std::shared_ptr<IElementSize<double>> _isoSize;
		std::shared_ptr<IElementSize<vector3>> _anisoSize;
	public:
		template<class S, class... Targs>
		void setIsoSize(Targs... args) {
			_isoSize = std::make_shared<S>(args... );
		}
		template<class S, class... Targs>
		void setAnisoSize(Targs... args) {
			_anisoSize = std::make_shared<S>(args...);
		}
		ElementSize(std::shared_ptr<IElementSize<double>> iso = nullptr, std::shared_ptr<IElementSize<vector3>> aniso = nullptr) : _isoSize{ iso }, _anisoSize{ aniso } {}


		virtual inline double getSize(const vector3& point, double) const {
			return _isoSize ? _isoSize->get_size(point) : (_anisoSize ? _anisoSize->get_size(point).length() : 1e100);
		}
		virtual inline vector3 getSize(const vector3& point, vector3) const {
			return _anisoSize ? _anisoSize->get_size(point) : vector3::Repeat(_isoSize ? _isoSize->get_size(point) : 1e100);
		}
	};

	template<class S>
	class LambdaElementSize : public IElementSize<S>
	{
		std::function<S(const vector3&)> _func;
	public:
		LambdaElementSize() = delete;
		LambdaElementSize(std::function<S(const vector3&)> f) : _func{ f }, IElementSize<S>() {};
		LambdaElementSize(const LambdaElementSize&) = default;
		virtual S get_size(const vector3& point) const {
			return _func(point);
		}
		//virtual vector3 get_size_aniso(const vector3& point) const = 0;
	};

	template<class S>
	class MeshElementSize : public IElementSize<S>
	{
	protected:
		const Mesh2& _mesh;
	public:
		MeshElementSize(const Mesh2& _mesh) : _mesh{ _mesh } {}
		MeshElementSize(const MeshElementSize&) = default;
		//MeshE
	};

	template<class S>
	class BoundaryElementSize : public IElementSize<S> {
		const ILine& l;
	public:
		BoundaryElementSize(const ILine& l) : l{ l } {}
		BoundaryElementSize(const BoundaryElementSize&) = default;

		// вычисляется размер элемента в точке point
		// возвращает примерную длину ребра на границе
		S get_size(const vector3& point) const override
		{
			auto t = l.getParam(point);

			auto N = size_t(l.getSetting()->getParameter<DoubleParameter>("N"));
			auto q = l.getSetting()->getParameter<DoubleParameter>("q");

			if (q == 1.0) {
				auto s0 = int(t * N) * 1.0 / N;
				auto s1 = s0 + 1.0 / N;
				return (l.sample(s1) - l.sample(s0)).length();
			}
			auto div = 1.0 / (std::pow(q, (double)N) - 1.0);
			double s1 = 0.0, s0 = 0.0;
			for (size_t i{}; i < N; ++i) {
				s0 = s1;
				s1 = (std::pow(q, (double)i) - 1) * div;
				if (s1 >= t) break;
			}
			return _eval<S>(l.sample(s1) - l.sample(s0));
		}
	private:
		template<class T>
		T _eval(const vector3&) const { throw; }
		template<>
		vector3 _eval<vector3>(const vector3& v) const { return v; }
		template<>
		double _eval<double>(const vector3& v) const { return v.length(); }
	};

	class MeshElementSizeIsoMaxEdgeLength : public MeshElementSize<double>
	{
	public:
		using MeshElementSize<double>::MeshElementSize;
		MeshElementSizeIsoMaxEdgeLength(const MeshElementSizeIsoMaxEdgeLength&) = default;

		/*virtual vector3 get_size_aniso(const vector3& point) const {
			return get_size(point);
		}*/

		// вычисляется размер элемента в точке point
		// возвращает взвешенный размер элемента на треугольнике
		double get_size(const vector3& point) const override
		{
			// индексы точек треугольника, на который попадает point
			std::tuple<size_t, size_t, size_t> verts;

			auto res = _mesh.cast(point, verts);
			return res & vector3{ max_edge(std::get<0>(verts)), max_edge(std::get<1>(verts)), max_edge(std::get<2>(verts)) };
		}

		// ищется максимальное ребро (хотя внутри ищется минимальное ребро, т.е. с наименьшей длиной)
		double max_edge(size_t vertex) const
		{
			double res = std::numeric_limits<double>::max();
			for (auto i : _mesh.point_edge(vertex))
				res = std::min(res, _mesh.edge_length_sq(i));
			return std::sqrt(res);
		}
	};

	template<class S>
	class ComplexElementSize : public IElementSize<S>
	{
	protected:
		std::vector<std::shared_ptr<IElementSize<S>>> collection;
	public:
		template<class It>
		ComplexElementSize(It _esBegin, It _esEnd) : collection(_esBegin, _esEnd) {}
		//virtual S get_size(const vector3& point) const = 0;
	};

	template<class S>
	class ComplexElementSizeMinimal : public ComplexElementSize<S>
	{
	public:
		template<class It>
		ComplexElementSizeMinimal(It _esBegin, It _esEnd) : ComplexElementSize<S>(_esBegin, _esEnd) {}
		virtual S get_size(const vector3& point) const {
			auto i = collection.begin();
			S min = (*i)->get_size(point);
			for (++i; i != collection.end(); ++i) {
				min = std::min(min, (*i)->get_size(point));
			}
			return min;
		}
	};

	typedef ComplexElementSizeMinimal<double> ComplexElementSizeMinimalIso;
	typedef ComplexElementSizeMinimal<vector3> ComplexElementSizeMinimalAniso;
}

