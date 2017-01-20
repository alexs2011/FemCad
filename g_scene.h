#pragma once
#include "stdafx.h"
#include "g_geometry.h"
#include "g_transformation.h"

namespace fg {

	class FEMCADGEOMSHARED_EXPORT Scene {
	protected:
		static int ID;
		friend class FEMCADGEOMSHARED_EXPORT IGeometry;
		int id;
		std::vector<std::unique_ptr<IGeometry>> geometry;
		//std::vector<std::shared_ptr<ISetting>> settings;
		std::vector<std::set<GHANDLE>> ownership;
		TransformRoot root;
	public:
		bool isPrimitive;
		SETTINGHANDLE defaultVertex;//VertexSetting defaultVertex;
		SETTINGHANDLE defaultLine;//LineSetting defaultLine;
		SETTINGHANDLE defaultCSG;//GeometrySetting defaultCSG;
		Scene();
		void copy(Scene& s) const {} // TODO
		TransformPtr getRootTransform() {
			return TransformPtr(root);
		}
		TransformRoot& Root() { return root; }

		bool operator ==(const Scene& rhs) const {
			return id == rhs.id;
		}
		bool operator !=(const Scene& rhs) const {
			return id != rhs.id;
		}

		int ownersCount(GHANDLE object) {
			return ownership[object].size();
		}

		void remove(GHANDLE object);

		void merge(GHANDLE target, GHANDLE object);

		//template<class T>
		//SETTINGHANDLE addSetting(T& object) {
		//	static_assert(std::is_base_of<ISetting, T>::value, "Can't add non ISetting type object");
		//	for (size_t i = 0U; i<settings.size(); i++) {
		//		if (*(settings[i].get()) == object) return settings[i]->getHandle();
		//	}
		//
		//	settings.emplace_back(std::make_unique<T>(object));
		//	settings.back()->setHandle(settings.size() - 1);
		//	return settings.back()->getHandle();
		//}

		template<class T>
		GHANDLE add(T&& object) {
			static_assert(std::is_base_of<IGeometry, T>::value, "Can't add non IGeometry type object");
			geometry.emplace_back(std::make_unique<T>(std::move(object)));
			geometry.back()->setHandle(geometry.size() - 1);
			ownership.push_back(std::set<GHANDLE>());
			//geometry.back()->init();
			return geometry.back()->init();
		}
		template<class T>
		GHANDLE add(T&& object, std::vector<GHANDLE> children) {
			static_assert(std::is_base_of<IGeometry, T>::value, "Can't add non IGeometry type object");
			geometry.emplace_back(std::make_unique<T>(std::move(object)));
			geometry.back()->setHandle(geometry.size() - 1);
			ownership.push_back(std::set<GHANDLE>());
			for (auto i = children.begin(); i != children.end(); ++i) {
				if (ownership[*i].count(geometry.size() - 1) == 0)
					ownership[*i].insert(geometry.size() - 1);
			}
			//geometry.back()->init();
			//ownership.push_back(std::vector<const IGeometry*>(children));
			return geometry.back()->init();
		}

		void registerChild(GHANDLE owner, GHANDLE child) {
			assert(owner < geometry.size() && child < geometry.size());
			ownership[child].insert(owner);
		}

		void unregisterChildren(GHANDLE owner) {
			//std::set<int> a;
			//a.
			for (GHANDLE i : get_ptr(owner)->getChildren()) {
				auto s = &ownership[i];
				auto d = s->find(owner);
				if (d != s->end())
					s->erase(d);
			}
		}
		template<class G>
		G& find_first() {
			for (auto i = geometry.begin(); i != geometry.end(); ++i) {
				auto it = i->get();
				G* s = dynamic_cast<G*>(it);
				if (s) return *s;
			}
			throw FGObjectNotFound();
		}

		//ISetting* get(SETTINGHANDLE& setting) {// const{
		//	assert(setting < geometry.size());
		//	return settings[setting].get();
		//}
		template<class G>
		G& get(GHANDLE object) const {
			//static_assert(std::is_base_of<IGeometry, G>::value, "Can't add non IGeometry type object");
			return static_cast<G&>(*get_ptr(object));
		}
		IGeometry* get_ptr(GHANDLE object) const {
			if (object >= geometry.size()) {
				throw FGUndefinedObject();
			}
			//assert(object < geometry.size());
			return geometry[object].get();
		}

		template<class G>
		void for_each(std::function<void(G&)> pred) {
			for (auto i = geometry.begin(); i != geometry.end(); ++i) {
				auto it = i->get();
				G* s = dynamic_cast<G*>(it);
				if (s) pred(*s);
			}
			_applyDeletion();
			_applyMerge();
		}

		bool update(GHANDLE object);

		void onDelete(GHANDLE object) {
			_onDeletion.insert(object);
		}
		void onMerge(GHANDLE target, GHANDLE object) {
			if (_onMerge.count(object)) {
				//onMerge(_onMerge[object], object);
				throw FGException("Failed to merge objects");
			}
			else
				_onMerge[object] = target;
		}
		void applyDelete() {
			_applyDeletion();
		}
		void applyMerge() {
			_applyMerge();
		}
		// OBSOLETE
		//ITransformable* firstTransformableParent(GHANDLE handle) const{
		//    std::queue<GHANDLE> owners;
		//    owners.push(handle);
		//    while(!owners.empty()){
		//        auto i = owners.front();
		//        owners.pop();
		//        if(auto v = dynamic_cast<ITransformable*>(get_ptr(i))){
		//            if(!v->isInherited()) return v;
		//        }
		//        if(ownership[i].size() > 0) owners.push(*ownership[i].begin());
		//        //for(auto j : ownership[i]) owners.push(j);
		//    }
		//    return nullptr;
		//}

		//[TODO] Scene copy();
	protected:
		void _applyDeletion() {
			if (!_onDeletion.empty()) {
				//std::sort(_onDeletion.begin(), _onDeletion.end());
				for (auto i = _onDeletion.rbegin(); i != _onDeletion.rend(); ++i) {
					remove(*i);
					//remapping[geometry.size()-1] = *i;
				}
				_onDeletion.clear();
			}
		}
		void _applyDeletion(std::map<GHANDLE, GHANDLE>& remapping) {
			if (!_onDeletion.empty()) {
				//std::sort(_onDeletion.begin(), _onDeletion.end());
				for (auto i = _onDeletion.rbegin(); i != _onDeletion.rend(); ++i) {
					remove(*i);
					remapping[geometry.size() - 1] = *i;
				}
				_onDeletion.clear();
			}
		}
		void _applyMerge() {
			if (!_onMerge.empty()) {
				//std::sort(_onMerge.begin(), _onMerge.end());
				for (auto i = _onMerge.rbegin(); i != _onMerge.rend(); ++i) {
					merge(i->second, i->first);
				}
				_onMerge.clear();
			}
		}
		std::set<GHANDLE> _onDeletion;
		std::map<GHANDLE, GHANDLE> _onMerge;
	};

}