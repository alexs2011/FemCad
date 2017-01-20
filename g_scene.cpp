#include "stdafx.h"
#include "g_scene.h"

namespace fg {
	bool equals_eps(const double a, const double b) {
		return (std::fabs(a - b) < FG_EPS);
	}

	int Scene::ID = 0;
	int ISetting::ID = 0;

	bool fg::Geometry::update() { return getContext().update(_handle); }

	Scene::Scene() : isPrimitive(false), root(), geometry(), ownership(), id(ID++), defaultVertex(std::make_shared<VertexSetting>(VertexSetting())),
		defaultLine(std::make_shared<LineSetting>(LineSetting())), defaultCSG(std::make_shared<GeometrySetting>(GeometrySetting())) {
	}

	void fg::Scene::remove(fg::GHANDLE object) {
		for (auto i : ownership[object]) {
			remove(i);
		}
		auto s = geometry[object]->getChildren();
		for (auto i : s) {
			//std::set<GHANDLE>& p = ownership[i];
			ownership[i].erase(object);
		}
		if (object == geometry.size() - 1) {
			ownership.resize(ownership.size() - 1);
			geometry.resize(geometry.size() - 1);
			return;
		}

		auto& last = ownership[geometry.size() - 1];
		for (auto i : last) {
			geometry[i]->replace(geometry.size() - 1, object);
		}
		for (auto i : geometry[geometry.size() - 1]->getChildren()) {
			ownership[i].erase(geometry.size() - 1);
			ownership[i].insert(object);
		}
		geometry[geometry.size() - 1]->setHandle(object);
		std::swap(last, ownership[object]);
		ownership.resize(ownership.size() - 1);
		std::swap(geometry.back(), geometry[object]);
		geometry.resize(geometry.size() - 1);
	}

	void fg::Scene::merge(fg::GHANDLE target, fg::GHANDLE object) {
		//if(typeid(geometry[target].get()) != typeid(geometry[object].get())) throw FGException("Cannot merge objects of different types");
		//auto och = geometry[object]->getChildren();
		//auto tch = geometry[target]->getChildren();
		//if(och.size() != tch.size()) throw FGException("Different children of mergable objects");
		//for(int i = 0; i<och.size(); i++){
		//    if(och[i] != tch[i]) throw FGException("Different children of mergable objects");
		//}
		if (!geometry[target].get()->equals(*geometry[object].get()))
			throw FGException("Different children of mergable objects");

		// [TODO] Setting priority choose

		for (auto i : ownership[object]) {
			geometry[i]->replace(object, target);
			ownership[target].insert(i);
		}
		ownership[object].clear();
		remove(object);
	}

	bool fg::Scene::update(fg::GHANDLE object) {
		bool result = true;
		for (auto i : ownership[object]) {
			result = result && geometry[i]->update();
		}
		return result;
	}

}