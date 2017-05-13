#pragma once

#define _USE_MATH_DEFINES

#include "stdafx.h"
#include "fg_exception.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

constexpr double PI = 3.1415926535897932384626433832795;

#define FG_EPS 1e-7
#define FG_EPS_SQ 1e-14


namespace fg {

	const double DEG_TO_RAD = std::acos(0.0) / 90.0;
	const double RAD_TO_DEG = 90.0 / std::acos(0.0);

	enum axis {
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2,
		AXIS_W = 3
	};

	bool equals_eps(const double a, const double b);

	class FEMCADGEOMSHARED_EXPORT vector3 {
	public:
		union {
			struct {
				double x, y, z;
			};
			double data[3];
		};

		constexpr vector3() : x(0.0), y(0.0), z(0.0) {}
		constexpr vector3(const vector3& v) : x(v.x), y(v.y), z(v.z) {}
		constexpr vector3(double all) : x(all), y(all), z(all) {}
		constexpr vector3(double x, double y, double z = 0.0) : x{ x }, y{ y }, z{ z } {}
		inline constexpr double operator[](int a) const {
			return data[a];
		}
		inline double& operator[](int a) {
			return data[a];
		}
		inline vector3 operator -() const {
			return vector3(-x, -y, -z);
		}
		inline vector3 operator +() const {
			return *this;
		}
		inline friend vector3 operator+(const vector3& lhs, const vector3& rhs) {
			return vector3(lhs.x + rhs.x,
				lhs.y + rhs.y,
				lhs.z + rhs.z);
		}
		inline friend vector3 operator-(const vector3& lhs, const vector3& rhs) {
			return vector3(lhs.x - rhs.x,
				lhs.y - rhs.y,
				lhs.z - rhs.z);
		}
		inline friend vector3 operator*(const vector3& lhs, const vector3& rhs) {
			return vector3(lhs.x * rhs.x,
				lhs.y * rhs.y,
				lhs.z * rhs.z);
		}
		inline friend vector3 operator/(const vector3& lhs, const vector3& rhs) {
			return vector3(lhs.x / rhs.x,
				lhs.y / rhs.y,
				lhs.z / rhs.z);
		}
		//scalar multiplication (dot product)
		inline friend double operator&(const vector3& lhs, const vector3& rhs) {
			return (lhs.x * rhs.x +
				lhs.y * rhs.y +
				lhs.z * rhs.z);
		}
		//cross product
		inline friend vector3 operator^(const vector3& lhs, const vector3& rhs) {
			return vector3(lhs.y * rhs.z - lhs.z * rhs.y,
				lhs.z * rhs.x - lhs.x * rhs.z,
				lhs.x * rhs.y - lhs.y * rhs.x);
		}
		inline double length() const {
			return sqrt(x*x + y*y + z*z);
		}
		inline double lengthSq() const {
			return x*x + y*y + z*z;
		}

		inline vector3 normalToPoint(const vector3& direction, const vector3& point) {
			auto d = direction.getNormalized();
			auto p = point - *this;
			return p - (p&d)*d;
		}

		inline vector3 getNormalized() const {// throw(FGException) {
			double l = length();
			if (l < FG_EPS) throw FGException("Failed to normalize 0-length vector");
			l = 1 / l;
			return l * *this;
		}
		inline vector3& normalize() {//throw(FGException) {
			double l = length();
			if (l < FG_EPS) throw FGException("Failed to normalize 0-length vector");
			l = 1 / l;
			x *= l;
			y *= l;
			z *= l;
			return *this;
		}

