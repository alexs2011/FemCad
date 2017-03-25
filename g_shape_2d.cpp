// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "stdafx.h"
#include "g_utility.h"
#include "g_shape_2d.h"

///
/// конструктор фигуры, т.е. полигона
/// Scene & s - та сцена, в которую будет добавлена новая фигура
/// lines_context - та сцена, в которой лежат добавляемые отрезки
/// const std::vector<GHANDLE>& lines - отрезки, из которых будет состоять новая фигура
///
fg::primitive::Shape::Shape(Scene & s, const SETTINGHANDLE& setting, Scene & lines_context, const std::vector<GHANDLE>& lines)
	: Primitive(s, setting)
{
	std::set<GHANDLE> verts;
	std::multimap<GHANDLE, std::pair<GHANDLE, bool>> allLines;
	std::map<GHANDLE, int> vertexOwners;

	for (auto i : lines) {
		auto ii = lines_context.get_ptr(i);
		ILine *l;
		if (!(l = dynamic_cast<ILine*>(ii))) throw std::logic_error("Trying to create polygon from not line object");
		// Хэндлеру начала отрезка ставится в соответствие пара номер отрезка и false/true
		allLines.insert(std::make_pair((l->p0Handle()), std::make_pair(i, false)));
		allLines.insert(std::make_pair((l->p1Handle()), std::make_pair(i, true)));
		// подсчитаем количество владельцев каждого начала и конца отрезка, чтобы определить, замкнутый ли полигон
		vertexOwners[l->p0Handle()] = (vertexOwners.count(l->p0Handle())) ? vertexOwners[l->p0Handle()] + 1 : 1;
		vertexOwners[l->p1Handle()] = (vertexOwners.count(l->p1Handle())) ? vertexOwners[l->p1Handle()] + 1 : 1;
		auto c = ii->getChildren();
		verts.insert(c.begin(), c.end());
	}
	// замкнутый ли полигон
	for (auto i : vertexOwners) {
		if (i.second != 2) 
			throw FGException("Error! Unclosed or non CSG polygon!");
	}

	//
	// Развернем все отрезки так, чтобы их направление обхода полигона совпадало
	//
	// выберем первый отрезок из добавляемых
	GHANDLE last = lines[0];
	// вытащим этот отрезок из изначальной сцены
	ILine * ll = static_cast<ILine*>(lines_context.get_ptr(last));
	// geometry.push_back(l1));
	while (true) {
		// для конца этого отрезка вытащим все пары номера этого отрезка и false/true ???
		auto begin = allLines.lower_bound(ll->p1Handle());
		auto end = allLines.upper_bound(ll->p1Handle());
		for (auto i = begin; i != end; i++) {
			if (i->second.first != last) {
				last = i->second.first;
				ll = static_cast<ILine*>(lines_context.get_ptr(last));
				if (i->second.second) {// || !i->second.second) {
					ll->swapOrientation();
				}
				geometry.push_back(ll->getHandle());// ->copy(s));
				break;
			}
		}

		if (last == lines[0]) break;
	}

	// посчитаем площадь
	auto a = GeometryUtility::getArea(lines_context, lines);
	int side = a > FG_EPS ? 1 : a < FG_EPS ? -1 : 0;
	if (side == 0) throw FGException("Unable to create polygon with all the points in a line");
	// если площадь вышла отрицательной, то придется развернуть направление всех отрезков
	if (side < 0) {
		for (auto i : geometry) {
			ll = static_cast<ILine*>(lines_context.get_ptr(i));
			ll->swapOrientation();
		}
	}

	addSelfToContext();
}

inline bool fg::primitive::Shape::isConvex() const {
	return GeometryUtility::isConvex(getConstContext(), geometry);
}
