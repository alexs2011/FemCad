#pragma once
#include "g_utility.h"

namespace fg {
	namespace primitive {
		class FEMCADGEOMSHARED_EXPORT Cube2D : public Primitive {
		protected:
			vector3 _middle;
			// IClassifiable interface
		public:
			virtual GHANDLE init() { return getHandle(); }
			Cube2D(Cube2D&& p) : Primitive(std::move(p)), _middle(p._middle) {}
			Cube2D(Scene& s, const SETTINGHANDLE& setting) :
				Primitive(s, setting) {
				Vertex v0(s, s.defaultVertex, vector3(0.0, 0.0, 0.0));
				Vertex v1(s, s.defaultVertex, vector3(1.0, 0.0, 0.0));
				Vertex v2(s, s.defaultVertex, vector3(0.0, 1.0, 0.0));
				Vertex v3(s, s.defaultVertex, vector3(1.0, 1.0, 0.0));

				_middle = vector3(0.5, 0.5, 0.0);

				LineSegment l0{ s, s.defaultLine, v0.getHandle(), v1.getHandle() };
				LineSegment l1{ s, s.defaultLine, v1.getHandle(), v3.getHandle() };
				LineSegment l2{ s, s.defaultLine, v3.getHandle(), v2.getHandle() };
				LineSegment l3{ s, s.defaultLine, v2.getHandle(), v0.getHandle() };

				geometry.insert(geometry.end(), { l0.getHandle(), l1.getHandle(), l2.getHandle(), l3.getHandle() });
				addSelfToContext();
			}
			virtual GHANDLE instantiate(Scene& s) const {
				return copy(s);
			}
			virtual void applyTransform(const matrix4x4& m) {
				std::set<GHANDLE> points;
				_middle = m * _middle;
				for (auto i : geometry) {
					auto& l = getContext().get<ILine>(i);
					points.insert({ l.p0Handle(), l.p1Handle() });
					l.applyTransform(m);
				}
				for (auto i : geometry) {
					auto& g = getContext().get<IGeometry>(i);
					g.applyTransform(m);
				}
			}
			virtual bool isConvex() const { return true; }

			virtual int classify(const vector3 &pp) const {
				return 1;
			}
			virtual vector3 middle() const {
				return _middle;
			}

			// IGeometry interface
		protected:
			virtual void addSelfToContext() {
				_handle = getContext().add(std::move(*this), geometry);
			}

		public:
			virtual void replace(GHANDLE o, GHANDLE n) {
				for (auto& i : geometry) {
					if (i == o) i = n;
					else getContext().get_ptr(i)->replace(o, n);
				}
			}
			virtual GHANDLE copy() const {
				return copy(getConstContext());
			}
			virtual GHANDLE copy(Scene &context) const {
				//[TODO]
				return Cube2D(context, getSetting()).getHandle();
			}
			virtual double getDistance(const ray &v) const {
				//[TODO]
				return 1.0; // FAKE!
			}
			virtual bool isInPlane(const plane &p) const {
				//[TODO]
				return true; // FAKE
			}
			virtual bool equals(const IGeometry &geom) const {
				const Cube2D* g = dynamic_cast<const Cube2D*>(&geom);
				return false; //g && g->getConstContext() == getConstContext() && position == g->position && size == g->size;
			}
			virtual std::vector<GHANDLE> getGeometry() const { return geometry; }
			virtual inline rect getBoundingRect() const {
				return rect{};
			}

			// IPrimitive interface
		public:
			virtual SETTINGHANDLE inside(const vector3 point) const {
				if (classify(point) < 0) return setting;
				return NoSetting;
			}
		};
	}
}