		vector3& operator =(const vector3& v) {
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		inline friend bool operator ==(const vector3& lhs, const vector3& rhs) {
			return (std::fabs(lhs.x - rhs.x) < FG_EPS) &&
				(std::fabs(lhs.y - rhs.y) < FG_EPS) &&
				(std::fabs(lhs.z - rhs.z) < FG_EPS);
		}
		inline friend bool operator !=(const vector3& lhs, const vector3& rhs) {
			return !(lhs == rhs);
		}
		inline friend bool operator <(const vector3& lhs, const vector3& rhs) {
			return (lhs.x < rhs.x - FG_EPS) ? true :
				(std::fabs(lhs.x - rhs.x) < FG_EPS ? ((lhs.y < rhs.y - FG_EPS) ? true :
					std::fabs(lhs.y - rhs.y) < FG_EPS ? (lhs.z < rhs.z - FG_EPS) : false) : false);
		}
		inline friend bool operator >(const vector3& lhs, const vector3& rhs) {
			return rhs < lhs;
		}
		inline friend bool operator <=(const vector3& lhs, const vector3& rhs) {
			return !(lhs > rhs);
		}
		inline friend bool operator >=(const vector3& lhs, const vector3& rhs) {
			return !(lhs < rhs);
		}

		friend std::ostream& operator<<(std::ostream& s, const vector3& v) {
			s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
			return s;
		}

		operator std::string() const {
			std::stringstream s;
			s << *this;
			return std::string(s.str());
		}

		static vector3 X() {
			return vector3(1.0, 0.0, 0.0);
		}
		static vector3 Y() {
			return vector3(0.0, 1.0, 0.0);
		}
		static vector3 Z() {
			return vector3(0.0, 0.0, 1.0);
		}
		static vector3 Nan() {
			return vector3(std::nan(""), std::nan(""), std::nan(""));
		}
		static vector3 Inf() {
			return vector3(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
		}
		static vector3 Repeat(double v) {
			return vector3{ v,v,v };
		}
		inline bool isNan() const {
			return std::isnan(x) || std::isnan(y) || std::isnan(z);
		}
		inline bool isInf() const {
			return std::isinf(x) || std::isinf(y) || std::isinf(z);
		}
	};


	class FEMCADGEOMSHARED_EXPORT quaternion {
	public:
		double w;
		vector3 v;
		quaternion() : v() { w = 1.0; }
		quaternion(double w, double x, double y, double z) : w(w), v(x, y, z) {}
		quaternion(double w, vector3 v) : w(w), v(v) {}
		quaternion(double w) : w(w), v() {}
		quaternion(const quaternion& q) : w(q.w), v(q.v) {}
		quaternion& operator =(const quaternion& q) {
			w = (q.w);
			v = (q.v);
			return *this;
		}
		quaternion get_unit() const {
			quaternion r;
			double q = 1.0 / (w*w + v.lengthSq());
			r.w = w * q;
			r.v = v * q;
			return r;
		}
		friend quaternion operator*(const quaternion& lhs, const quaternion& rhs) {
			quaternion r;
			r.w = lhs.w * rhs.w + (lhs.v & rhs.v);
			r.v = lhs.w * rhs.v + rhs.w * lhs.v + (lhs.v ^ rhs.v);
			return r;
		}
		quaternion get_inversed()const {
			quaternion r;
			double q = 1.0 / (w*w + v.lengthSq());
			r.w = w * q;
			r.v = -v * q;
			return r;
		}
		friend vector3 operator*(const quaternion& lhs, const vector3& rhs) {
			auto q = lhs * quaternion(1.0, rhs) * lhs.get_inversed();
			return q.v;
		}
		static quaternion rotate_by_axis(double angle, const vector3& axis) {
			return quaternion(std::cos(angle*0.5), axis * std::sin(angle*0.5));
		}
		static quaternion get_by_rotation(double rotx, double roty, double rotz) {
			//        auto rx = rotate_by_axis(rotx, vector3::X());
			//        auto ry = rotate_by_axis(roty, vector3::Y());
			//        auto rz = rotate_by_axis(rotz, vector3::Z());

			//        auto rxy = rx * ry;
			//        auto rxyz = rxy * rz;
			return rotate_by_axis(rotx, vector3::X()) *
				rotate_by_axis(roty, vector3::Y()) *
				rotate_by_axis(rotz, vector3::Z());
		}
	};

	template<int D>
	class plane_n {
		vector3 normal;
		double d;
		vector3 locals[D - 1];
	public:
		plane_n() : normal{}, d{} {}
		plane_n(const vector3& n, double d) : normal{ n.getNormalized() }, d{ d } {
			evaluate_locals<D>();
		}
		plane_n(const vector3& n, const vector3& p) : normal{ n.getNormalized() } {
			d = { -(normal&p) }
			evaluate_locals<D>();
		}
		plane_n(const plane_n<D>& n) = default;
		plane_n<D>& operator=(const plane_n<D>& n) = default;

		inline int classify(const vector3& n) const {
			auto p = n & normal;
			return (p > -d - FG_EPS) ? (p < d + FG_EPS ? 0 : 1) : -1;
		}
		template<int D>
		inline void evaluate_locals() {
		}
		template<>
		inline void evaluate_locals<3>() {
			vector3 x{ 1.0 - normal.x * normal.x, -normal.x * normal.y, -normal.x * normal.z };
			vector3 y{ -normal.y * normal.x, 1.0 - normal.y * normal.y, -normal.y * normal.z };
			vector3 z{ -normal.z * normal.x, -normal.z * normal.y, 1.0 - normal.z * normal.z };
			double xx = x.lengthSq();
			double yy = y.lengthSq();
			double zz = z.lengthSq();
			if (xx > yy) {
				if (xx > zz) {
					locals[0] = x / std::sqrt(xx);
				}
				else {
					locals[0] = z / std::sqrt(zz);
				}
			}
			else {
				if (yy > zz) {
					locals[0] = y / std::sqrt(yy);
				}
				else {
					locals[0] = z / std::sqrt(zz);
				}
			}
			locals[1] = normal ^ locals[0];
		}

		template<>
		inline void evaluate_locals<2>() {
			locals[0] = vector3{ -normal.y, normal.x };
		}
		template<int N>
		inline plane_n<N - 1> intersect(const plane_n<N>& p) {

		}
		template<>
		inline plane_n<2> intersect<3>(const plane_n<3>& p) {
			vector3 t = normal ^ p.normal;
			if (t.lengthSq() < FG_EPS_SQ) throw;
			//t = {-t & locals[1], t & locals[0], 0}

			// [TODO] Find p

			// p = {p & locals[0], p & locals[1], 0}
			// d = -t & p;

		}
	};

	class FEMCADGEOMSHARED_EXPORT plane {
	public:
		vector3 p0;
		vector3 n;
		plane() : p0(), n() {}
		plane(const plane& l) : p0(l.p0), n(l.n) {}
		plane& operator=(const plane&) = default;
		plane(const vector3& p0, const vector3& n) : p0(p0), n(n) {
			if (n == 0) throw std::logic_error("0-length normal vector");
		}
		inline static plane byThreePonts(const vector3& p0, const vector3& p1, const vector3& p2) {
			vector3 n;
			try {
				n = ((p1 - p0) ^ (p2 - p0)).getNormalized();
			}
			catch (std::exception&) {
				throw std::logic_error("Points are on one line");
			}

			return plane(p0, n);
		}
		friend bool operator==(const plane& lhs, const plane& rhs) {
			return lhs.n == rhs.n && lhs.classify(rhs.p0) == 0;
		}
		friend bool operator!=(const plane& lhs, const plane& rhs) {
			return lhs.n != rhs.n || lhs.classify(rhs.p0);
		}

		inline static plane byTwoPointsAndNormal(const vector3& p0, const vector3& p1, const vector3& n) {
			plane result;
			try {
				result = byThreePonts(p0, p0 + n, p1);
			}
			catch (std::logic_error&) {
				throw std::logic_error("Points are same");
			}
			return result;
		}

		// определе¤ет, с какой стороны плоскости находитс¤ точка
		// 0 - на плоскости, 1 - спереди, -1 - сзади
		inline int classify(const vector3& p) const {
			double test = (p - p0)&n;
			if (test < -FG_EPS) return -1;
			if (test > FG_EPS) return 1;
			return 0;
		}
		friend std::ostream& operator<<(std::ostream& s, const plane& p) {
			s << "Point: " << p.p0 << " Normal: " << p.n;

			return s;
		}
	};

	class FEMCADGEOMSHARED_EXPORT matrix4x4 {
	public:
		union {
			struct {
				double m00;
				double m01;
				double m02;
				double m03;
				double m10;
				double m11;
				double m12;
				double m13;
				double m20;
				double m21;
				double m22;
				double m23;
				double m30;
				double m31;
				double m32;
				double m33;
			};
			struct {
				double data[4][4];
			};
			double rowdata[16];
		};
		matrix4x4() {
			memset(rowdata, 0, sizeof(double) * 16);
			//m00 = 0.0;
			//m01 = 0.0;
			//m02 = 0.0;
			//m03 = 0.0;
			//m10 = 0.0;
			//m11 = 0.0;
			//m12 = 0.0;
			//m13 = 0.0;
			//m20 = 0.0;
			//m21 = 0.0;
			//m22 = 0.0;
			//m23 = 0.0;
			//m30 = 0.0;
			//m31 = 0.0;
			//m32 = 0.0;
			//m33 = 0.0;
		}
		matrix4x4(const matrix4x4& m) {
			for (int i = 0; i < 16; i++)
				rowdata[i] = m.rowdata[i];
		}
		matrix4x4& operator =(const matrix4x4& m) {
			for (int i = 0; i < 16; i++)
				rowdata[i] = m.rowdata[i];
			return *this;
		}

		inline friend matrix4x4 operator +(const matrix4x4& lhs, const matrix4x4& rhs) {
			matrix4x4 res;
			for (int i = 0; i < 16; i++)
				res.rowdata[i] = lhs.rowdata[i] + rhs.rowdata[i];
			return res;
		}
		inline friend matrix4x4 operator -(const matrix4x4& lhs, const matrix4x4& rhs) {
			matrix4x4 res;
			for (int i = 0; i < 16; i++)
				res.rowdata[i] = lhs.rowdata[i] - rhs.rowdata[i];
			return res;
		}
		inline friend matrix4x4 operator *(const matrix4x4& lhs, const matrix4x4& rhs) {
			matrix4x4 res;
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++) {
					register double r = 0.0;
					r += lhs.data[i][0] * rhs.data[0][j];
					r += lhs.data[i][1] * rhs.data[1][j];
					r += lhs.data[i][2] * rhs.data[2][j];
					r += lhs.data[i][3] * rhs.data[3][j];
					//for(int k = 0; k< 4; k++)
					//    res.data[i][j] += lhs.data[i][k] * rhs.data[k][j];
					res.data[i][j] = r;
				}
			return res;
		}
		inline friend vector3 operator *(const matrix4x4& lhs, const vector3& rhs) {
			vector3 res{};
			double w = 0.0;

			for (int i = 0; i < 3; i++) {
				double p = lhs.data[i][3] + lhs.data[i][0] * rhs.data[0] +
					lhs.data[i][1] * rhs.data[1] + lhs.data[i][2] * rhs.data[2];
				res.data[i] = p;
				w = rhs.x * lhs.m30 + rhs.y * lhs.m31 + rhs.z * lhs.m32 + lhs.m33;
			}
			if (std::fabs(w) < FG_EPS) {
				if (res.lengthSq() < FG_EPS_SQ) return vector3{};
				throw FGException("Singular transformation");
			}
			return res / w;
		}
		inline friend vector3 operator &(const matrix4x4& lhs, const vector3& rhs) {
			vector3 res;

			for (int i = 0; i < 3; i++) {
				double p = lhs.data[i][0] * rhs.data[0] +
					lhs.data[i][1] * rhs.data[1] + lhs.data[i][2] * rhs.data[2];
				res.data[i] = p;
			}

			return res;
		}
		inline friend vector3 operator *(const vector3& rhs, const matrix4x4& lhs) {
			vector3 res;
			double w;

			for (int i = 0; i < 3; i++) {
				res.data[i] = lhs.data[3][i];
				for (int j = 0; j < 3; j++)
					res.data[i] += lhs.data[j][i] * rhs.data[j];
				w = rhs.x * lhs.m03 + rhs.y * lhs.m13 + rhs.z * lhs.m23 + lhs.m33;
			}
			if (std::fabs(w) < FG_EPS) throw FGException("Singular transformation");

			return res / (w*lhs.determinant());
		}

		double determinant() const {
			return m00 * (m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 - m13 * m22 * m31 -
				m11 * m23 * m32 - m12 * m21 * m33) -
				m01 * (m10 * m22 * m33 + m12 * m23 * m30 + m13 * m20 * m32 - m13 * m22 * m30 -
					m10 * m23 * m32 - m12 * m20 * m33) +
				m02 * (m10 * m21 * m33 + m11 * m23 * m30 + m13 * m20 * m31 - m13 * m21 * m30 -
					m10 * m23 * m31 - m11 * m20 * m33) -
				m03 * (m10 * m21 * m32 + m11 * m22 * m30 + m12 * m20 * m31 - m12 * m21 * m30 -
					m10 * m22 * m31 - m11 * m20 * m32);
		}

		inline matrix4x4 get_inversed() const
		{
			matrix4x4 result;
			double det = 1.0 / determinant();
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					result.data[i][j] = minor(j, i) * det;//data[j][i] / det;
				}
			}
			return result;
		}

