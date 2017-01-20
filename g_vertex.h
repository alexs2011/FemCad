#pragma once
#include "stdafx.h"
#include "g_common_interface.h"
#include "g_geometry.h"
#include "g_scene.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT Vertex : public Geometry, public IPrintable {
		vector3 _pos;
	public:
		Vertex& operator=(const vector3& p) noexcept {
			auto prepos = _pos;
			_pos = p;
			if (!update()) {
				_pos = prepos;
				//throw FGWarning("Unable to change point coordinate at " + static_cast<std::string>(_pos));
			}
			return *this;
		}
		Vertex(Scene& context, const SETTINGHANDLE& setting) :
			Geometry(context, setting) {
			addSelfToContext();
		}
		Vertex(Scene& context, const SETTINGHANDLE& setting, const vector3& _pos) :
			Geometry(context, setting),
			_pos(_pos) {
			addSelfToContext();
			//transformation = TransformPtr(nullptr);
		}
		Vertex(Vertex&& v) : _pos(v._pos), Geometry(std::move(v)) {}
		
		vector3 position() const {
			return _pos;
		}
		// IPrintable
		virtual std::string toString() const {
			return "";
			return std::string{ _pos };
		}

#pragma region IGeometry
	protected:
		virtual void addSelfToContext() {
			_handle = getContext().add(std::move(*this));
		}
	public:
		virtual GHANDLE init() { return getHandle(); }
		virtual void replace(GHANDLE, GHANDLE) {}
		virtual std::vector<GHANDLE> getChildren() const { return{}; }
		virtual std::vector<GHANDLE> getBoundary() const { return{}; }
		virtual GHANDLE copy() const { return copy(getConstContext()); }
		virtual GHANDLE copy(Scene& context) const {
			auto res = Vertex(context, getSetting(),
				/*context.addSetting(dynamic_cast<VertexSetting&>(*this->context.get(setting))),*/ _pos);
			return res.getHandle();//context.add(res);
		}
		
		virtual void applyTransform(const matrix4x4& m) { _pos = m * _pos; }
		virtual double getDistance(const ray &v) const {
			//ray v = v_origin;//(getInversedTransform() * v_origin.p0, getInversedTransform() & v_origin.l);
			return ((v.l & (_pos - v.p0)) * v.l - _pos + v.p0).length();
		}
		bool isInPlane(const plane& p) const {
			//plane p(getInversedTransform() * p_origin.p0, getInversedTransform() & p_origin.n);
			return !p.classify(_pos);
		}
		virtual inline rect getBoundingRect() const { return rect{ _pos }; }

		virtual bool equals(const IGeometry& geom) const {
			const Vertex* rhs = dynamic_cast<const Vertex*>(&geom);
			return rhs && rhs->getConstContext() == getConstContext() && rhs->position() == position();
		}
#pragma endregion
		// IClassifiable interface
	public:
		virtual int classify(const vector3 &p) const { return (p == position()) ? 0 : 1; }
		virtual vector3 middle() const { return position(); }
	};
}