#pragma once
#include "stdafx.h"
#include "femcadgeom_global.h"
#include "fg_exception.h"
#include <sstream>

namespace fg {
	class FEMCADGEOMSHARED_EXPORT ISetting;
	class FEMCADGEOMSHARED_EXPORT ISettingHasher;
	class FEMCADGEOMSHARED_EXPORT ISettingComparer;

	typedef std::shared_ptr<ISetting> SETTINGHANDLE;
	const SETTINGHANDLE NoSetting;

	class FEMCADGEOMSHARED_EXPORT IParameter;
	class FEMCADGEOMSHARED_EXPORT IParameter {
	public:
		virtual ~IParameter() {}
		//virtual ISetting& getOwner() const = 0;
		virtual operator std::string() const = 0;
	};
	class FEMCADGEOMSHARED_EXPORT DoubleParameter : public IParameter {
		double value;
	public:
		inline DoubleParameter(const double value) : value{ value } {}
		virtual operator std::string() const { return std::to_string(value); }
		virtual operator double() const { return value; }
	};
	template<class Ty>
	class FEMCADGEOMSHARED_EXPORT SettingParameter : public IParameter {
		Ty value;
	public:
		inline SettingParameter(const Ty value) : value{ value } {}
		virtual operator std::string() const { 
			std::stringstream s;
			s << value;
			return s.str(); 
		}
		virtual operator Ty() const { return value; }
	};
	template<>
	class FEMCADGEOMSHARED_EXPORT SettingParameter<std::string> : public IParameter {
		std::string value;
	public:
		inline SettingParameter(const std::string value) : value{ value } {}
		virtual operator std::string() const {
			return value;
		}
	};

	class FEMCADGEOMSHARED_EXPORT ISetting {
	protected:
		static int ID;
		int id;
		std::map <std::string, std::shared_ptr<IParameter>> parameters;
	public:
		int dim = 0;
		virtual ~ISetting() {}
		ISetting() : id(ID++) { }
		ISetting(const ISetting& s) : id(ID++), parameters(s.parameters) { dim = s.dim; }
		ISetting& operator=(const ISetting& s) {
			id = ID++;
			parameters = s.parameters;
			dim = s.dim;
			return *this;
		}
		virtual const IParameter* getParameterByName(const std::string& name) {
			if (parameters.count(name))
				return parameters[name].get();
			throw FGException((std::string() + "Parameter \"" + name + "\" does not exsist!").c_str());
		}
		friend bool operator ==(const ISetting& lhs, const ISetting& rhs) {
			return lhs.id == rhs.id;
		}
		virtual SETTINGHANDLE copy() const = 0;
		int getID() const { return id; }
		template<class T>
		const T getParameter(const std::string& name) {
			if (parameters.count(name)) {
				auto ptr = dynamic_cast<const T*>(parameters[name].get());
				if (ptr) return *ptr;
				throw FGException((std::string() + "Parameter \"" + name + "\" has wrong type!").c_str());
			}
			throw FGException((std::string() + "Parameter \"" + name + "\" does not exsist!").c_str());
		}
		template<class T>
		void setParameter(const std::string &name, T&& parameter) {
			if (parameters.count(name)) {
				parameters[name].reset(new T{ parameter });
				return;
			}
			throw FGException((std::string() + "Parameter \"" + name + "\" does not exsist!").c_str());
		}

		template<class T>
		void addParameter(const std::string& name, T&& p) {
			if (parameters.count(name)) {
				throw FGException((std::string() + "Parameter \"" + name + "\" already exsist!").c_str());
			}
			parameters[name].reset(new T{ p });
		}
		typedef ISettingHasher hahser;
		typedef ISettingComparer comparer;
	protected:
		virtual void setup() {}
	};

	class FEMCADGEOMSHARED_EXPORT VertexSetting : public ISetting {
	public:
		VertexSetting() :ISetting() { setup(); dim = 0; }
		VertexSetting(const VertexSetting& s) :ISetting(s) { }
		virtual SETTINGHANDLE copy()const {
			return std::make_shared<VertexSetting>(VertexSetting(*this));
		}
	protected:
		virtual void setup() {}
	};
	class FEMCADGEOMSHARED_EXPORT LineSetting : public ISetting {
	public:
		LineSetting() : ISetting() {
			setup(); dim = 1;
		}
		LineSetting(const LineSetting& s) :ISetting(s) { }
		virtual SETTINGHANDLE copy()const {
			return std::make_shared<LineSetting>(LineSetting(*this));
		}
	protected:
		virtual void setup() {
			addParameter("q", DoubleParameter(1.0));
			addParameter("N", DoubleParameter(1.0));
		}
	};

	class FEMCADGEOMSHARED_EXPORT GeometrySetting : public ISetting {
	public:
		GeometrySetting() : ISetting() { setup(); dim = 2;}
		GeometrySetting(const GeometrySetting& s) :ISetting(s) { }
		virtual SETTINGHANDLE copy()const {
			return std::make_shared<GeometrySetting>(GeometrySetting(*this));
		}
	protected:
		virtual void setup() {}
	};

	class FEMCADGEOMSHARED_EXPORT ISettingHasher {
	public:
		int operator()(const ISetting& s)const { return s.getID(); }
	};
	class FEMCADGEOMSHARED_EXPORT ISettingComparer {
	public:
		bool operator()(const SETTINGHANDLE& lhs, const SETTINGHANDLE& rhs)const { return lhs->getID() < rhs->getID(); }
	};

}