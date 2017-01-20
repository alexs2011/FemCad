#pragma once
#include "stdafx.h"
#include "g_common_interface.h"

namespace fg {

	class FEMCADGEOMSHARED_EXPORT FGException : public std::exception {
	protected:
		std::string _what;
	public:
		FGException() : std::exception() {}
		FGException(const char * c) : std::exception(), _what(c) {}
		virtual const char* what() const noexcept {
			return _what.c_str();
		}
	};

	class FEMCADGEOMSHARED_EXPORT FGWarning : public FGException {
	public:
		FGWarning() : FGException() {}
		FGWarning(const char * c) : FGException(c) {}
	};

	class FEMCADGEOMSHARED_EXPORT FGSplitEquality : public FGException {
		GHANDLE geom;
	public:
		FGSplitEquality(GHANDLE g) : FGException(), geom(g) {}
		FGSplitEquality(const char * c, GHANDLE g) : geom(g), FGException(c) {}
		GHANDLE getHandle() const { return geom; }
	};

	class FEMCADGEOMSHARED_EXPORT FGTransformNotFound : public FGException {
	public:
		FGTransformNotFound() {}
		virtual const char* what() const noexcept {
			return "Transform not found";
		}
	};

	class FEMCADGEOMSHARED_EXPORT FGObjectNotFound : public FGException {
	public:
		FGObjectNotFound() {}
		virtual const char* what() const noexcept {
			return "Object not found";
		}
	};
	class FEMCADGEOMSHARED_EXPORT FGUndefinedObject : public FGException {
	public:
		FGUndefinedObject() {}
		virtual const char* what() const noexcept {
			return "Trying to access object does not exsist";
		}
	};
}