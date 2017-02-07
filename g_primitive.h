#pragma once
#include "stdafx.h"
#include "g_common_interface.h"
#include "g_geometry.h"
#include "g_scene.h"
#include "g_vertex.h"
#include "g_line.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT IPrimitive {
	public:
		virtual SETTINGHANDLE inside(const vector3 point) const = 0;
		virtual GHANDLE instantiate(Scene& s) const = 0;
		//virtual bool isConvex() const = 0;
	};

	class FEMCADGEOMSHARED_EXPORT PrimitiveBase : public Geometry, public IPrimitive {
	public:
		PrimitiveBase(Scene& s, const SETTINGHANDLE& setting) : Geometry(s, setting) {}
		PrimitiveBase(PrimitiveBase&& p) = default;
		template<class G>
		void for_each(std::function<void(G&)> pred) {
			std::set<G*> values;
			for (auto i : getGeometry()) {
				auto s = dynamic_cast<G*>(context.get_ptr(i));
				if (s) {
					values.insert(s);
				}
				for (auto j : getContext().get_ptr(i)->getChildren()) {
					auto p = dynamic_cast<G*>(context.get_ptr(j));
					if (p) {
						values.insert(p);
					}
				}
			}
			for (auto i : values) {
				pred(*i);
			}
		}
	};

	// Абстрактный класс, обобщающий замкнутые совокупности отрезков и кривых
	class FEMCADGEOMSHARED_EXPORT Primitive : public PrimitiveBase
	{
	protected:
		std::vector<GHANDLE> geometry;
		Primitive(Scene& s, const SETTINGHANDLE& setting) : PrimitiveBase(s, setting) { }
		Primitive(Scene& s, const SETTINGHANDLE& setting, std::vector<GHANDLE>& geom) : PrimitiveBase(s, setting), geometry(geom) { }
		Primitive(Primitive&& p) : geometry(std::move(p.geometry)), PrimitiveBase(std::move(p)) { }
	public:
		virtual std::vector<GHANDLE> getChildren() const { return geometry; }
		template<class T>
		T& get(int index) const {
			return getConstContext().get<T>(geometry[index]);
		}
		template<class T>
		T* get_ptr(int index) const {
			return dynamic_cast<T>(getConstContext().get_ptr(geometry[index]));
		}

	public:
		virtual std::vector<GHANDLE> getBoundary() const { return geometry; }
		virtual const std::vector<GHANDLE>& getConstBoundary() const { return geometry; }

		// ITransformable interface
	public:
		//virtual void ApplyTransform() { Transformation(). }
		//virtual TransformPtr Transformation() {return isInherited() ? getTransformation() : transformation;}
		//virtual const TransformPtr ConstTransformation() const {return isInherited() ? getTransformation() : transformation;}
		//virtual TransformPtr getParentTransform() const {return transformation.GetParent();}
		//virtual matrix4x4 get() const {return transformation.GetTransform();}
		//virtual matrix4x4 getInversed() const {return transformation.GetInversedTransform();}
		//virtual bool isInherited() const { return inherited; }
		//virtual void setParent(ITransformable& t) { transformation.SetParent(t.Transformation()); inherited = true;}
	};

	class FEMCADGEOMSHARED_EXPORT CompoundPrimitive : public PrimitiveBase {
	public:
		CompoundPrimitive(Scene& s)	: PrimitiveBase(s, s.defaultCSG) {}
		CompoundPrimitive(CompoundPrimitive&& c) = default;// : PrimitiveBase(std::move(c)) {}
		virtual vector3 middle() const { throw FGException("Unable to get middle from CompoundPrimitive object"); }
	public:
		virtual std::vector<GHANDLE> getGeometry() const = 0;
		virtual vector3 middle(GHANDLE subobject) const = 0;
	};



}
