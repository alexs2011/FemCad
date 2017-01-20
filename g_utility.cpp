#include "stdafx.h"
#include "g_utility.h"
namespace fg {
	GHANDLE fg::GeometryUtility::ApplyCSG(Scene & context, CSGOperation op, const Primitive & primitive0, const Primitive & primitive1) {
		Scene resultContext;
		Scene tempContext;
		std::unique_ptr<Lookup> tree0;
		std::vector<std::pair<rect, GHANDLE>> bounds;
		auto& Shape0 = tempContext.get<Primitive>(primitive0.instantiate(tempContext));
		auto& Shape1 = tempContext.get<Primitive>(primitive1.instantiate(tempContext));

		bounds.reserve(std::max(Shape0.getConstBoundary().size(), Shape1.getConstBoundary().size()));
		rect whole;
		std::map<const ILine*, std::vector<GHANDLE>> points_on_lines0, points_on_lines1;
		std::map<vector3, GHANDLE> points;
		//std::vector<vector3> point_array;
		int last_p = 0;
		// Построение октодерева
		auto boundary = Shape0.getConstBoundary();
		for (size_t i = 0; i < boundary.size(); i++) {
			auto line = dynamic_cast<ILine*>(Shape0.getConstContext().get_ptr(boundary[i]));
			for (auto j : line->getChildren()) {
				auto& point = Shape0.getConstContext().get<Vertex>(j).position();
				if (points.count(point) == 0) { points[point] = Vertex(resultContext, resultContext.defaultVertex, point).getHandle(); }
				points_on_lines0[line].push_back(points[point]);
			}

			auto p0 = line->P0();
			auto p1 = line->P1();
			//if (points.count(p0) == 0) { points[p0] = Vertex(resultContext, resultContext.defaultVertex, p0).getHandle(); }
			//if (points.count(p1) == 0) { points[p1] = Vertex(resultContext, resultContext.defaultVertex, p1).getHandle(); }

			points_on_lines0[line].insert(std::end(points_on_lines0[line]), { points[p0], points[p1] });
			auto box = line->getBoundingRect();
			bounds.push_back(std::make_pair(box, line->getHandle()));
			whole = whole.add_rect(box);
		}
		tree0 = std::make_unique<Lookup>(Lookup(100, std::move(bounds), whole.Min(), whole.Max()));

		std::set<GHANDLE> geoms;
		std::vector<vector3> p0;
		std::vector<vector3> p1;
		p0.reserve(4); p1.reserve(4);
		// Нахождение пересечений
		for (size_t i = 0; i < Shape1.getConstBoundary().size(); i++) {
			auto line = dynamic_cast<ILine*>(Shape1.getConstContext().get_ptr(Shape1.getConstBoundary()[i]));
			for (auto j : line->getChildren()) {
				auto& point = Shape1.getConstContext().get<Vertex>(j).position();
				if (points.count(point) == 0) { points[point] = Vertex(resultContext, resultContext.defaultVertex, point).getHandle(); }
			}
			auto pp0 = line->P0();
			auto pp1 = line->P1();
			points_on_lines1[line].insert(std::end(points_on_lines1[line]), { points[pp0], points[pp1] });
			//if (points.count(pp0) == 0) { points[pp0] = Vertex(resultContext, resultContext.defaultVertex, pp0).getHandle(); }
			//if (points.count(pp1) == 0) { points[pp1] = Vertex(resultContext, resultContext.defaultVertex, pp1).getHandle(); }
			auto box = line->getBoundingRect();
			tree0->get_overlap(box, geoms);
			for (auto j : geoms) {
				auto l = dynamic_cast<ILine*>(Shape0.getConstContext().get_ptr(j));
				auto ir = Intersector<ILine>::intersect_dynamic(p0, p1, *line, *l);
				for (auto k : p0) {
					if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
					points_on_lines1[line].push_back(points[k]);
				}
				for (auto k : p1) {
					if (points.count(k) == 0) { points[k] = Vertex(resultContext, resultContext.defaultVertex, k).getHandle(); }
					points_on_lines0[l].push_back(points[k]);
				}
				p0.clear();
				p1.clear();
			}
			geoms.clear();
		}
		std::vector<GHANDLE> lines0, lines1;

		// Построение субсегментов
		auto func = [&](std::vector<GHANDLE>& result, std::map<const ILine*, std::vector<GHANDLE>>& m, bool flip) {
			std::vector<std::pair<vector3, GHANDLE>> sorter;
			sorter.reserve(16);
			for (auto i : m) {
				sorter.clear();
				std::set<GHANDLE> s(std::begin(i.second), std::end(i.second));
				for (auto j : s) { sorter.push_back(std::make_pair(resultContext.get<Vertex>(j).position(), j)); }
				//i.second = std::vector<GHANDLE>(std::begin(s), std::end(s));
				i.first->sortAlong(sorter);
				i.second.resize(sorter.size());
				for (size_t j = 0; j < sorter.size(); j++) {
					i.second[j] = sorter[j].second;
				}
				auto seg = dynamic_cast<const LineSegment*>(i.first);
				if (seg) {
					for (size_t j = 0U; j < i.second.size() - 1; j++)
						result.push_back(LineSegment(resultContext, seg->getSetting(), i.second[j + flip], i.second[j + !flip]).getHandle());
				}
				auto segel = dynamic_cast<const EllipticSegment*>(i.first);
				if (segel) {
					for (size_t j = 0; j < i.second.size() - 1; j++)
						result.push_back(EllipticSegment(resultContext, segel->getSetting(), i.second[j + flip], i.second[j + !flip], points[Shape0.getConstContext().get<Vertex>(segel->centerHandle()).position()], segel->getCurve()).getHandle());
				}
			}
		};
		func(lines0, points_on_lines0, false);
		func(lines1, points_on_lines1, op == CSGOperation::Subtract);

		std::vector<GHANDLE> result;
		// КСГ
		for (auto i : lines0) {
			auto c = resultContext.get<ILine>(i).middle();
			int seconds = Shape1.classify(c);
			if (op == CSGOperation::Union || op == CSGOperation::Subtract) {
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
				if (seconds < 0) result.push_back(i); continue;
			}
		}
		tempContext.remove(Shape0.getHandle());
		tempContext.remove(Shape1.getHandle());
		auto resultShape = resultContext.get<primitive::Shape>(fg::primitive::Shape(resultContext, primitive0.getSetting(), resultContext, result).getHandle()).instantiate(context);
		// Удаление копий

		return resultShape;
	}
}