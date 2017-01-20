#pragma once
#include "stdafx.h"
#include "g_common_interface.h"
#include "g_geometry.h"
#include "g_scene.h"
#include "g_vertex.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT ILine : public Geometry {
	protected:
		GHANDLE _p0, _p1;
	public:
		ILine(Scene& context, const SETTINGHANDLE& setting, GHANDLE _p0, GHANDLE _p1) :
			Geometry(context, setting), _p0(_p0), _p1(_p1) {
			auto pp0 = dynamic_cast<Vertex*>(context.get_ptr(_p0));
			auto pp1 = dynamic_cast<Vertex*>(context.get_ptr(_p1));
			if (pp0 == nullptr || pp1 == nullptr) {
				throw std::runtime_error("Unable do define line. _p0 or _p1 is not Vertex");
			}
			if (pp0 == pp1)
				throw std::logic_error("Unable to define zero-length line");
		}
		ILine(ILine&& l) : _p0(l._p0), _p1(l._p1), Geometry(std::move(l)) {}
		virtual inline square_curve getCurve() const = 0;

		virtual inline GHANDLE createSame(Scene& context, const SETTINGHANDLE& setting, const std::vector<GHANDLE>& p) = 0;

		inline GHANDLE p0Handle() const { return _p0; }
		inline GHANDLE p1Handle() const { return _p1; }
		virtual vector3 P0() const {
			return p0ptr()->position();
		}
		virtual vector3 P1() const {
			return p1ptr()->position();
		}
		inline Vertex * p0ptr() const {
			return static_cast<Vertex*>(getConstContext().get_ptr(_p0));
		}
		inline Vertex * p1ptr() const {
			return static_cast<Vertex*>(getConstContext().get_ptr(_p1));
		}

		virtual bool pointCast(const vector3 &v)const = 0;
		virtual void sortAlong(std::vector<std::pair<vector3, GHANDLE>>& v) const = 0;
		inline void swapOrientation() {
			_p0 ^= _p1 ^= _p0 ^= _p1;
		}

		virtual GHANDLE init() { return getHandle(); }
		virtual void replace(GHANDLE o, GHANDLE n) {
			if (o == _p0) _p0 = n;
			if (o == _p1) _p1 = n;
		}
		virtual std::vector<GHANDLE> getChildren() const { return{ _p0, _p1 }; }
		virtual void applyTransform(const matrix4x4& m) = 0;
		virtual std::vector<GHANDLE> getBoundary() const { return{ _p0, _p1 }; }

		virtual vector3 sample(double t) const = 0;

		//virtual bool IsIncident(vector3 p) {
		//	return getDistance(ray(p + vector3(0, 0, 10.0), -vector3::Z())) < FG_EPS;
		//}
	protected:
		virtual void addSelfToContext() = 0;
	};

	class FEMCADGEOMSHARED_EXPORT LineSegment : public ILine, public IPrintable {
	public:
		// IPrintable
		virtual std::string toString() const {
			return "LineSegment {" + std::to_string(_p0) + ", " + std::to_string(_p1) + " }";
		}

		LineSegment(Scene& context, const SETTINGHANDLE& setting, GHANDLE _p0, GHANDLE _p1) :
			ILine(context, setting, _p0, _p1) {

			addSelfToContext();
		}
		LineSegment(LineSegment&& l) :
			ILine(std::move(l)) {}//, l._p0, l._p1) {}
		virtual inline GHANDLE createSame(Scene& context, const SETTINGHANDLE& setting, const std::vector<GHANDLE>& p) {
			try {
				return LineSegment(context, setting, p[0], p[1]).getHandle();
			}
			catch (...) { throw FGException("Failed to create same line"); }
		}


		virtual square_curve getCurve() const {
			return square_curve::line(P0(), getTangent());
		}
		virtual void sortAlong(std::vector<std::pair<vector3, GHANDLE>>& v) const {
			auto t = getTangent();
			auto p0 = P0();
			std::function<double(vector3)> f = [&](vector3 x) {return t & (x - p0); };
			std::sort(std::begin(v), std::end(v), [=](std::pair<vector3, GHANDLE> x, std::pair<vector3, GHANDLE> y)->bool {
#ifdef _DEBUG
				auto p = f(x.first);
				auto q = f(y.first);
				return p < q;
#else
				return f(x.first) < f(y.first);
#endif
			});
		}

		inline vector3 getTangent() const {
			return (P1() - P0());
		}
		inline vector3 getNormal() const {
			return (P1() - P0()) ^ vector3::Z();
		}

		virtual int classify(const vector3& p) const {
			vector3 pt0 = P0();
			vector3 pt1 = P1();
			vector3 n = (pt1 - pt0) ^ vector3::Z();
			double dot = (p - pt0) & n;
			if (dot > FG_EPS) return 1;
			if (dot < -FG_EPS) return -1;
			return 0;
		}
		virtual vector3 middle() const {
			return 0.5 * (P1() + P0());
		}

		virtual bool equals(const IGeometry& geom) const {
			const LineSegment* rhs = dynamic_cast<const LineSegment*>(&geom);
			return rhs && rhs->getConstContext() == getConstContext() &&
				(rhs->P0() == P0() && rhs->P1() == P1() || rhs->P0() == P1() && rhs->P1() == P0());
		}

		virtual GHANDLE copy() const {
			return copy(getConstContext());
		}
		virtual GHANDLE copy(Scene& context) const {
			auto rp0 = p0ptr()->copy(context);
			auto rp1 = p1ptr()->copy(context);
			//auto sett = this->getSetting();
			auto res = LineSegment(context, getSetting(), rp0, rp1);
			return res.getHandle();//context.add(res);
		}
		virtual inline void applyTransform(const matrix4x4& m) { }
		virtual inline rect getBoundingRect() const {
			return rect{ P0(), P1() };
		}
		bool isInPlane(const plane& p) const {
			//plane p(transformation.GetInversedTransform() * p_origin._p0, transformation.GetInversedTransform() & p_origin.n);
			return p0ptr()->isInPlane(p) && p1ptr()->isInPlane(p);
		}
		virtual double getDistance(const ray &v) const {
			//ray v(transformation.GetInversedTransform() * v_origin._p0, transformation.GetInversedTransform() & v_origin.l);
			vector3 ort;
			vector3 pt0 = P0();
			vector3 pt1 = P1();
			ort = (pt1 - pt0) ^ v.l;
			try {
				ort.normalize();
			}
			catch (std::exception&) {
				return p1ptr()->getDistance(v);
			}
			vector3 bnormal = ort ^ v.l;
			try {
				bnormal.normalize();
			}
			catch (std::exception&) {
				return 0;
			}
			vector3 p01 = (pt1 - pt0);
			p01 = (p01 & ort) * ort + (p01 & bnormal) * bnormal;
			vector3 pp0 = (v.p0 - pt0);
			pp0 = (pp0 & ort) * ort + (pp0 & bnormal) * bnormal;
			vector3 pp1 = (v.p0 - pt1);
			pp1 = (pp1 & ort) * ort + (pp1 & bnormal) * bnormal;

			if ((p01 & pp0) < 0)
				return p0ptr()->getDistance(v);
			if ((pp1 & p01) > 0)
				return p1ptr()->getDistance(v);
			return (ort & (pt0 - v.p0));
		}
		virtual bool pointCast(const vector3 &v)const {
#ifdef FAST_POINTCAST
			auto _p0 = P0(), _p1 = P1(), vv = (v - _p0);
			return (vv & (_p1 - _p0)) > -FG_EPS && ((v - _p1) & (_p0 - _p1)) > -FG_EPS && (getNormal() & vv) < FG_EPS_SQ;
#else
			auto _p0 = P0(); auto _p1 = P1();
			return std::fabs((v - _p0).length() + (v - _p1).length() - (_p1 - _p0).length()) < FG_EPS;
#endif
		}
		virtual vector3 sample(double t) const {
			return P0() * (1.0 - t) + P1() * t;
		}
	protected:
		virtual void addSelfToContext() {
			_handle = getContext().add(std::move(*this), { _p0, _p1 });
		}
	};

	class FEMCADGEOMSHARED_EXPORT EllipticSegment : public ILine, public IPrintable {
	protected:
		GHANDLE _center;
		square_curve _curve;
	public:
		EllipticSegment(Scene& context, const SETTINGHANDLE& setting, GHANDLE _p0, GHANDLE _p1, GHANDLE _center) :
			ILine(context, setting, _p0, _p1), _center(_center) {
			auto c = dynamic_cast<Vertex*>(context.get_ptr(_center));
			if (c == nullptr)
				throw std::runtime_error("Unalble to define CircleSegment. Center is not a point");
			auto r0 = (c->position() - p0ptr()->position()).lengthSq();
			auto r1 = (c->position() - p1ptr()->position()).lengthSq();
			if (std::fabs(r0 - r1) > FG_EPS) throw std::logic_error("Unable to define circle");
			_curve = square_curve::circle(r0, c->position());
			_curve.cache_parametrization(p0ptr()->position(), p1ptr()->position(), cached_matrix, cached_t0, cached_t1);
			addSelfToContext();
		}
		EllipticSegment(Scene& context, const SETTINGHANDLE& setting, GHANDLE _p0, GHANDLE _p1, GHANDLE _center, const square_curve& curve) :
			ILine(context, setting, _p0, _p1), _center(_center), _curve(curve) {
			auto c = dynamic_cast<Vertex*>(context.get_ptr(_center));
			if (c == nullptr)
				throw std::runtime_error("Unalble to define CircleSegment. Center is not a point");
			_curve.cache_parametrization(p0ptr()->position(), p1ptr()->position(), cached_matrix, cached_t0, cached_t1);
			addSelfToContext();
		}
		EllipticSegment(EllipticSegment&& l) :
			ILine(std::move(l)), _center(l._center), _curve(l._curve), 
			cached_matrix(l.cached_matrix), cached_t0(l.cached_t0), cached_t1(l.cached_t1) {
		}

		virtual inline GHANDLE createSame(Scene& context, const SETTINGHANDLE& setting, const std::vector<GHANDLE>& p) {
			try {
				return EllipticSegment(context, setting, p[0], p[1], p[2]).getHandle();
			}
			catch (...) { throw FGException("Failed to create same line"); }
		}

		virtual std::string toString() const {
			return "EllipticSebment {" + std::to_string(_p0) + ", " + std::to_string(_p1) + ", c:" + std::to_string(_center) + "}";
		}
		GHANDLE centerHandle() const {
			return _center;
		}
		Vertex * centerptr() const {
			return static_cast<Vertex*>(getConstContext().get_ptr(_center));
		}
		virtual vector3 Center() const {
			return centerptr()->position();
		}
		//double Radius() const {
		//	return 1.0;
		//	return (P0() - Center()).length();
		//}
		//double RadiusSq() const {
		//	return 1.0;
		//	return (P0() - Center()).lengthSq();
		//}

		inline vector3 getNormal() const {
			return (P1() - P0()) ^ vector3::Z();
		}
		square_curve getCurve() const {
			return _curve;
			//return _curve.getTransformed(getInversedTransform());
		}
		virtual bool pointCast(const vector3 &v)const {
			if (classify(v) != 0) return false;
			vector3 cp = Center();
			vector3 cp0 = P0() - cp;
			vector3 cp1 = P1() - cp;
			double sign = (cp0 ^ cp1).z;
			vector3 cv = v - cp;
			return (cv ^ cp0).z * sign < FG_EPS && sign * (cv ^ cp1).z > -FG_EPS;
			/*double cvcp0z = (cv ^ cp0).z;
			if (cvcp0z * sign < 0) {
			if ((cv ^ cp1).z * sign > 0) {
			return cv.length();
			}*/
		}

		virtual void sortAlong(std::vector<std::pair<vector3, GHANDLE>>& v) const {
			//auto t = getTangent();
			auto _p0{ P0() }, _p1{ P1() }, c{ Center() };
			double beg = std::atan2(_p0.y - c.y, _p0.x - c.x);
			double end = std::atan2(_p1.y - c.y, _p1.x - c.x);
			std::function<double(vector3)> f = [&](vector3 y) {
				double x = std::atan2(y.y - c.y, y.x - c.x);
				x = x - beg;
				return x >= PI ? x - 2 * PI : (x < -PI ? x + 2 * PI : x); };
			std::sort(std::begin(v), std::end(v), [=](std::pair<vector3, GHANDLE > x, std::pair<vector3, GHANDLE> y)->bool {
				return f(x.first) < f(y.first);
			});
		}

		virtual void replace(GHANDLE o, GHANDLE n) {
			ILine::replace(o, n);
			if (o == _center) _center = n;
		}
		virtual GHANDLE copy() const {
			return copy(getConstContext());
		}
		virtual GHANDLE copy(Scene& context) const {
			auto rp0 = p0ptr()->copy(context);
			auto rp1 = p1ptr()->copy(context);
			auto ctr = centerptr()->copy(context);
			auto res = EllipticSegment(context, getSetting(),
				/*context.addSetting(dynamic_cast<LineSetting&>(*this->context.get(setting))),*/ rp0, rp1, ctr, _curve);
			return res.getHandle();//context.add(res);
		}
		virtual std::vector<GHANDLE> getChildren() const { return{ _p0, _p1, _center }; }
		virtual inline rect getBoundingRect() const {
			auto pp0 = P0();
			auto pp1 = P1();
			try {
				return rect(pp0, pp1).add_point(getCurve().control_point(pp0, pp1));
			}catch(std::invalid_argument&){
				throw FGException("Invalid curve");
			}
		}
		virtual void applyTransform(const matrix4x4& m) {
			//ILine::applyTransform(m);
			//centerptr()->applyTransform(m);
			_curve = _curve.getTransformed(m);
		}
		virtual double getDistance(const ray &v) const {
			if (std::fabs(v.l.x) > FG_EPS || std::fabs(v.l.y) > FG_EPS) throw std::runtime_error("Unimplemented");
			vector3 cp = Center();
			vector3 cp0 = P0() - cp;
			vector3 cp1 = P1() - cp;
			double sign = (cp0 ^ cp1).z;
			vector3 cv = v.p0 - cp;
			double cvcp0z = (cv ^ cp0).z;
			if (cvcp0z * sign > 0) {
				vector3 cn = -0.5*(cp0 + cp1);
				if ((cn ^ cv).z * sign > 0) {
					return p0ptr()->getDistance(v);
				}
				else {
					return p1ptr()->getDistance(v);
				}
			}
			else {
				if ((cv ^ cp1).z * sign > 0) {
					return cv.length();
				}
				else {
					return p1ptr()->getDistance(v);
				}
			}
		}
		bool isInPlane(const plane& p) const {
			//plane p(getTransform().GetInversedTransform() * p_origin._p0, getTransform().GetInversedTransform() & p_origin.n);
			return p0ptr()->isInPlane(p) && p1ptr()->isInPlane(p) && centerptr()->isInPlane(p);
		}

		int classify(const vector3& pp) const {
			vector3 pt0 = P0();
			vector3 pt1 = P1();
			vector3 n = (pt1 - pt0) ^ vector3::Z();
			n.normalize();
			auto c = Center();
			double dot = (c - pt0) & n; // + внутрь
			auto curv = getCurve().classify(pp);
			auto ddot = n & (pp - pt0);
			//if (curv == 0 && ddot * dot < 0.0f) return 0; // on _curve
			if (ddot > FG_EPS) {
				if (dot > 0) return 1;
				else return curv;
			}
			if (ddot < -FG_EPS) {
				if (dot < 0) return -1;
				else return -curv;
			}
			if (curv >= 0) return 0;
			if (dot > 0) return 1;
			return -1;
			//return (dot > 0 ? -1 : 1) * (curv < 0 ? -1 : (curv == 0 ? (ddot > -FG_EPS ? 0 : -1) : (ddot < -FG_EPS ? -1 : (ddot > FG_EPS ? 1 : 0))));
		}
		virtual vector3 middle() const {
			return getCurve().get_point(0.5, P0(), P1());
			//return getTransform() * (0.5 * (localP0() + localP1()).getNormalized() * Radius());
		}
		virtual bool equals(const IGeometry& geom) const {
			const EllipticSegment* rhs = dynamic_cast<const EllipticSegment*>(&geom);
			return rhs && rhs->getConstContext() == getConstContext() && rhs->_center == _center &&
				(rhs->_p0 == _p0 && rhs->_p1 == _p1 || rhs->_p0 == _p1 && rhs->_p1 == _p0);
		}
		virtual vector3 sample(double t) const {
			return getCurve().get_point(t, cached_matrix, cached_t0, cached_t1);
		}
	protected:
		matrix4x4 cached_matrix;
		double cached_t0, cached_t1;
		virtual void addSelfToContext() {
			_handle = getContext().add(std::move(*this), getChildren());
		}
	};

}