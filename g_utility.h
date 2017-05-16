#pragma once
#include "g_shape_2d.h"
#include "g_intersector.h"

namespace fg {
	//using namespace primitive;
	class FEMCADGEOMSHARED_EXPORT GeometryUtility {
		using Lookup = lookup_tree<2, ILine*>;
	public:
		//friend Shape::Shape(Scene&, const SETTINGHANDLE&, std::vector<GHANDLE>&&);
		static bool isConvex(const Scene& context, const std::vector<GHANDLE>& geometry) {
			int side = 0;
			for (size_t i = 0U; i < geometry.size(); i++) {
				const IClassifiable* c = static_cast<const IClassifiable*>(context.get_ptr(geometry[i]));
				for (size_t j = i + 1; j < geometry.size(); j++) {
					const IClassifiable* p = static_cast<const IClassifiable*>(context.get_ptr(geometry[j]));
					auto s = c->classify(p->middle());
					if (side == 0 && s != 0) { side = s; continue; }
					if (s != 0 && s != side) return false;
				}
			}
			return true;
		}
		static void subdivideLineSetting(const SETTINGHANDLE& base_setting, double t, SETTINGHANDLE& left, SETTINGHANDLE& right) {
			if (t < -FG_EPS || t > 1.0 + FG_EPS) throw FGException("Line setting subdivision problem. 't' is out of bounds.");
			if (t < FG_EPS) {
				right = base_setting; left = nullptr; return;
			}
			if (t > 1.0 - FG_EPS) {
				left = base_setting; right = nullptr; return;
			}
			try {
				auto N = (int)base_setting->getParameter<DoubleParameter>("N");
				auto q = base_setting->getParameter<DoubleParameter>("q");

				auto n = std::fabs(q - 1.0) < FG_EPS ? std::max(1, (int)(t * N)) : std::max(1, (int)std::floor(std::log(t * (std::pow(q, (double)N) - 1) + 1) / std::log(q)));
				left = base_setting->copy();
				left->setParameter("N", DoubleParameter(n));
				right = base_setting->copy();
				right->setParameter("N", DoubleParameter(std::max(1, N - n)));
			}
			catch (const FGException& ex) {
				return;
			}
		}
		static double getArea(const Scene& context, const std::vector<GHANDLE>& lines) {
			double sum = 0;
			for (auto i : lines) {
				ILine& l = context.get<ILine>(i);
				sum += (l.P0() ^ l.P1()).length();
			}
			return sum;
		}

		static GHANDLE CreateContourShape(Scene& context, SETTINGHANDLE& setting, const Scene& lines_context, const std::vector<GHANDLE>& lines) {
			if (lines.size() < 2) throw FGException("Bad shape. Not enough lines");
			std::set<GHANDLE> verts;
			for (auto i : lines) {
				auto ii = lines_context.get_ptr(i);
				ILine *l;
				if (!(l = dynamic_cast<ILine*>(ii))) throw std::logic_error("Trying to create polygon from not line object");
				auto c = ii->getChildren();
				verts.insert(c.begin(), c.end());
			}
			std::vector<GHANDLE> geometry;
			std::map<GHANDLE, GHANDLE> new_verts;
			for (auto i : verts) {
				new_verts[i] = lines_context.get_ptr(i)->copy(context);
			}
			std::vector<GHANDLE> vx;
			//geometry = lines;
			for (auto i : lines) {
				auto ll = static_cast<ILine*>(lines_context.get_ptr(i));
				vx = ll->getChildren();

				for (size_t j{}; j < vx.size(); ++j)
					vx[j] = new_verts[vx[j]];
				geometry.push_back(ll->createSame(context, ll->getSetting(), vx));
			}
			std::multimap<GHANDLE, std::pair<GHANDLE, bool>> allLines;
			std::map<GHANDLE, int> vertexOwners;

			for (auto i : geometry) {
				auto ii = context.get_ptr(i);
				ILine *l;
				if (!(l = dynamic_cast<ILine*>(ii))) throw std::logic_error("Trying to create polygon from not line object");
				// Хэндлеру начала отрезка ставится в соответствие пара номер отрезка и false/true
				allLines.insert(std::make_pair((l->p0Handle()), std::make_pair(i, false)));
				allLines.insert(std::make_pair((l->p1Handle()), std::make_pair(i, true)));
				// подсчитаем количество владельцев каждого начала и конца отрезка, чтобы определить, замкнутый ли полигон
				vertexOwners[l->p0Handle()] = (vertexOwners.count(l->p0Handle())) ? vertexOwners[l->p0Handle()] + 1 : 1;
				vertexOwners[l->p1Handle()] = (vertexOwners.count(l->p1Handle())) ? vertexOwners[l->p1Handle()] + 1 : 1;
			}
			// замкнутый ли полигон
			for (auto i : vertexOwners) {
				if (i.second % 2)
					throw FGException("Error! Unclosed or non CSG polygon!");
			}
			//
			// Развернем все отрезки так, чтобы их направление обхода полигона было правильным
			//
			// выберем первый отрезок из добавляемых
			GHANDLE last = geometry[0];
			// вытащим этот отрезок из изначальной сцены
			// geometry.push_back(l1));
			ILine * ll = static_cast<ILine*>(context.get_ptr(last));
			while (true) {
				// для конца этого отрезка находим следующий отрезок, повторяем до тех пор, пока не достигнем замыкания
				auto begin = allLines.lower_bound(ll->p1Handle());
				auto end = allLines.upper_bound(ll->p1Handle());
				for (auto i = begin; i != end; i++) {
					if (i->second.first != last) {
						last = i->second.first;
						ll = static_cast<ILine*>(context.get_ptr(last));
						if (i->second.second) {// || !i->second.second) {
							ll->swapOrientation();
						}
						//geometry.push_back(ll->getHandle());// ->copy(s));
						break;
					}
				}

				if (last == geometry[0]) break;
			}

			// посчитаем площадь
			auto a = GeometryUtility::getArea(context, lines);
			int side = a > FG_EPS ? 1 : a < FG_EPS ? -1 : 0;
			if (side == 0) throw FGException("Unable to create polygon with all the points in a line");
			// если площадь вышла отрицательной, то придется развернуть направление всех отрезков
			if (side < 0) {
				for (auto i : geometry) {
					ll = static_cast<ILine*>(context.get_ptr(i));
					ll->swapOrientation();
				}
			}

			return primitive::Shape(context, setting, std::move(geometry)).getHandle();
		}

		static GHANDLE ApplyCSG(Scene& context, CSGOperation op, const Primitive& primitive0, const Primitive& primitive1, std::vector<GHANDLE>* v = nullptr);
	};
}