		inline double minor(size_t ii, size_t jj)const {
			size_t i_ind[3];
			size_t j_ind[3];
			size_t p{}, q{};
			for (size_t i = 0U; i < 4U; i++) {
				if (i != ii) i_ind[p++] = i;
				if (i != jj) j_ind[q++] = i;
			}
			return ((ii + jj) % 2 ? -1 : 1) *
				(data[i_ind[0]][j_ind[0]] * data[i_ind[1]][j_ind[1]] * data[i_ind[2]][j_ind[2]] +
					data[i_ind[0]][j_ind[1]] * data[i_ind[1]][j_ind[2]] * data[i_ind[2]][j_ind[0]] +
					data[i_ind[0]][j_ind[2]] * data[i_ind[1]][j_ind[0]] * data[i_ind[2]][j_ind[1]] -
					data[i_ind[2]][j_ind[0]] * data[i_ind[1]][j_ind[1]] * data[i_ind[0]][j_ind[2]] -
					data[i_ind[2]][j_ind[1]] * data[i_ind[1]][j_ind[2]] * data[i_ind[0]][j_ind[0]] -
					data[i_ind[2]][j_ind[2]] * data[i_ind[1]][j_ind[0]] * data[i_ind[0]][j_ind[1]]);
		}

		static matrix4x4 diag(double m0, double m1, double m2, double m3) {
			matrix4x4 m;
			m.m00 = m0;
			m.m01 = 0.0;
			m.m02 = 0.0;
			m.m03 = 0.0;
			m.m10 = 0.0;
			m.m11 = m1;
			m.m12 = 0.0;
			m.m13 = 0.0;
			m.m20 = 0.0;
			m.m21 = 0.0;
			m.m22 = m2;
			m.m23 = 0.0;
			m.m30 = 0.0;
			m.m31 = 0.0;
			m.m32 = 0.0;
			m.m33 = m3;

			return m;
		}

