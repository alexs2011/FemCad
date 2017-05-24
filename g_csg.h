#pragma once
#include "g_utility.h"
#include "g_line_ext.h"
#include "g_meshview.h"

namespace fg {
	class CSG : public ElementGeometry {
	public:
		std::shared_ptr<Scene> scene;
		std::vector<std::pair<std::pair<std::shared_ptr<Scene>, GHANDLE>, CSGOperation>> forms;
		std::vector<std::shared_ptr<ElementGeometry>> meshes;
		std::vector<MeshedLine> boundaries;

		CSG(const SETTINGHANDLE& setting, const std::vector<std::pair<std::shared_ptr<ElementGeometry>, CSGOperation>>& form) : scene{ std::make_unique<Scene>() } {
			std::vector<GHANDLE> result_bounds;
			auto get_sc = [](ElementGeometry& s) -> Scene& {return s.boundary(0).line.getConstContext(); };
			auto get_lines = [](ElementGeometry& s) ->std::vector<GHANDLE> {
				std::vector<GHANDLE> lines;
				for (size_t i{}; i < s.boundary_size(); ++i) {
					lines.push_back(s.boundary(i).line.getHandle());
				}
				return lines;
			};
			auto tmp_scene0 = std::make_shared<Scene>();
			auto tmp_scene1 = std::make_shared<Scene>();
			auto* p_base = &tmp_scene0->get<primitive::Shape>(primitive::Shape(*tmp_scene0, setting, get_sc(*form.front().first), get_lines(*form.front().first)).getHandle());
			forms.push_back(std::make_pair(std::make_pair(tmp_scene0, p_base->getHandle()), form.front().second));
			meshes.push_back(form.front().first);
			for (size_t i{ 1U }; i < form.size(); ++i) {
				auto& p_cur = tmp_scene1->get<primitive::Shape>(primitive::Shape(*tmp_scene1, setting, get_sc(*form[i].first), get_lines(*form[i].first)).getHandle());
				forms.push_back(std::make_pair(std::make_pair(tmp_scene1, p_cur.getHandle()), form[i].second));
				meshes.push_back(form[i].first);
				p_base = &scene->get<primitive::Shape>(GeometryUtility::ApplyCSG(*scene, forms[i].second, *p_base, p_cur, &result_bounds));
				tmp_scene0.swap(scene);
				tmp_scene1.reset(new Scene());
				scene.reset(new Scene());
			}
			scene.swap(tmp_scene0);

			for (auto i : result_bounds) {
				auto& l = scene->get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
			for (auto i : p_base->getBoundary()) {
				auto& l = scene->get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
		}
		CSG(const std::vector<std::tuple<std::shared_ptr<ElementGeometry>, CSGOperation, SETTINGHANDLE>>& form) : scene{ std::make_unique<Scene>() } {
			std::vector<GHANDLE> result_bounds;
			auto get_sc = [](ElementGeometry& s) -> Scene& {return s.boundary(0).line.getConstContext(); };
			auto get_lines = [](ElementGeometry& s) ->std::vector<GHANDLE> {
				std::vector<GHANDLE> lines;
				for (size_t i{}; i < s.boundary_size(); ++i) {
					lines.push_back(s.boundary(i).line.getHandle());
				}
				return lines;
			};
			auto tmp_scene0 = std::make_shared<Scene>();
			auto tmp_scene1 = std::make_shared<Scene>();
			auto* p_base = &tmp_scene0->get<primitive::Shape>(primitive::Shape(*tmp_scene0, std::get<2>(form.front()), 
				get_sc(*std::get<0>(form.front())), get_lines(*std::get<0>(form.front()))).getHandle());
			forms.push_back(std::make_pair(std::make_pair(tmp_scene0, p_base->getHandle()), std::get<1>(form.front())));
			meshes.push_back(std::get<0>(form.front()));
			for (size_t i{ 1U }; i < form.size(); ++i) {
				auto& p_cur = tmp_scene1->get<primitive::Shape>(primitive::Shape(*tmp_scene1, std::get<2>(form[i]), 
					get_sc(*std::get<0>(form[i])), get_lines(*std::get<0>(form[i]))).getHandle());
				forms.push_back(std::make_pair(std::make_pair(tmp_scene1, p_cur.getHandle()), std::get<1>(form[i])));
				meshes.push_back(std::get<0>(form[i]));
				p_base = &scene->get<primitive::Shape>(GeometryUtility::ApplyCSG(*scene, forms[i].second, *p_base, p_cur, &result_bounds));
				tmp_scene0.swap(scene);
				tmp_scene1.reset(new Scene());
				scene.reset(new Scene());
			}
			scene.swap(tmp_scene0);

			for (auto i : result_bounds) {
				auto& l = scene->get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
			for (auto i : p_base->getBoundary()) {
				auto& l = scene->get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
		}

		std::pair<size_t, SETTINGHANDLE> cast(const vector3& point) const {
			size_t r = 0xFFFFFFFF;
			SETTINGHANDLE r_s = nullptr;
			for (size_t i{}; i < forms.size(); ++i) {
				auto& shape = forms[i].first.first->get<primitive::Shape>(forms[i].first.second);
				auto r_cast = shape.classify(point);
				auto op = forms[i].second;
				if (r_s == nullptr || shape.getSetting()->getID() == r_s->getID()) {
					if (op == CSGOperation::Union) {
						if (r_cast <= 0) {
							r = i;
							r_s = shape.getSetting();
						}
					}
					if (op == CSGOperation::Intersect) {
						if (r_cast > 0) {
							r = 0xFFFFFFFF;
							r_s = nullptr;
						}
					}
					if (op == CSGOperation::Subtract) {
						if (r_cast <0) {
							r = 0xFFFFFFFF;
							r_s = nullptr;
						}
					}
				}
				else {
					if (op == CSGOperation::Intersect) {
						if (r_cast < 0) {
							r = i;
							r_s = shape.getSetting();
						}
					}
					if (op == CSGOperation::Subtract) {
						if (r_cast < 0) {
							r = i;
							r_s = shape.getSetting();
						}
					}
				}
			}
			return std::make_pair(r, r_s);
		}

		virtual inline double getSize(const vector3& point, double) const {
			auto r = cast(point).first;
			if (r == 0xFFFFFFFF) return 1e100;
			auto s = meshes[r]->getSize(point, double{});
			if (s >= 0.999e100 || std::isinf(s)) 
				return ElementGeometry::getSize(point, double{});
			return r == 0xFFFFFFFF ? 1e100 : s;
		}
		virtual inline vector3 getSize(const vector3& point, vector3) const {
			auto r = cast(point).first;
			if (r == 0xFFFFFFFF) return vector3::Repeat(1e100);
			auto s = meshes[r]->getSize(point, vector3{});
			if (s >= 0.999e100 || std::isinf(s.x) || std::isinf(s.y) || std::isinf(s.z)) 
				return ElementGeometry::getSize(point, vector3{});
			return r == 0xFFFFFFFF ? vector3::Repeat(1e100) : s;
		}

		virtual inline SETTINGHANDLE getSetting(const vector3& point) const {
			return cast(point).second;
		}

		virtual size_t materialId(const vector3& p)const {
			auto s = getSetting(p);
			return s ? s->getID() : 0; 
		}
		virtual std::map<size_t, SETTINGHANDLE> listSettings() const {
			std::map<size_t, SETTINGHANDLE> settings;
			for (auto i : boundaries) {
				auto s = i.line.getSetting();
				try {
					if (s->getParameterByName("Name") && settings.count(s->getID()))
						settings[s->getID()] = s;
				}
				catch (const FGException&) {}
			}
			for (auto i : forms) {
				auto s = i.first.first->get<primitive::Shape>(i.first.second).getSetting();
				try {
					if (s->getParameterByName("Name") && settings.count(s->getID()))
						settings[s->getID()] = s;
				}
				catch (const FGException&) {}
			}
			return settings;
		}

		virtual MeshedLine boundary(size_t index) const {
			return boundaries[index];
		}
		virtual const size_t boundary_size() const {
			return boundaries.size();
		}
	};
}