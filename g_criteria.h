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
	};

	class OnePointCriterion : public ICriterion {
	public:
		using ICriterion::ICriterion;
		virtual CriterionResult get(Mesh2::EdgeIndex edge, const IElementSize<double>& s) override {
			auto size = s.get_size(base.sample_edge(edge, 0.5));
			return (base.edge_length(edge) > size) ? CriterionResult::Long : CriterionResult::Fit;
		}
		virtual CriterionResult get(Mesh2::EdgeIndex, const IElementSize<vector3>&) override {
			// [TODO] vector criterion
			return CriterionResult::Fit;
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