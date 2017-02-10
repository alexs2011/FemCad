#pragma once
#include "g_element_size.h"

namespace fg {
	class ICriterion {
	protected:
		Mesh2& base;
	public:
		ICriterion(Mesh2& mesh) : base{ mesh } {}
		virtual bool operator()(Mesh2::EdgeIndex, const IElementSize<double>&) = 0;
		virtual bool operator()(Mesh2::EdgeIndex, const IElementSize<vector3>&) = 0;
	};

	class OnePointCriterion : public ICriterion {
	public:
		using ICriterion::ICriterion;
		virtual bool operator()(Mesh2::EdgeIndex edge, const IElementSize<double>& s) override {
			auto size = s.get_size(base.sample_edge(edge, 0.5));
			return (base.edge_length(edge) > size);
		}
		virtual bool operator()(Mesh2::EdgeIndex, const IElementSize<vector3>&) override {
			return false;
		}

	};
}