#pragma once
#include "g_element_size.h"

namespace fg {
	enum class CriterionResult {
		Fit = 0x0,
		Long = 0x1,
		Short = 0x2
	};
	class ICriterion {
	protected:
		const Mesh2& base;
	public:
		ICriterion(const Mesh2& _mesh) : base{ _mesh } {}
		virtual CriterionResult get(Mesh2::EdgeIndex, const IElementSize<double>&) = 0;
		virtual CriterionResult get(Mesh2::EdgeIndex, const IElementSize<vector3>&) = 0;
		virtual double get_error(Mesh2::EdgeIndex, const IElementSize<double>&) = 0;
		virtual double get_error(Mesh2::EdgeIndex, const IElementSize<vector3>&) = 0;
	};

	class OnePointCriterion : public ICriterion {
	public:
		using ICriterion::ICriterion;
		virtual CriterionResult get(Mesh2::EdgeIndex edge, const IElementSize<double>& s) override {
			// берем середину отрезка и на основе него вычисляем размер элемента ???
			auto size = s.get_size(base.sample_edge(edge, 0.5));
			double l = base.edge_length(edge);
			// относительная погрешность
			if (size > 1e38) return CriterionResult::Fit;
			double error = (l / size - 1.0);
			return (error > 1e-2) ? 
				CriterionResult::Long : 
				((error < -0.6) ? 
					CriterionResult::Short : 
					CriterionResult::Fit);
		}
		virtual CriterionResult get(Mesh2::EdgeIndex, const IElementSize<vector3>&) override {
			// [TODO] vector criterion
			return CriterionResult::Fit;
		}
		virtual double get_error(Mesh2::EdgeIndex edge, const IElementSize<double>& s) {
			auto size = s.get_size(base.sample_edge(edge, 0.5));
			register double l = base.edge_length(edge);
			if (size > 1e38) return 0.0;
			return (l - size) / size;
		}
		virtual double get_error(Mesh2::EdgeIndex, const IElementSize<vector3>&) {
			return 0.0;
		}
	};
	/*enum class CriterionType {
		OnePoint = 0x1
	};
	class CriterionFabric {
	public:
		ICriterion&& create(CriterionType type, const Mesh2& m) {
			switch (type)
			{
			case CriterionType::OnePoint:
				return std::move(OnePointCriterion(m));
			default:
				break;
			}
		}
	};*/
}