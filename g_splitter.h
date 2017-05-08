#pragma once

#include "g_intersector.h"

namespace fg {

	template<class Element>
	class Splitter : public IClassifiable {
		const Element& curve;
		double t0, t1;
	public:
		GHANDLE getHandle() const { return curve.getHandle(); }
		inline Splitter() = delete;
		inline Splitter(const Splitter& s) = default;
		inline Splitter(Splitter&& s) = default;
		inline Splitter& operator=(const Splitter& s) = default;
		inline Splitter(const Element& l, double t0 = 0.0, double t1 = 1.0) : curve(l), t0{ t0 }, t1{ t1 } { }
		// определяет сзади или спереди или на линии лежит точка
		inline virtual int classify(const vector3 & p) const { return curve.classify(p); }
		// определяет сзади или спереди или на сплиттере или на линии лежит точка
		inline virtual int classify_point(const vector3 & p) const { 
			auto par = curve.getParam(p);
			auto res = curve.classify(p); 
			if (res == 0) {
				return (t0 - FG_EPS <= par && par <= t1 + FG_EPS) ? 0 : 0x7FFFFFFF;
			}
			return res;
		}
		// определяет сзади или спереди или на линии лежит линия
		inline virtual ClassificationState classify(const Splitter<Element>& line) const {
			return Intersector<Element>::classify_dynamic(curve, line.curve);
		}
		inline virtual void intersect(const Splitter<Element>& line, std::vector<Splitter<Element>>& front, std::vector<Splitter<Element>>& back) {
			std::vector<vector3> _, pl1;
			auto res = Intersector<Element>::intersect_dynamic(_, pl1, curve, line.curve);
			std::set<double> params;
			params.insert(line.t0);
			params.insert(line.t1);
			for (auto i : pl1) {
				params.insert(line.curve.getParam(i));
			}
			for (auto i = params.lower_bound(line.t0); i != params.lower_bound(line.t1); ++i) {
				auto j = i; ++j;
				Splitter<Element> ns(line.curve, *i, *j);
				auto cf = classify(ns.middle());
				if (cf > 0) front.push_back(ns);
				else back.push_back(ns);
			}
		}

		virtual vector3 middle() const { return curve.sample((t0+t1)*0.5);	}
		inline constexpr bool special() const { return _special<Element>(); }
	protected:
		template<class Element> 
		inline bool _special() const { return false; }
		template<> inline bool _special<ILine>() const {
			try { dynamic_cast<const EllipticSegment&>(curve); return true; }
			catch (...) { return false; }
		}
	};

}