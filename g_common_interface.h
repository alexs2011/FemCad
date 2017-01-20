#pragma once
#include "stdafx.h"
#include "femcadgeom_global.h"

namespace fg {
	typedef size_t GHANDLE;
	const GHANDLE NoGeometry = 0xFFFFFFFF;

	class IPrintable {
	public:
		virtual std::string toString() const = 0;
	};
	template<class T>
	class IComparable {
	public:
		virtual bool equals(const T& obj) const = 0;
	};

	class ICSGElement {
	public:
		virtual void Union(const ICSGElement&) = 0;
		virtual void Subtract(const ICSGElement&) = 0;
		virtual void Intersect(const ICSGElement&) = 0;
	};

	enum class CSGOperation {
		Union, Subtract, Intersect
	};

	enum IntersectionState {
		IntersectFirst = 0x1, IntersectSecond = 0x2, Intersect = 0x3, Incident = 0x10, NotIntersect = 0x0, IncidentAndIntersect = 0x13,
		PartialyIncident = 0x30, PartialyIncidentFirst = 0x31, PartialyIncidentSecond = 0x32
	};

	enum class ClassificationState {
		Back = -1,
		Front = 1,
		Incident = 0,
		Cross = 0x7FFFFFFF
	};
}