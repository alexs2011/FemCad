#pragma once
#include "g_shape_2d.h"
#include "g_intersector.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT GeometryUtility {
		using Lookup = lookup_tree<2, ILine*>;
	public:
		static bool isConvex(const Scene& context, const std::vector<GHANDLE>& geometry) {
			int side = 0;
			for (size_t i = 0U; i < geometry.size(); i++) {
				const IClassifiable* c = static_cast<const IClassifiable*>(context.get_ptr(geometry[i]));
				for (size_t j = i + 1; j < geometry.size(); j++) {
					const IClassifiable* p = static_cast<const IClassifiable*>(context.get_ptr(geometry[j]));
					auto s = c->classify(p->middle());
					if (side == 0 && s != 0) { side = s; continue; }
					if (s != 0 && s != side) return false;
				}
			}
			return true;
		}
		static void subdivideLineSetting(const SETTINGHANDLE& base_setting, double t, SETTINGHANDLE& left, SETTINGHANDLE& right) {
			if (t < -FG_EPS || t > 1.0 + FG_EPS) throw FGException("Line setting subdivision problem. 't' is out of bounds.");
			if (t < FG_EPS) {
				left = nullptr; right = base_setting; return;
			}
			if (t > 1.0 - FG_EPS) {
				left = base_setting;  right = nullptr; return;
			}
			try {
				auto N = (int)base_setting->getParameter<DoubleParameter>("N");
				auto q = base_setting->getParameter<DoubleParameter>("q");

				auto n = std::fabs(q - 1.0) < FG_EPS ? std::max(1, (int)(t * N)) : std::max(1, (int)std::floor(std::log(t * (std::pow(q, (double)N) - 1) + 1) / std::log(q)));
				left = base_setting->copy();
				left->setParameter("N", DoubleParameter(n));
				right = base_setting->copy();
				right->setParameter("N", DoubleParameter(std::max(1, N - n)));
			}
			catch (const FGException& ex) {
				return;
			}
		}
		static double getArea(const Scene& context, const std::vector<GHANDLE>& lines) {
			double sum = 0;
			for (auto i : lines) {
				ILine& l = context.get<ILine>(i);
				sum += (l.P0() ^ l.P1()).length();
			}
			return sum;
		}

		static GHANDLE ApplyCSG(Scene& context, CSGOperation op, const Primitive& primitive0, const Primitive& primitive1, std::vector<GHANDLE>* v = nullptr);
	};
}