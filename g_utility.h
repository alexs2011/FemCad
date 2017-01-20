#pragma once
#include "g_shape_2d.h"
#include "g_intersector.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT GeometryUtility {
		using Lookup = lookup_tree<2, GHANDLE>;
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
		static double getArea(const Scene& context, const std::vector<GHANDLE>& lines) {
			double sum = 0;
			for (auto i : lines) {
				ILine& l = context.get<ILine>(i);
				sum += (l.P0() ^ l.P1()).length();
			}
			return sum;
		}

		static GHANDLE ApplyCSG(Scene& context, CSGOperation op, const Primitive& primitive0, const Primitive& primitive1);
	};
}