		bool operator ==(const matrix4x4& m) const {
			for (size_t i{}; i < 16U; i++)
				if (!(equals_eps(rowdata[i], m.rowdata[i]))) return false;
			return true;
		}
	};
	class FEMCADGEOMSHARED_EXPORT transform {
	public:
		vector3 position;
		quaternion rotation;
		vector3 scale;
		transform() : position(), rotation(), scale(1.0, 1.0, 1.0) {}
		transform(vector3 translation, quaternion rotation = quaternion(), vector3 scale = vector3(1.0, 1.0, 1.0)) :
			position(translation), rotation(rotation.get_unit()), scale(scale) {}
		transform(const transform& t) : position(t.position), rotation(t.rotation), scale(t.scale) {}
		transform& operator =(const transform& t) {
			position = (t.position); rotation = (t.rotation); scale = (t.scale);

			return *this;
		}
		matrix4x4 get_rotation()const {
			matrix4x4 m;
			auto v = rotation.v;
			auto w = rotation.w;
			m.m00 = 1. - 2. * v.y*v.y - 2. * v.z*v.z;
			m.m11 = 1. - 2. * v.x*v.x - 2. * v.z*v.z;
			m.m22 = 1. - 2. * v.y*v.y - 2. * v.x*v.x;
			m.m01 = 2. * v.x * v.y - 2. * v.z * w;
			m.m02 = 2. * v.x * v.z - 2. * v.y * w;
			m.m10 = 2. * v.x * v.y + 2. * v.z * w;
			m.m12 = 2. * v.z * v.y - 2. * v.x * w;
			m.m20 = 2. * v.x * v.z + 2. * v.y * w;
			m.m21 = 2. * v.z * v.y + 2. * v.x * w;
			m.m03 = m.m30 = m.m13 = m.m31 = m.m23 = m.m32 = 0.0;
			m.m33 = 1.0;
			return m;
		}
		matrix4x4 get_translation()const {
			matrix4x4 m;
			m.m00 = 1;
			m.m01 = 0;
			m.m02 = 0;
			m.m03 = -position.x;
			m.m10 = 0;
			m.m11 = 1;
			m.m12 = 0;
			m.m13 = -position.y;
			m.m20 = 0;
			m.m21 = 0;
			m.m22 = 1;
			m.m23 = -position.z;
			m.m30 = 0;
			m.m31 = 0;
			m.m32 = 0;
			m.m33 = 1;

			return m;
		}
		matrix4x4 get_scale()const {
			return matrix4x4::diag(1. / scale.x, 1. / scale.y, 1. / scale.z, 1.0);
		}

		matrix4x4 get_rotation_inversed()const {
			matrix4x4 m;
			auto v = rotation.v;
			auto w = -rotation.w;
			m.m00 = 1. - 2. * v.y*v.y - 2. * v.z*v.z;
			m.m11 = 1. - 2. * v.x*v.x - 2. * v.z*v.z;
			m.m22 = 1. - 2. * v.y*v.y - 2. * v.x*v.x;
			m.m01 = 2. * v.x * v.y - 2. * v.z * w;
			m.m02 = 2. * v.x * v.z - 2. * v.y * w;
			m.m10 = 2. * v.x * v.y + 2. * v.z * w;
			m.m12 = 2. * v.z * v.y - 2. * v.x * w;
			m.m20 = 2. * v.x * v.z + 2. * v.y * w;
			m.m21 = 2. * v.z * v.y + 2. * v.x * w;
			m.m03 = m.m30 = m.m13 = m.m31 = m.m23 = m.m32 = 0.0;
			m.m33 = 1.0;
			return m;
		}
		matrix4x4 get_translation_inversed()const {
			matrix4x4 m;
			m.m00 = 1;
			m.m01 = 0;
			m.m02 = 0;
			m.m03 = position.x;
			m.m10 = 0;
			m.m11 = 1;
			m.m12 = 0;
			m.m13 = position.y;
			m.m20 = 0;
			m.m21 = 0;
			m.m22 = 1;
			m.m23 = position.z;
			m.m30 = 0;
			m.m31 = 0;
			m.m32 = 0;
			m.m33 = 1;

			return m;
		}
		matrix4x4 get_scale_inversed()const {
			return matrix4x4::diag(scale.x, scale.y, scale.z, 1.0);
		}

		matrix4x4 get_inversed()const {
			return get_rotation() * get_scale() * get_translation();
		}
		matrix4x4 get() const {
			return get_translation_inversed() * get_scale_inversed() * get_rotation_inversed();
		}

		vector3 apply_point(const vector3& p) const {
			return get() * p;
		}
		vector3 apply_inverse_point(const vector3& p) const {
			return get_inversed() * p;
		}
		vector3 apply_vector(const vector3& p) const {
			return get() & p;
		}
		vector3 apply_inverse_vector(const vector3& p) const {
			return get_inversed() & p;
		}
	};

	class FEMCADGEOMSHARED_EXPORT ray {
	public:
		vector3 p0, l;
		ray() : l(0.0, 0.0, 1.0) {}
		ray(const ray& r) : p0(r.p0), l(r.l) {}
		ray& operator=(const ray& r) = default;
		ray(const vector3& p0, const vector3& line) : p0(p0), l(line) {
			try {
				l.normalize();
			}
			catch (std::exception&) {
				throw std::logic_error("Zero-length ray");
			}
		}
		static ray byTwoPoints(const vector3& p0, const vector3& p1) {
			return ray(p0, p1 - p0);

		}
	};

	vector3 segmentIntersection(const vector3& l0p0, const vector3& l0p1, const vector3& l1p0, const vector3& l1p1);

	class FEMCADGEOMSHARED_EXPORT polynome {
	public:
		std::vector<double> coef;
		polynome() {}
		polynome(int n) : coef(n) {}
		polynome(const std::vector<double>& c) : coef(c) {}
		polynome(const polynome& p) = default;// : coef(p.coef) {}
		polynome& operator =(const polynome& p) = default;// { coef = (p.coef); return *this; }
		static polynome constant(double c) { polynome s(1); s.coef[0] = c; return s; }
		static polynome linear(double a, double b) { polynome s(2); s.coef[0] = b; s.coef[1] = a; return s; }
		static polynome square(double a, double b, double c) { polynome s(3); s.coef[0] = c; s.coef[1] = b;  s.coef[2] = a; return s; }
		static polynome shrink(const polynome& p) {
			polynome s;
			s.coef.insert(s.coef.begin(), p.coef.begin(), p.coef.end() - 1);
			return s;
		}

		double round_eps(double x, double eps) const {
			long long u = long long(x);
			return u + (round((x - u) / eps)) * eps;
		}
#define _EQ_EPS 1e-12

		polynome derivative() {
			polynome nc(coef.size() - 1);
			for (size_t i = 1U; i < coef.size(); i++) {
				nc.coef[i - 1] = coef[i] * i;
			}
			return (nc);
		}
		void find_roots(bool& v, std::vector<double>& roots) {
			if (coef.size() == 0) {
				v = true;
				return;
			}
			if (fabs(coef.back()) < 2*_EQ_EPS) {
				auto pp = shrink(*this);
				pp.find_roots(v, roots);
				return;
			}
			if (coef.size() == 1) {
				v = false;
				return;
			}
			if (coef.size() == 2) {
				v = false;
				roots.push_back(round_eps(-coef[0] / coef[1], _EQ_EPS));
				return;
			}
			/*if (coef.size() == 3) {
				v = false;
				double D = coef[1] * coef[1] - 4.0 * coef[0] * coef[2];
				double eps = std::fabs(2.0e-11*std::max(coef[0], std::max(coef[1], coef[2]))*(coef[1] - 2 * (coef[0] + coef[2])));
				if (std::fabs(D) < -eps) {
					return;
				}
				if (D < eps) {
					roots.push_back(-coef[1] / coef[2] * 0.5);
					return;
				}
				roots.push_back((-coef[1] - std::sqrt(D)) / coef[2] * 0.5);
				roots.push_back((-coef[1] + std::sqrt(D)) / coef[2] * 0.5);
				return;
			}*/
			if (!(coef.size() % 2)) {
				// find one root
				double root;
				double l, r;
				find_boundaries_odd(l, r);
				root = binary_search(l, r, evaluate(l), evaluate(r));
				roots.push_back(round_eps(root, _EQ_EPS));
				divide(root).find_roots(v, roots);
				return;
			}
			std::vector<double> der_roots;
			derivative().find_roots(v, der_roots);
			std::sort(der_roots.begin(), der_roots.end());
			double right = der_roots.front(), fr;
			double left, fl;
			find_left_boundary_even(left, right);
			fl = evaluate(left);
			for (size_t i = 0U; i < der_roots.size(); i++) {
				right = der_roots[i];
				fr = evaluate(right);
				if (std::fabs(fr) < 1e-100) {
					roots.push_back(round_eps(right, _EQ_EPS));
					if (i == der_roots.size() - 1) {
						v = false;
						return;
					}
					left = 0.5f*(der_roots[i + 1] + der_roots[i]);
					fl = evaluate(left);
					continue;
				}
				if (std::fabs(fl) < 1e-100) {
					roots.push_back(round_eps(left, _EQ_EPS));
					left = right;
					fl = fr;
					continue;
				}
				if (fr * fl <= 0) {
					double root = binary_search(left, right, fl, fr);
					roots.push_back(round_eps(root, _EQ_EPS));
					left = right;
					fl = fr;
				}
			}
			if (fl * coef.back() > 0) {
				v = false;
				return;
			}
			left = right;
			fl = fr;
			find_right_boundary_even(left, right);
			double r = binary_search(left, right, fl, evaluate(right));
			roots.push_back(round_eps(r, _EQ_EPS));
			v = false;
			return;
		}
		double evaluate(double x)  const { return this->operator()(x); }
		double operator()(double x) const {
			double result = 0;
			for (int i = coef.size() - 1; i > 0; i--) {
				result = (result + coef[i]) * x;
			}
			result += coef[0];
			return result;
		}
		polynome divide(double root) {
			polynome result(coef.size() - 1);
			result.coef[coef.size() - 2] = coef.back();
			for (int i = coef.size() - 2; i > 0; i--) {
				result.coef[i - 1] = coef[i] + root * result.coef[i];
			}
			return result;
		}

	protected:
		double binary_search(double l, double r, double fl, double fr) {
			double c = 0.5 * (r + l);
			if (fabs(fl) < 1e-100) return l;
			if (fabs(fr) < 1e-100) return r;
			double fc = this->operator ()(c);
			if (c == r || c == l) return c;
			//if (r - l < 1e-12) return c;
			if (fl > 0) {
				if (fc > 0) return binary_search(c, r, fc, fr);
				else return binary_search(l, c, fl, fc);
			}
			else {
				if (fc < 0) return binary_search(c, r, fc, fr);
				else return binary_search(l, c, fl, fc);
			}
		}
		void find_boundaries_odd(double& left, double& right) {
			bool exit;
			left = -1;
			right = 1;
			double fl = this->operator ()(left);
			double fr = this->operator ()(right);
			while (true) {
				exit = true;
				if (fr * coef.back() < 0) {
					right *= 2;
					fr = this->operator ()(right);
					exit = false;
				}
				if (fl * coef.back() > 0) {
					left *= 2;
					fl = this->operator ()(left);
					exit = false;
				}
				if (exit) return;
			}
		}
		void find_left_boundary_even(double& left, const double& right) {
			bool exit;
			double shift = 1;
			left = right - shift;
			double fl = this->operator ()(left);
			while (true) {
				exit = true;
				if (fl * coef.back() < 0) {
					shift *= 2;
					left = right - shift;
					fl = this->operator ()(left);
					exit = false;
				}
				if (exit) return;
			}
		}
		void find_right_boundary_even(const double& left, double& right) {
			bool exit;
			double shift = 1;
			right = left + shift;
			double fr = this->operator ()(right);
			while (true) {
				exit = true;
				if (fr * coef.back() < 0) {
					shift *= 2;
					right = left + shift;
					fr = this->operator ()(right);
					exit = false;
				}
				if (exit) return;
			}
		}
	};

	class FEMCADGEOMSHARED_EXPORT rect {
		vector3 min;
		vector3 max;
	public:
		inline const vector3 Min() const { return min; }
		inline const vector3 Max() const { return max; }
		inline constexpr rect() = default;
		inline constexpr rect(const rect&) = default;
		inline rect& operator=(const rect&) = default;
		inline constexpr rect(const vector3& a) : min(a), max(a) {}
		inline constexpr rect(double x0, double x1, double y0, double y1, double z0 = 0, double z1 = 0) :
			min{ std::min(x0, x1) ,std::min(y0, y1) ,std::min(z0, z1) },
			max{ std::max(x0, x1) ,std::max(y0, y1) ,std::max(z0, z1) } {
		}
		inline constexpr rect(const vector3& a, const vector3&b) :
			min{ std::min(a.x, b.x) ,std::min(a.y, b.y) ,std::min(a.z, b.z) },
			max{ std::max(a.x, b.x) ,std::max(a.y, b.y) ,std::max(a.z, b.z) } {
		}

		inline rect& add_point(const vector3& p) {
			if (std::numeric_limits<double>::infinity() == p.x || std::numeric_limits<double>::infinity() == p.y || std::numeric_limits<double>::infinity() == p.z) throw std::invalid_argument("Point at infinity");
			min = vector3{ std::min(min.x, p.x) ,std::min(min.y, p.y) ,std::min(min.z, p.z) };
			max = vector3{ std::max(max.x, p.x) ,std::max(max.y, p.y) ,std::max(max.z, p.z) };


			return *this;
		}
		inline bool operator ==(const rect& p) const {
			return min == p.min && max == p.max;
		}
		inline bool operator !=(const rect& p) const {
			return min != p.min || max != p.max;
		}
		inline rect& add_rect(const rect& p) {
			min = vector3{ std::min(min.x, p.min.x) ,std::min(min.y, p.min.y) ,std::min(min.z, p.min.z) };
			max = vector3{ std::max(max.x, p.max.x) ,std::max(max.y, p.max.y) ,std::max(max.z, p.max.z) };

			return *this;
		}
		inline vector3 center() const { return 0.5 * (min + max); }
		inline bool itersect(const vector3& p) const {
			return p.x >= min.x && p.y >= min.y && p.z >= min.z &&
				p.x <= max.x && p.y <= max.y && p.z <= max.z;
		}

		inline bool itersect(const rect& p) const {
			return !(max.x < p.min.x || max.y < p.min.y || max.z < p.min.z ||
				min.x > p.max.x || min.y > p.max.y || min.z > p.max.z);
		}
		template<int A>
		inline int classify_by(double x) const {
			return x < min[A] - FG_EPS ? 1 : (x > max[A] + FG_EPS ? -1 : 0);
		}
	};






	
	// вроде бы это - квадродерево ???
	template<int D, class T, int plane = axis::AXIS_X>
	class FEMCADGEOMSHARED_EXPORT lookup_tree
	{
#ifdef _DEBUG
	public:
#endif
		// поддеревья (у них плоскость поиска будет перпендикулярной родительскому)
		std::array<lookup_tree<D, T, (plane + 1) % D>*, 2> s;
		std::vector<std::pair<rect, T>> container;
		size_t Mx, max_depth;
		vector3 minimum, maximum;
	public:
		lookup_tree(lookup_tree&& r) : container{ std::move(r.container) }, Mx{ r.Mx }, max_depth{ r.max_depth }, minimum{ r.minimum }, maximum{ r.maximum }
		{
			s[0] = r.s[0];
			s[1] = r.s[1];
			r.s[0] = nullptr;
			r.s[1] = nullptr;
		}

		lookup_tree(size_t max_depth, std::vector<std::pair<rect, T>>&& rectangles, const vector3& min, const vector3& max) :
			lookup_tree(max_depth, std::max(1000U, (size_t)std::sqrt(rectangles.size()) + 10), std::move(rectangles), min, max, max_depth) {
		}

		lookup_tree(size_t depth, size_t max_elements, std::vector<std::pair<rect, T>>&& rectangles,
			const vector3& min, const vector3& max, const size_t max_depth)
			: Mx{ max_elements }, max_depth{ max_depth }
		{
			s[0] = nullptr;
			s[1] = nullptr;
			// мин и макс - координаты вершин ограниивающего прямоугольника, в который входит весь примитив
			minimum = min;
			maximum = max;
			if (rectangles.size() < Mx || depth <= 0) {
				container = rectangles;
				container.reserve(Mx); 
				return;
			}
			std::vector<std::pair<rect, T>> front, back;
			// усредн¤ем координаты х вершин мин и макс ограничивающего пр¤моугольника
			double midaxis = 0.5 * (minimum[plane] + maximum[plane]);
			for (size_t i = 0U; i < rectangles.size(); i++) {
				auto c = rectangles[i].first.classify_by<plane>(midaxis);
				if (c <= 0) back.push_back(rectangles[i]);
				if (c >= 0) front.push_back(rectangles[i]);
			}
			vector3 nmin = min; nmin[plane] = midaxis;
			vector3 nmax = max; nmax[plane] = midaxis;
			if (back.size())
				//s[0] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(lookup_tree<D, T, (plane + 1) % D>(depth - 1, Mx, std::move(back), min, nmax, max_depth)));
				s[0] = new lookup_tree<D, T, (plane + 1) % D>(depth - 1, Mx, std::move(back), min, nmax, max_depth);
			if (front.size())
				//s[1] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(lookup_tree<D, T, (plane + 1) % D>(depth - 1, Mx, std::move(front), nmin, max, max_depth)));
				s[1] = new lookup_tree<D, T, (plane + 1) % D>(depth - 1, Mx, std::move(front), nmin, max, max_depth);
		}

		~lookup_tree()
		{
			if (s[0]) {
				delete s[0];
				s[0] = nullptr;
			}
			if (s[1]) {
				delete s[1];
				s[1] = nullptr;
			}
		}

		// находит все элементы дерева, которые пересекаются с данным прямоугольником
		inline void get_overlap(const rect& r, std::set<T>& output) const
		{
			auto c = r.classify_by<plane>(0.5*(maximum[plane] + minimum[plane]));
			if (c <= 0 && s[0]) s[0]->get_overlap(r, output);
			if (c >= 0 && s[1]) s[1]->get_overlap(r, output);
			for (auto& i : container) {
				if (r.itersect(i.first)) {
					output.insert(i.second);
				}
			}
		}

		mutable size_t counter = 0;

		// находит все элементы дерева, на которые попадает данная точка
		inline void get_overlap(const vector3& r, std::vector<T>& output) const
		{
			auto mid = (0.5*(maximum[plane] + minimum[plane]));
			counter++;
			auto c = r[plane] < mid ? -1 : r[plane] > mid ? 1 : 0;
			if (c<=0 && s[0])
				s[0]->get_overlap(r, output);
			if (c>=0 && s[1])
				s[1]->get_overlap(r, output);
			for (auto& i : container)
				if (i.first.itersect(r))
					output.push_back(i.second);
			return;
		}

		inline bool add_to_container(const std::pair<rect, T>& element) {
			for (size_t i{}; i < container.size(); ++i) {
				if (container[i].second == element.second) {
					if (container[i].first != element.first)
					{
						std::cout << "Problem in tree";
						throw "Tree error";
					}
					return false;
				}
			}
			container.push_back(element);
			return true;
		}

		inline void add_element(const std::pair<rect, T>& element, size_t depth = 0)
		{
			if (depth == max_depth) { // || container.size() < Mx
				add_to_container(element);
				return;
			}
			if (!(s[0] || s[1])) {
				add_to_container(element);
				if (depth < max_depth && container.size() >= Mx) {
					std::vector<std::pair<rect, T>> front, back;
					double midaxis = 0.5 * (minimum[plane] + maximum[plane]);
					for (size_t i = 0U; i < container.size(); i++) {
						auto c = container[i].first.classify_by<plane>(midaxis);
						if (c <= 0) back.push_back(container[i]);
						if (c >= 0) front.push_back(container[i]);
					}
					container.clear();
					container.shrink_to_fit();
					vector3 nmin = minimum; nmin[plane] = midaxis;
					vector3 nmax = maximum; nmax[plane] = midaxis;
					if (back.size()) 
						//s[0] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(\
						lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(back), minimum, nmax, max_depth)));
						s[0] = new lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(back), minimum, nmax, max_depth);
					if (front.size()) 
						//s[1] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(\
						lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(front), nmin, maximum, max_depth)));
						s[1] = new lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(front), nmin, maximum, max_depth);
				}
				return;
			}
			double midaxis = 0.5 * (minimum[plane] + maximum[plane]);
			auto c = element.first.classify_by<plane>(midaxis);
			if (c <= 0) {
				if (s[0]) s[0]->add_element(element, depth + 1);
				else /*if (max_depth > depth)*/ {
					vector3 nmax = maximum; nmax[plane] = midaxis;
					//s[0] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(\
						lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(std::vector<std::pair<rect, T>>{ element }), minimum, nmax, max_depth)));
					s[0] = new lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(std::vector<std::pair<rect, T>>{ element }), minimum, nmax, max_depth);
				}
			}
			if (c >= 0)
				if (s[1]) s[1]->add_element(element, depth + 1);
				else /*if (max_depth > depth)*/ {
					vector3 nmin = minimum; nmin[plane] = midaxis;
					//s[1] = std::make_unique<lookup_tree<D, T, (plane + 1) % D>>(std::move(\
						lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(std::vector<std::pair<rect, T>>{ element }), nmin, maximum, max_depth)));
					s[1] = new lookup_tree<D, T, (plane + 1) % D>(max_depth - depth - 1, Mx, std::move(std::vector<std::pair<rect, T>>{ element }), nmin, maximum, max_depth);
				}
		}

		inline bool remove_element(const std::pair<rect, T>& element) {
			if (!container.empty()) {
				size_t i{};
				auto s = container.size();
				size_t deleted{s};
				for (; i < s; i++) {
					if (container[i].second == element.second) {
						if (container[i].first != element.first) {
							std::cout << "Tree problems";
							throw;
						}
						if (deleted != s) {
							std::cout << "Tree duplicate!" <<std::endl;
							throw;
						}
						deleted = i;
						//break;
					}
				}
				if (deleted == container.size()) {
					return false;
				}
				else {
					container[deleted] = std::move(container.back());
					container.pop_back();
				}
				return true;
			}
			double midaxis = 0.5 * (minimum[plane] + maximum[plane]);
			auto c = element.first.classify_by<plane>(midaxis);
			bool result = true;
			if (c <= 0)
				if (s[0]) 
					result = s[0]->remove_element(element) && result;
			if (c >= 0)
				if (s[1]) 
					result = s[1]->remove_element(element) && result;
			return result;
		}
		inline void replace_element(const std::pair<rect, T>& element, T newv) {
			if (!container.empty()) {
				size_t i{};
				for (; i < container.size(); i++) {
					if (container[i].second == element.second) {
						container[i].second = newv;
						break;
					}
				}
				return;
			}
			double midaxis = 0.5 * (minimum[plane] + maximum[plane]);
			auto c = element.first.classify_by<plane>(midaxis);
			if (c <= 0 && s[0]) s[0]->replace_element(element, newv);
			if (c >= 0 && s[1]) s[1]->replace_element(element, newv);
		}
		inline void replace_element_rect(const std::pair<rect, T>& element, rect newv) {
			remove_element(element);
			add_element(std::make_pair(newv, element.second));
		}
		inline bool has_element_pred(T e, std::function<bool(T,T)> f) const {
			if (container.size()) {
				for (size_t i{}; i < container.size(); ++i) {
					if (f(container[i].second, e)) {
						return true;
					}
				}
				return false;
			}
			return (s[0] && s[0]->has_element_pred(e,f)) || (s[1] && s[1]->has_element_pred(e,f));
		}
		inline bool traverse_tree(std::function<bool(const rect&, T)> f) const {
			if (container.size()) {
				for (size_t i{}; i < container.size(); ++i) {
					if (f(container[i].first, container[i].second)) {
						std::cout << "Element " << container[i].second << " is wrong!" << std::endl;
						return true;
					}
				}
				return false;
			}
			return (s[0] && s[0]->traverse_tree(f)) || (s[1] && s[1]->traverse_tree(f));
		}


		bool isCorrect() const {
			if (container.size()) {
				std::set<T> dups;
				for (size_t i{}; i < container.size(); ++i) {
					if (dups.count(container[i].second)) {
						std::cout << "Duplicate!";
						return false;
					}
					dups.insert(container[i].second);
					if (!rect(minimum, maximum).itersect(container[i].first)) {
						std::cout << "Element " << container[i].second << " is wrong!" << std::endl;
						return false;
					}
				}
				return true;
			}
			return (!s[0] || s[0]->isCorrect()) && (!s[1] || s[1]->isCorrect());
		}
		friend std::ostream& operator<<(std::ostream& s, const lookup_tree<D, T, plane>& p) {
			s << "{" << std::endl;
			s << "\"plane\": \"" << plane << "\"," << std::endl;
			s << "\"minimum\": \"" << p.minimum << "\"," <<std::endl;
			s << "\"maximum\" : \"" << p.maximum << "\"," <<std::endl;
			
			if (p.container.size()) {
				s << "\"container\" : [" << std::endl;
				for (auto elem : p.container)
					s << "{\"min\": \"" << elem.first.Min() << "\", \"max\": \"" << elem.first.Max() << "\", \"element\": " << elem.second << "}," << std::endl;
				s << "]," << std::endl;
			}
			if (p.s[0]) {
				s << "\"front\" : "<<*(p.s[0]) << ',';
			}
			if (p.s[1]) {
				s << "\"back\" : " << *(p.s[1]) << ',';
			}
			s << "}" << std::endl;
			return s;
		}
	};



	// класс, задающий уравнение окружности
	class FEMCADGEOMSHARED_EXPORT square_curve {
	public:
		// a * x2 + b * y2 + c * x * y + d * x + e * y + f = 0
		double a, b, c, d, e, f;
		square_curve() {
			a = b = c = d = e = f = 0.0;
		}
		//inline rect get_bounding_rect() const {
		//	double py = 4 * a * b - c * c;
		//	double qy = 4 * a * e - 2 * c * d;
		//	double sy = 4 * a * f - (2 * c + d) * d;
		//
		//	double px = 4 * b * a - c * c;
		//	double qx = 4 * b * d - 2 * c * e;
		//	double sx = 4 * b * f - (2 * c + e) * e;
		//
		//	double ay = -qy / 2 * py;
		//	double dy = std::sqrt(qy*qy - 4 * py * sy) / 2 * py;
		//
		//	double ax = -qx / 2 * px;
		//	double dx = std::sqrt(qx*qx - 4 * px * sx) / 2 * px;
		//	return rect{ {ax - dx, ay - dy, 0}, {ax + dx, ay + dy, 0} };
		//}
		//

		// точка пересечени¤ касательных к дуге, проведЄнных в точках p0 и p1
		vector3 control_point(const vector3& p0, const vector3& p1) {
			double dy0 = 2.0*a*p0.x + c*p0.y + d;
			double dy1 = 2.0*a*p1.x + c*p1.y + d;
			double dx0 = c*p0.x + 2.0*b*p0.y + e;
			double dx1 = c*p1.x + 2.0*b*p1.y + e;

			double b0 = p0.x * dy0 + p0.y*dx0;
			double b1 = p1.x * dy1 + p1.y*dx1;

			double delta = dx0 * dy1 - dx1 * dy0;
			double deltax = dx0 * b1 - dx1 * b0;
			double deltay = -dy0 * b1 + dy1 * b0;

			if (std::fabs(delta) < FG_EPS) {
				if (std::fabs(deltax) < FG_EPS && std::fabs(deltay) < FG_EPS) return (p0 + p1) * 0.5;
				return{ std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), 0.0 };
			}
			return{ deltax / delta, deltay / delta, 0.0 };
		}
		static square_curve line(const vector3& p0, const vector3& t) {
			square_curve result;
			result.a = 0;
			result.b = 0;
			result.c = 0;
			result.d = t.y;
			result.e = -t.x;
			result.f = p0.y*t.x - p0.x*t.y;

			return result;
		}
		static square_curve line(const vector3& n, const double m) {
			square_curve result;
			result.a = 0;
			result.b = 0;
			result.c = 0;
			result.d = n.x;
			result.e = n.y;
			result.f = m;

			return result;
		}
		static square_curve circle(double radius_squared, const vector3& center = vector3()) {
			square_curve result;
			result.a = 1;
			result.b = 1;
			result.c = 0;
			result.d = -2.0*center.x;
			result.e = -2.0*center.y;
			result.f = -radius_squared + center.lengthSq();

			return result;
		}
		//static square_curve ellipse(double a, double b, const vector3& a_dir, const vector3& center = vector3()){
		//     square_curve result;
		//    result.a = 1;
		//     result.b = 1;
		//    result.c = 0;
		//     result.d = 0;
		//    result.e = 0;
		//     result.f = -1;
		//[TODO Quaternion from X-rotation]
		//    transform t(center, quaternion::get_by_rotation(0.0, 0.0, std::atan2(a_dir.x, a_dir.y)), vector3(a, b, 1.0));
		//      return result.getTransformed(t.get());
		// }

		square_curve(double a, double b, double c, double d, double e, double f) :
			a(a),
			b(b),
			c(c),
			d(d),
			e(e),
			f(f) {}
		square_curve(square_curve&& c) = default;
		square_curve(const square_curve& c) :
			a(c.a),
			b(c.b),
			c(c.c),
			d(c.d),
			e(c.e),
			f(c.f) {}
		square_curve& operator=(const square_curve& cr) {
			a = (cr.a);
			b = (cr.b);
			c = (cr.c);
			d = (cr.d);
			e = (cr.e);
			f = (cr.f);

			return *this;
		}

		int classify(const vector3& pos) const {
			auto s = get_y_equ(pos.x).evaluate(pos.y);
			return (s > -FG_EPS) ? (s < FG_EPS ? 0 : (b > -FG_EPS ? 1 : 0)) : -(b > -FG_EPS ? 1 : 0);
		}

		int cross(const vector3& n, const double m) const {
			if (std::fabs(n.y) < FG_EPS) {
				auto s = (e * n.x - c * m);
				auto result = (s * s - 4 * b * (a * m * m + n.x * (n.x * f - d * m)));
				return result > -FG_EPS ? (result < FG_EPS ? 1 : 2) : 0;
			}
			else {
				auto s = a * n.y * n.y + (b * n.x - c * n.y) * n.x;
				auto u = 2 * b * n.x * m + (d * n.y - e * n.x - c * m) * n.y;
				auto v = b * m * m + (f * n.y - e * m) * n.y;
				auto result = (u * u - 4 * s * v);
				return result > -FG_EPS ? (result < FG_EPS ? 1 : 2) : 0;
			}
		}

		square_curve getTransformed(const matrix4x4& mm)const {
			square_curve result;
			auto m = mm.get_inversed();
			result.a = (d * m.m00 * m.m30 + b * m.m10 * m.m10 + e * m.m10 * m.m30 + a * m.m00 * m.m00 + c * m.m00 * m.m10 + f * m.m30 * m.m30);
			result.b = (b * m.m11 * m.m11 + d * m.m01 * m.m31 + e * m.m11 * m.m31 + a * m.m01 * m.m01 + f * m.m31 * m.m31 + c * m.m01 * m.m11);
			result.c = (c * m.m00 * m.m11 + 2 * b * m.m11 * m.m10 + e * m.m11 * m.m30 + e * m.m10 * m.m31 + d * m.m01 * m.m30 + 2 * a * m.m01 * m.m00 + d * m.m00 * m.m31 + c * m.m01 * m.m10 + 2 * f * m.m31 * m.m30);
			result.d = (2 * a * m.m03 * m.m00 + d * m.m00 * m.m33 + c * m.m03 * m.m10 + 2 * f * m.m33 * m.m30 + c * m.m00 * m.m13 + 2 * b * m.m13 * m.m10 + e * m.m13 * m.m30 + e * m.m10 * m.m33 + d * m.m03 * m.m30);
			result.e = (2 * a * m.m03 * m.m01 + 2 * f * m.m33 * m.m31 + d * m.m03 * m.m31 + d * m.m01 * m.m33 + e * m.m13 * m.m31 + e * m.m11 * m.m33 + 2 * b * m.m13 * m.m11 + c * m.m03 * m.m11 + c * m.m01 * m.m13);
			result.f = (a * m.m03 * m.m03 + b * m.m13 * m.m13 + c * m.m03 * m.m13 + d * m.m03 * m.m33 + e * m.m13 * m.m33 + f * m.m33 * m.m33);


			//        result.a = d * m.m30 * m.m00 + b * m.m10 * m.m10 + e * m.m30 * m.m10 + a * m.m00 * m.m00 + c * m.m00 * m.m10 + f * m.m30 * m.m30;
			//        result.b = b * m.m11 * m.m11 + d * m.m31 * m.m01 + e * m.m31 * m.m11 + a * m.m01 * m.m01 + f * m.m31 * m.m31 + c * m.m01 * m.m11;
			//        result.c = c * m.m00 * m.m11 + 2 * b * m.m11 * m.m10+e * m.m31 * m.m10+e * m.m30 * m.m11 + d * m.m31 * m.m00 + 2 * a * m.m01 * m.m00 + d * m.m30 * m.m01 + c * m.m01 * m.m10 + 2 * f * m.m31 * m.m30;
			//        result.d = 2 * a * m.m03 * m.m00 + d * m.m30 * m.m03 + c * m.m03 * m.m10 + 2 * f * m.m33 * m.m30 + c * m.m00 * m.m13 + 2 * b * m.m13 * m.m10 + e * m.m33 * m.m10 + e * m.m30 * m.m13 + d * m.m33 * m.m00;
			//        result.e = 2 * a * m.m03 * m.m01 + 2 * f * m.m33 * m.m31 + d * m.m33 * m.m01 + d * m.m31 * m.m03 + e * m.m33 * m.m11 + e * m.m31 * m.m13 + 2 * b * m.m13 * m.m11 + c * m.m03 * m.m11 + c * m.m01 * m.m13;
			//        result.f = a * m.m03 * m.m03 + b * m.m13 * m.m13 + c * m.m03 * m.m13 + d * m.m33 * m.m03 + e * m.m33 * m.m13 + f * m.m33 * m.m33;

			return result;
		}

		void find_intersection(bool& v, const square_curve& cr, std::vector<vector3>& intersections) const
		{
			polynome xp = this->collect_x(cr);
			const square_curve* cur = &cr;
			const square_curve* ths = this;
			//square_curve ccr;
			//square_curve th;
			//        bool changed = xp.coef[0] != xp.coef[0];
			//        if(changed){
			//            ccr.a = cr.b;
			//            ccr.b = cr.a;
			//            ccr.c = cr.c;
			//            ccr.d = cr.e;
			//            ccr.e = cr.d;
			//            ccr.f = cr.f;
			//            cur = &ccr;
			//            th.a = this->b;
			//            th.b = this->a;
			//            th.c = this->c;
			//            th.d = this->e;
			//            th.e = this->d;
			//            th.f = this->f;
			//            ths = &th;
			//            xp = ths->collect_x(ccr);
			//        }
			std::vector<double> xs;
			xp.find_roots(v, xs);
			if (v) return;
			for (size_t i = 0U; i < xs.size(); i++) {
				polynome yp = ths->get_y_equ(xs[i]);
				std::vector<double> ys;
				bool vv;
				yp.find_roots(vv, ys);
				if (vv) {
					yp = cur->get_y_equ(xs[i]);
					ys.clear();
					yp.find_roots(v, ys);
					if (v) return;
				}
				if (ys.size() == 0) return;
				if (ys.size() == 1) {
					if (ths->is_on_boundary(vector3(xs[i], ys[0], 0.0)) && cur->is_on_boundary(vector3(xs[i], ys[0], 0.0)))
						intersections.push_back(vector3(xs[i], ys[0], 0.0));
					continue;
				}
				if (ths->is_on_boundary(vector3(xs[i], ys[0], 0.0)) && cur->is_on_boundary(vector3(xs[i], ys[0], 0.0)))
					intersections.push_back(vector3(xs[i], ys[0], 0.0));
				if (ths->is_on_boundary(vector3(xs[i], ys[1], 0.0)) && cur->is_on_boundary(vector3(xs[i], ys[1], 0.0)))
					intersections.push_back(vector3(xs[i], ys[1], 0.0));
			}
			//if(changed){
			//    for(size_t i = 0U; i<intersections.size(); i++){
			//        double tmp = intersections[i].x;
			//        intersections[i].x = intersections[i].y;
			//        intersections[i].y = tmp;
			//    }
			//}
			return;
		}

		bool operator ==(const square_curve& curve) const {
			return equals_eps(a, curve.a) &&
				equals_eps(b, curve.b) &&
				equals_eps(c, curve.c) &&
				equals_eps(d, curve.d) &&
				equals_eps(e, curve.e) &&
				equals_eps(f, curve.f);
		}

		void cache_parametrization(const vector3& v0, const vector3& v1, matrix4x4& invm, double& t0, double& t1) const {
			invm = get_parametrized_transform();//.get_inversed();
			vector3 loc0 = invm * v0;
			vector3 loc1 = invm * v1;
			t0 = std::atan2(loc0.y, loc0.x);
			t1 = std::atan2(loc1.y, loc1.x);
			invm = invm.get_inversed();
		}

		vector3 get_point(double t, const vector3& v0, const vector3& v1) const {
			auto m = get_parametrized_transform();//.get_inversed();
			vector3 loc0 = m * v0;
			vector3 loc1 = m * v1;
			double t0 = std::atan2(loc0.y, loc0.x);
			double t1 = std::atan2(loc1.y, loc1.x);
			double tt;
			if (std::abs(t0 - t1) < PI) {
				tt = t1 * t + (1.0 - t) * t0;
			}
			else {
				if (t0 < 0) t0 += 2 * PI;
				else t1 += 2 * PI;
				tt = t1 * t + (1.0 - t) * (t0);
			}
			double x = std::cos(tt);
			double y = std::sin(tt);
			return m.get_inversed() * vector3(x, y, 0.0);
			//double x = std::cos(t)
		}
		vector3 get_point(double t, const matrix4x4& m, double t0, double t1) const {
			double tt;
			if (std::abs(t0 - t1) < PI) {
				tt = t1 * t + (1.0 - t) * t0;
			}
			else {
				if (t0 < 0) t0 += 2 * PI;
				else t1 += 2 * PI;
				tt = t1 * t + (1.0 - t) * t0;
			}
			double x = std::cos(tt);
			double y = std::sin(tt);
			return m * vector3(x, y, 0.0);
		}
		double get_param(const vector3& point, const matrix4x4& m_inv, double t0, double t1) {
			auto p = m_inv/*.get_inversed()*/ * point;
			double tt = std::atan2(p.y, p.x);
			if (std::abs(t0 - t1) < PI) {
				return (tt - t0) / (t1 - t0);
			}
			else {
				if (t0 < 0) t0 += 2 * PI;
				else t1 += 2 * PI;
				if (tt < 0) tt += 2 * PI;
				return (tt - t0) / (t1 - t0);
			}
		}
	protected:
		matrix4x4 get_parametrized_transform() const {
			double ff = 1.0 / std::sqrt(-(4 * a * f - d * d) * (4 * a * b - c * c) + (2.0*a*e - c*d)*(2.0*a*e - c*d));
			double r = sqrt(4.0*a*b - c*c);
			matrix4x4 m;
			m.m00 = 2.0 * a * r * ff;
			m.m01 = c * r * ff;
			m.m02 = 0.0;
			m.m03 = d * r * ff;
			m.m10 = 0.0;
			m.m11 = (4.0*a*b - c*c)*ff;
			m.m12 = 0.0f;
			m.m13 = (2.0*a*e - c*d) * ff;
			m.m20 = 0.0f;
			m.m21 = 0.0f;
			m.m22 = 1.0f;
			m.m23 = 0.0f;
			m.m30 = 0.0f;
			m.m31 = 0.0f;
			m.m32 = 0.0f;
			m.m33 = 1.0f;

			return m;
		}

		polynome get_y_equ(double x) const {
			return polynome::square(b, c*x + e, a * x * x + d * x + f);
		}
		bool is_on_boundary(const vector3 &x) const {
			return (std::fabs(a * x.x * x.x + b * x.y * x.y + c * x.x * x.y + d * x.x + e * x.y + f) < FG_EPS);
		}
		double ppow(double x, int p) const {
			if (p < 0) { x = 1.0 / x; p = -p; }
			if (p == 0) return 1;
			double res = x;
			for (int i = 1; i < std::abs(p); i++) {
				res *= x;
			}
			return (p >= 0) ? res : 1. / res;
		}

		polynome collect_x(const square_curve& pp) const {
			double na;
			double nb;
			double nc;
			double nd;
			double ne;
			if (std::fabs(pp.b) < 1e-15) {
				if (std::fabs(pp.c) < 1e-15 && std::fabs(pp.e) < 1e-15) {
					na = 0.0;
					nb = 0.0;
					nc = pp.a;
					nd = pp.d;
					ne = pp.f;
				}
				else {
					na = a*pp.c*pp.c - c*pp.a*pp.c + b*pp.a*pp.a;
					nb = -e*pp.a*pp.c + d*pp.c*pp.c + 2 * b*pp.d*pp.a + 2 * a*pp.c*pp.e - c*pp.d*pp.c - c*pp.a*pp.e;
					nc = 2 * b*pp.f*pp.a - c*pp.f*pp.c + 2 * d*pp.c*pp.e - c*pp.d*pp.e + a*pp.e*pp.e + b*pp.d*pp.d - e*pp.a*pp.e + f*pp.c*pp.c - e*pp.d*pp.c;
					nd = 2 * b*pp.f*pp.d + d*pp.e*pp.e - c*pp.f*pp.e + 2 * f*pp.c*pp.e - e*pp.d*pp.e - e*pp.f*pp.c;
					ne = f*pp.e*pp.e - e*pp.f*pp.e + b*pp.f*pp.f;
				}
			}
			else {
				auto invppb = ppow(pp.b, -4);
				na = (invppb * ppow(c * pp.b - b * pp.c, 2) * (-4 * pp.b * pp.a + pp.c * pp.c) / 4 - ppow((-pp.c * c - 2 * b * pp.a) * pp.b + 2 * a * pp.b * pp.b + b * pp.c * pp.c, 2) * invppb / 4);
				nb = (-(2 * d * pp.b * pp.b + (-2 * b * pp.d - pp.e * c - pp.c * e) * pp.b + 2 * b * pp.e * pp.c) * ((-pp.c * c - 2 * b * pp.a) * pp.b + 2 * a * pp.b * pp.b + b * pp.c * pp.c) * invppb / 2 + invppb * (e * pp.b - b * pp.e) * (c * pp.b - b * pp.c) * (-4 * pp.b * pp.a + pp.c * pp.c) / 2 + invppb * ppow(c * pp.b - b * pp.c, 2) * (2 * pp.e * pp.c - 4 * pp.b * pp.d) / 4);
				nc = (-(2 * (2 * f * pp.b * pp.b + b * pp.e * pp.e + (-2 * b * pp.f - e * pp.e) * pp.b) * ((-pp.c * c - 2 * b * pp.a) * pp.b + 2 * a * pp.b * pp.b + b * pp.c * pp.c) + ppow(2 * d * pp.b * pp.b + (-2 * b * pp.d - pp.e * c - pp.c * e) * pp.b + 2 * b * pp.e * pp.c, 2)) * invppb / 4 + invppb * ppow(e * pp.b - b * pp.e, 2) * (-4 * pp.b * pp.a + pp.c * pp.c) / 4 + invppb * (e * pp.b - b * pp.e) * (c * pp.b - b * pp.c) * (2 * pp.e * pp.c - 4 * pp.b * pp.d) / 2 + invppb * ppow(c * pp.b - b * pp.c, 2) * (-4 * pp.b * pp.f + pp.e * pp.e) / 4);
				nd = (-(2 * f * pp.b * pp.b + b * pp.e * pp.e + (-2 * b * pp.f - e * pp.e) * pp.b) * (2 * d * pp.b * pp.b + (-2 * b * pp.d - pp.e * c - pp.c * e) * pp.b + 2 * b * pp.e * pp.c) * invppb / 2 + invppb * ppow(e * pp.b - b * pp.e, 2) * (2 * pp.e * pp.c - 4 * pp.b * pp.d) / 4 + invppb * (e * pp.b - b * pp.e) * (c * pp.b - b * pp.c) * (-4 * pp.b * pp.f + pp.e * pp.e) / 2);
				ne = invppb * ppow(e * pp.b - b * pp.e, 2) * (-4 * pp.b * pp.f + pp.e * pp.e) / 4 - ppow(2 * f * pp.b * pp.b + b * pp.e * pp.e + (-2 * b * pp.f - e * pp.e) * pp.b, 2) * invppb / 4;

				//na = (ppow(pp.a + pp.b * c * c / b / 2. - pp.b * a - pp.c * c / b / 2., 2)
				//	- ppow(pp.b * c - pp.c, 2)
				//	* (-4. * b * a + c * c) * ppow(b, -2) / 4.);
				//nb = (-((2. * pp.b * e - (double)(2 * pp.e)) * (pp.b * c - pp.c) * (-4. * b * a + c * c) + ppow(pp.b * c - pp.c, 2) * (-4. * b * d + 2. * c * e)) * ppow(b, -2) / 4. + (-pp.c * e / b + 2. * pp.b * c * e / b - 2. * pp.b * d - (double)pp.e * c / b + (double)(2 * pp.d)) * (pp.a + pp.b * c * c / b / 2. - pp.b * a - pp.c * c / b / 2.));
				//nc = (-(ppow(pp.b * e - (double)pp.e, 2) * (-4. * b * a + c * c) + (2. * pp.b * e - (double)(2 * pp.e)) * (pp.b * c - pp.c) * (-4. * b * d + 2. * c * e) + ppow(pp.b * c - pp.c, 2) * (-4. * b * f + e * e)) * ppow(b, -2) / 4. + (pp.b * e * e / b - 2. * pp.b * f - (double)pp.e * e / b + (double)(2 * pp.f)) * (pp.a + pp.b * c * c / b / 2. - pp.b * a - pp.c * c / b / 2.) + ppow(-pp.c * e / b / 2. + pp.b * c * e / b - pp.b * d - (double)pp.e * c / b / 2. + (double)pp.d, 2));
				//nd = (-(ppow(pp.b * e - (double)pp.e, 2) * (-4. * b * d + 2. * c * e) + (2. * pp.b * e - (double)(2 * pp.e)) * (pp.b * c - pp.c) * (-4. * b * f + e * e)) * ppow(b, -2) / 4. + (pp.b * e * e / b - 2. * pp.b * f - (double)pp.e * e / b + (double)(2 * pp.f)) * (-pp.c * e / b / 2. + pp.b * c * e / b - pp.b * d - (double)pp.e * c / b / 2. + (double)pp.d));
				//ne = ppow(pp.b * e * e / b / 2. - pp.b * f - (double)pp.e * e / b / 2. + (double)pp.f, 2) - ppow(pp.b * e - (double)pp.e, 2) * (-4. * b * f + e * e) * ppow(b, -2) / 4.;
			}

			polynome result;
			result.coef.push_back(ne);
			result.coef.push_back(nd);
			result.coef.push_back(nc);
			result.coef.push_back(nb);
			result.coef.push_back(na);
			return result;
		}

	};
}

namespace std {
	fg::vector3 min(const fg::vector3& lhs, const fg::vector3& rhs);
}

