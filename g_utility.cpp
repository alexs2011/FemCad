﻿// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "g_utility.h"
namespace fg {
	// получить объект, созданный с помощью CSG операции, примененной к двум примитивам
	GHANDLE fg::GeometryUtility::ApplyCSG(Scene & context, CSGOperation op, const Primitive & primitive0, const Primitive & primitive1, std::vector<GHANDLE>* internal_boundary) {
		Scene resultContext;
		Scene tempContext;
		std::unique_ptr<Lookup> tree0;
		std::vector<std::pair<rect, ILine*>> bounds;
		auto& Shape0 = tempContext.get<Primitive>(primitive0.instantiate(tempContext));
		auto& Shape1 = tempContext.get<Primitive>(primitive1.instantiate(tempContext));
		bool setting_difference = Shape0.getSetting()->getID() != Shape1.getSetting()->getID();

		bounds.reserve(std::max(Shape0.getConstBoundary().size(), Shape1.getConstBoundary().size()) + (internal_boundary ? internal_boundary->size() : 0));
		rect whole;
		// соответствие между линиями и точками на линиях
		std::map<const ILine*, std::vector<GHANDLE>> points_on_lines0, points_on_lines1, points_on_bounds;
		std::map<vector3, GHANDLE> points;
		//std::vector<vector3> point_array;
		int last_p = 0;
		// Построение квадродерева
		auto boundary = Shape0.getConstBoundary();
		for (size_t i = 0; i < boundary.size(); i++) {
			auto line = dynamic_cast<ILine*>(Shape0.getConstContext().get_ptr(boundary[i]));
			for (auto j : line->getChildren()) {
				// перенос точек на новый контекст
				auto& point = Shape0.getConstContext().get<Vertex>(j).position();
				// если уже добавлена, не добавлять
				if (points.count(point) == 0) {
					points[point] = Vertex(resultContext, resultContext.defaultVertex, point).getHandle();
				}
			}
			/*for (auto j : line->getBoundary()) {
				auto& point = Shape0.getConstContext().get<Vertex>(j).position();
				points_on_lines0[line].push_back(points[point]);
			}*/
			auto p0 = line->P0();
			auto p1 = line->P1();
			//if (points.count(p0) == 0) { points[p0] = Vertex(resultContext, resultContext.defaultVertex, p0).getHandle(); }
			//if (points.count(p1) == 0) { points[p1] = Vertex(resultContext, resultContext.defaultVertex, p1).getHandle(); }

			// вокруг линий строятся ограничивающие объекты (прямоугольники)
			points_on_lines0[line].insert(std::end(points_on_lines0[line]), { points[p0], points[p1] });
			auto box = line->getBoundingRect();
			bounds.push_back(std::make_pair(box, line));
			// увеличиваются размеры области, в которой лежат объекты
			whole = whole.add_rect(box);
		}
		// дерево дробит области в котороых есть объекты на 4 подобласти до тех пор, пока в этих подобластях имеются объекты
		tree0 = std::make_unique<Lookup>(Lookup(100, std::move(bounds), whole.Min(), whole.Max()));


		std::set<ILine*> geoms;
		std::vector<vector3> p0;
		std::vector<vector3> p1;
		//
		std::set<ILine*> boundaryLines;
		if (internal_boundary) {
			for (size_t i = 0; i < internal_boundary->size(); i++) {
				auto line = dynamic_cast<ILine*>(primitive0.getConstContext().get_ptr((*internal_boundary)[i]));
				boundaryLines.insert(line);
				for (auto j : line->getChildren()) {
					// перенос точек на новый контекст
					auto& point = primitive0.getConstContext().get<Vertex>(j).position();
					// если уже добавлена, не добавлять
					if (points.count(point) == 0) {
						points[point] = Vertex(resultContext, resultContext.defaultVertex, point).getHandle();
					}
				}
				auto pp0 = line->P0();
				auto pp1 = line->P1();

				points_on_bounds[line].insert(std::end(points_on_bounds[line]), { points[pp0], points[pp1] });
				// вокруг линий строятся ограничивающие объекты (прямоугольники)
				auto box = line->getBoundingRect();
				bounds.push_back(std::make_pair(box, line));
				tree0->get_overlap(box, geoms);
				for (auto l : geoms) {
					// находим точки пересечения
					//auto l = dynamic_cast<ILine*>(Shape0.getConstContext().get_ptr(j));
					auto ir = Intersector<ILine>::intersect_dynamic(p0, p1, *line, *l);
					for (auto k : p0) {
						if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
						points_on_bounds[line].push_back(points[k]);
					}
					for (auto k : p1) {
						if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
						points_on_lines0[l].push_back(points[k]);
					}
					p0.clear();
					p1.clear();
				}
				geoms.clear();
				// увеличиваются размеры области, в которой лежат объекты
				whole = whole.add_rect(box);
			}
			tree0 = std::make_unique<Lookup>(Lookup(100, std::move(bounds), whole.Min(), whole.Max()));
		}

		p0.reserve(4); p1.reserve(4);
		// Нахождение пересечений
		for (size_t i = 0; i < Shape1.getConstBoundary().size(); i++) {
			auto line = dynamic_cast<ILine*>(Shape1.getConstContext().get_ptr(Shape1.getConstBoundary()[i]));
			// перенос точек на новый контекст
			for (auto j : line->getChildren()) {
				auto& point = Shape1.getConstContext().get<Vertex>(j).position();
				// если уже добавлена, не добавлять
				if (points.count(point) == 0) { points[point] = Vertex(resultContext, resultContext.defaultVertex, point).getHandle(); }
			}
			auto pp0 = line->P0();
			auto pp1 = line->P1();
			points_on_lines1[line].insert(std::end(points_on_lines1[line]), { points[pp0], points[pp1] });
			//if (points.count(pp0) == 0) { points[pp0] = Vertex(resultContext, resultContext.defaultVertex, pp0).getHandle(); }
			//if (points.count(pp1) == 0) { points[pp1] = Vertex(resultContext, resultContext.defaultVertex, pp1).getHandle(); }
			auto box = line->getBoundingRect();
			tree0->get_overlap(box, geoms);
			for (auto l : geoms) {
				// находим точки пересечения
				//auto l = dynamic_cast<ILine*>(Shape0.getConstContext().get_ptr(j));
				auto ir = Intersector<ILine>::intersect_dynamic(p0, p1, *line, *l);
				for (auto k : p0) {
					if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
					points_on_lines1[line].push_back(points[k]);
				}
				if (boundaryLines.count(l)) {
					for (auto k : p1) {
						if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
						points_on_bounds[l].push_back(points[k]);
					}
				}
				else {
					for (auto k : p1) {
						if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
						points_on_lines0[l].push_back(points[k]);
					}
				}
				p0.clear();
				p1.clear();
			}
			geoms.clear();
		}
		std::vector<GHANDLE> lines0, lines1, lines_bnd;

		// Построение субсегментов (раздробить геометрию найденными точками пересечения)
		// m - все точки на текущей форме
		// flip - если true, то результирующие линии будут развернуты относительно линий в m (только для опер. разность) 
		auto func = [&](std::vector<GHANDLE>& result, std::map<const ILine*, std::vector<GHANDLE>>& m, bool flip) {
			// временный массив соответствия точкам их хендлам. Нужен для сортировки точек вдоль линии
			static std::vector<std::pair<vector3, GHANDLE>> sorter;
			sorter.reserve(16);
			for (auto i : m) {
				sorter.clear();
				// множество точек на текущей линии
				std::set<GHANDLE> s(std::begin(i.second), std::end(i.second));
				for (auto j : s) { sorter.push_back(std::make_pair(resultContext.get<Vertex>(j).position(), j)); }
				//i.second = std::vector<GHANDLE>(std::begin(s), std::end(s));
				// сортирует точки вдоль линии i
				i.first->sortAlong(sorter);
				i.second.resize(sorter.size());
				for (size_t j = 0; j < sorter.size(); j++) {
					i.second[j] = sorter[j].second;
				}
				// создаем сегменты
				auto seg = dynamic_cast<const LineSegment*>(i.first);
				if (seg) {
					SETTINGHANDLE right = seg->getSetting();
					SETTINGHANDLE left = seg->getSetting();
					for (size_t j = 0U; j < i.second.size() - 1; j++) {
						auto& p = resultContext.get<Vertex>(i.second[j + 1]);
						auto t = i.first->getParam(p.position());
						GeometryUtility::subdivideLineSetting(right, t, left, right);
						if (left == nullptr) {
							std::cout << "ASDASD";
						}
						result.push_back(LineSegment(resultContext, left, i.second[j + flip], i.second[j + !flip]).getHandle());
					}
				}
				auto segel = dynamic_cast<const EllipticSegment*>(i.first);
				if (segel) {
					SETTINGHANDLE right = segel->getSetting();
					SETTINGHANDLE left = segel->getSetting();
					for (size_t j = 0; j < i.second.size() - 1; j++) {
						auto& p = resultContext.get<Vertex>(i.second[j + 1]);
						auto t = i.first->getParam(p.position());
						GeometryUtility::subdivideLineSetting(right, t, left, right);
						if (left == nullptr) {
							std::cout << "ASDASD";
						}
						result.push_back(EllipticSegment(resultContext, left, i.second[j + flip], i.second[j + !flip], points[Shape0.getConstContext().get<Vertex>(segel->centerHandle()).position()], segel->getCurve()).getHandle());
					}
				}
			}
		};
		func(lines0, points_on_lines0, false);
		func(lines1, points_on_lines1, op == CSGOperation::Subtract && !setting_difference);

		func(lines_bnd, points_on_bounds, false);

		std::map<std::pair<GHANDLE, GHANDLE>, size_t> lines1_map, lines0_map;

		for (size_t i{}; i < lines0.size(); ++i) {
			auto& l = resultContext.get<ILine>(lines0[i]);
			lines0_map[std::make_pair(l.p0Handle(), l.p1Handle())] = i;
		}
		for (size_t i{}; i < lines1.size(); ++i) {
			auto& l = resultContext.get<ILine>(lines1[i]);
			lines1_map[std::make_pair(l.p0Handle(), l.p1Handle())] = i;
		}

		std::vector<GHANDLE> result;
		// КСГ


		std::vector<GHANDLE> tmp_bnd;

		if (internal_boundary) {
			/*for (auto i : *internal_boundary) {
				internal_boundary_context->remove(i);
			}*/
			internal_boundary->clear();
		}
		if (!setting_difference) {
			for (auto i : lines0) {
				auto& l = resultContext.get<ILine>(i);
				auto c = l.middle();
				// определяем, где находится линия относительно второго объекта
				int seconds = Shape1.classify(c);
				if (seconds == 0) {
					if (lines1_map.count(std::make_pair(l.p0Handle(), l.p1Handle())))
						result.push_back(i);

					continue;
				}
				if (op == CSGOperation::Union) {
					if (seconds > 0) result.push_back(i); continue;
					/*if (internal_boundary && seconds < 0) {
						tmp_bnd.push_back(i);
					}*/
				}
				if (op == CSGOperation::Subtract) {
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Intersect) {
					if (seconds < 0) result.push_back(i); continue;
				}
			}

			for (auto i : lines1) {
				auto c = resultContext.get<ILine>(i).middle();
				int seconds = Shape0.classify(c);
				if (op == CSGOperation::Union) {
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Intersect || op == CSGOperation::Subtract) {
					/*if (internal_boundary && seconds > 0) {
						tmp_bnd.push_back(i);
					}*/
					if (seconds < 0) result.push_back(i); continue;
				}
			}
			if (internal_boundary) {
				for (auto i : lines_bnd) {
					auto& l = resultContext.get<ILine>(i);
					auto c = l.middle();
					// определяем, где находится линия относительно второго объекта
					int sh0 = Shape0.classify(c);
					int sh1 = Shape1.classify(c);

					if (op == CSGOperation::Union && sh0 < 0) {
						tmp_bnd.push_back(i); continue;
					}
					if (op == CSGOperation::Subtract && sh1 > 0 && sh0 < 0) {
						tmp_bnd.push_back(i); continue;
					}
					if (op == CSGOperation::Intersect && sh0 < 0 && sh1 < 0) {
						tmp_bnd.push_back(i); continue;
					}
				}
			}
		}
		else {
			for (auto i : lines0) {
				auto& l = resultContext.get<ILine>(i);
				auto c = l.middle();
				// определяем, где находится линия относительно второго объекта
				int seconds = Shape1.classify(c);
				if (op == CSGOperation::Union) {
					if (seconds == 0) {
						if (lines1_map.count(std::make_pair(l.p0Handle(), l.p1Handle())))
							result.push_back(i);
						if (lines1_map.count(std::make_pair(l.p1Handle(), l.p0Handle())))
							tmp_bnd.push_back(i);
						continue;
					}

					if (internal_boundary && seconds < 0) {
						tmp_bnd.push_back(i);
					}
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Subtract) {
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Intersect) {
					if (seconds == 0) {
						if (lines1_map.count(std::make_pair(l.p1Handle(), l.p0Handle())))
							result.push_back(i);
						continue;
					}
					result.push_back(i); continue;
				}
			}

			for (auto i : lines1) {
				auto& l = resultContext.get<ILine>(i);
				auto c = l.middle();
				int seconds = Shape0.classify(c);
				if (op == CSGOperation::Union) {
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Subtract) {
					if (internal_boundary && seconds < 0) {
						tmp_bnd.push_back(i);
					}
					if (seconds == 0) {
						if (lines0_map.count(std::make_pair(l.p0Handle(), l.p1Handle())))
							result.push_back(i);
						if (lines0_map.count(std::make_pair(l.p1Handle(), l.p0Handle())))
							tmp_bnd.push_back(i);
						continue;
					}
					if (seconds > 0) result.push_back(i); continue;
				}
				if (op == CSGOperation::Intersect) {
					if (internal_boundary && seconds < 0) {
						tmp_bnd.push_back(i);
					}
					if (seconds == 0) {
						if (lines0_map.count(std::make_pair(l.p0Handle(), l.p1Handle())))
							result.push_back(i);
					}
					//if (seconds < 0) result.push_back(i); continue;
				}
			}
			if (internal_boundary) {
				for (auto i : lines_bnd) {
					auto& l = resultContext.get<ILine>(i);
					auto c = l.middle();
					// определяем, где находится линия относительно второго объекта
					int sh0 = Shape0.classify(c);
					int sh1 = Shape1.classify(c);

					if (op == CSGOperation::Union && sh0 < 0) {
						tmp_bnd.push_back(i); continue;
					}
					if (op == CSGOperation::Subtract && sh1 > 0 && sh0 < 0) {
						tmp_bnd.push_back(i); continue;
					}
					if (op == CSGOperation::Intersect && sh0 < 0 && sh1 > 0) {
						tmp_bnd.push_back(i); continue;
					}
				}
			}
		}

		if (internal_boundary) {
			std::map<GHANDLE, GHANDLE> new_points;
			for (auto i : tmp_bnd) {
				auto& l = resultContext.get<ILine>(i);
				for (auto p : l.getChildren()) {
					if (new_points.count(p) == 0) {
						auto& point = resultContext.get<Vertex>(p);
						new_points[p] = Vertex(context, point.getSetting(), point.position()).getHandle();
					}
				}
			}
			for (auto i : tmp_bnd) {
				auto& l = resultContext.get<ILine>(i);
				std::vector<GHANDLE> remap;
				for (auto j : l.getChildren())
					remap.push_back(new_points[j]);
				internal_boundary->push_back(l.createSame(context, l.getSetting(), remap));
			}
		}

		// Удаление копий
		tempContext.remove(Shape0.getHandle());
		tempContext.remove(Shape1.getHandle());
		auto resultShape = resultContext.get<primitive::Shape>(fg::primitive::Shape(resultContext, primitive0.getSetting(), resultContext, result).getHandle()).instantiate(context);


		return resultShape;
	}
}