#pragma once
#include "g_utility.h"
#include "g_line_ext.h"
#include "g_meshview.h"

namespace fg {
	class CSG : public ElementGeometry {
	public:
		Scene scene;
		std::vector<std::pair<GHANDLE, CSGOperation>> forms;
		std::vector<std::shared_ptr<ElementGeometry>> meshes;
		std::vector<MeshedLine> boundaries;

		CSG(const SETTINGHANDLE& setting, const std::vector<std::pair<std::shared_ptr<ElementGeometry>, CSGOperation>>& form) : scene{} {
			std::vector<GHANDLE> result_bounds;
			auto get_sc = [](ElementGeometry& s) -> Scene& {return s.boundary(0).line.getConstContext(); };
			auto get_lines = [](ElementGeometry& s) ->std::vector<GHANDLE> {
				std::vector<GHANDLE> lines;
				for (size_t i{}; i < s.boundary_size(); ++i) {
					lines.push_back(s.boundary(i).line.getHandle());
				}
				return lines;
			};
			auto* p_base = &scene.get<primitive::Shape>(primitive::Shape(scene, setting, get_sc(*form.front().first), get_lines(*form.front().first)).getHandle());
			forms.push_back(std::make_pair(p_base->getHandle(), form.front().second));
			meshes.push_back(form.front().first);
			for (size_t i{ 1U }; i < form.size(); ++i) {
				auto& p_cur = scene.get<primitive::Shape>(primitive::Shape(scene, setting, get_sc(*form[i].first), get_lines(*form[i].first)).getHandle());
				forms.push_back(std::make_pair(p_cur.getHandle(), form[i].second));
				meshes.push_back(form[i].first);
				p_base = &scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(scene, forms[i].second, *p_base, p_cur, &result_bounds));
			}

			for (auto i : result_bounds) {
				auto& l = scene.get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
			for (auto i : p_base->getBoundary()) {
				auto& l = scene.get<ILine>(i);
				boundaries.emplace_back(MeshedLine(l));
				boundaries.back().build();
			}
		}

		size_t cast(const vector3& point) const {
			size_t r = 0xFFFFFFFF;
			for (size_t i{}; i < forms.size(); ++i) {
				auto& shape = scene.get<primitive::Shape>(forms[i].first);
				auto r_cast = shape.classify(point);
				auto op = forms[i].second;
				if (op == CSGOperation::Union) {
					if (r_cast <= 0) {
						r = i;
					}
				}
				if (op == CSGOperation::Intersect) {
					if (r_cast > 0) {
						r = 0xFFFFFFFF;
					}
				}
				if (op == CSGOperation::Subtract) {
					if (r_cast <0) {
						r = 0xFFFFFFFF;
					}
				}
			}
			return r;
		}

		virtual inline double getSize(const vector3& point, double) const {
			auto r = cast(point);
			auto s = meshes[r]->getSize(point, double{});
			if (s >= 0.999e100) return ElementGeometry::getSize(point, double{});
			return r == 0xFFFFFFFF ? 1e100 : s;
		}
		virtual inline vector3 getSize(const vector3& point, vector3) const {
			auto r = cast(point);
			auto s = meshes[r]->getSize(point, vector3{});
			if (s >= 0.999e100) return ElementGeometry::getSize(point, vector3{});
			return r == 0xFFFFFFFF ? vector3::Repeat(1e100) : s;
		}

		virtual MeshedLine boundary(size_t index) const {
			return boundaries[index];
		}
		virtual const size_t boundary_size() const {
			return boundaries.size();
		}
	};
}