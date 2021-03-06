﻿#pragma once
#include "stdafx.h"
#include "fg_math.h"
#include "g_setting.h"
#include "g_common_interface.h"

namespace fg {
	class FEMCADGEOMSHARED_EXPORT Scene;

	class FEMCADGEOMSHARED_EXPORT IClassifiable {
	public:
		// лежит ли точка p в этой геометрии (объекте)
		// 1 - лежит снаружи, 0 - на границе, -1 - внутри
		virtual int classify(const vector3& p) const = 0;
		// возвращает среднюю точку геометрии
		virtual vector3 middle() const = 0;
	};

	class IGeometry : public IClassifiable, public IComparable<IGeometry>
	{
	protected:
		// что за контекст ???
		virtual void addSelfToContext() = 0;
	public:
		virtual bool update() = 0;
		virtual void setHandle(GHANDLE value) = 0;
		virtual GHANDLE init() = 0;
		virtual void replace(GHANDLE o, GHANDLE n) = 0;
		virtual std::vector<GHANDLE> getChildren() const = 0;
		virtual GHANDLE copy() const = 0;
		virtual GHANDLE copy(Scene& context) const = 0;
		virtual inline GHANDLE getHandle() const = 0;
		// вернуть прямоугольник, ограничивающий геометрию
		virtual inline rect getBoundingRect() const = 0;
		virtual inline SETTINGHANDLE getSetting() const = 0;
		virtual inline std::vector<GHANDLE> getBoundary() const = 0;
		virtual inline double getDistance(const ray& v) const = 0;
		// лежит ли геометрия в плоскости ???
		virtual inline bool isInPlane(const plane& p) const = 0;
		virtual inline void applyTransform(const matrix4x4& m) = 0;
	};

	// суперкласс, от которого наследуются: вершина, линия, прямоуголник, кривая... ???
	class FEMCADGEOMSHARED_EXPORT Geometry : public IGeometry
	{
	protected:
		Scene& _context;
		GHANDLE _handle;
		SETTINGHANDLE _setting;
	public:
		virtual bool update();
		virtual void setHandle(GHANDLE value) {
			_handle = value;
		}
		inline Scene& getContext() {
			return _context;
		}
		inline Scene& getConstContext() const {
			return _context;
		}
		Geometry(Scene& context, const SETTINGHANDLE& s) :
			_setting(s),
			_context(context),
			_handle{NoGeometry}
		{}
		// move-конструктор
		Geometry(Geometry &&g) = default;
	public:
		virtual SETTINGHANDLE getSetting() const { return _setting; }
		virtual GHANDLE getHandle() const { return _handle; }
	};

}