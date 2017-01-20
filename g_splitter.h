#pragma once

#include "g_intersector.h"

namespace fg {

	template<class Element>
	class Splitter : public IClassifiable {
		const Element& curve;
	public:
		GHANDLE getHandle() const { return curve.getHandle(); }
		inline Splitter() = delete;
		inline Splitter(const Splitter& s) = default;
		inline Splitter(Splitter&& s) = default;
		inline Splitter& operator=(const Splitter& s) = default;
		inline Splitter(const Element& l) : curve(l) { }
		inline virtual int classify(const vector3 & p) const { return curve.classify(p); }
		inline virtual ClassificationState classify(const Element& line) const {
			return Intersector<Element>::classify_dynamic(curve, line);
		}

		virtual vector3 middle() const { return curve.middle();	}
		inline constexpr bool special() const { return _special<Element>(); }
	protected:
		template<class Element> inline bool _special() const { return false; }
		template<> inline bool _special<ILine>() const {
			try { dynamic_cast<const EllipticSegment&>(curve); return true; }
			catch (...) { return false; }
		}
	};

}