#pragma once
#include "g_bsp.h"
#include "g_primitive.h"

namespace fg {
	namespace primitive {
		// некоторая замкнутая совокупность отрезков и кривых
		class FEMCADGEOMSHARED_EXPORT Shape : public Primitive, public IPrintable
		{
			std::unique_ptr<BSPTree<ILine>> bsp;
		protected:
			// для чего этот конструктор ???
			Shape(Scene& s, const SETTINGHANDLE& setting, std::vector<GHANDLE>&& geom)
				: Primitive(s, setting, geom) {
				addSelfToContext();
			}
			void RebuildBSP()
			{
				// если уже есть bsp-дерево, то уничтожим его
				if (bsp) bsp.reset();
				// сложим в lines все объекты линий из геометрии shape
				std::vector<Splitter<ILine>> lines;
				for (auto i : geometry)
					lines.push_back(Splitter<ILine>(getConstContext().get<ILine>(i), 0.0, 1.0));
				// построим новое дерево BSP
				bsp = std::make_unique<BSPTree<ILine>>(BSPTree<ILine>(lines));
			}
		public:
			virtual std::string toString() const {
				std::string result = "Shape { ";

				for (size_t i = 0U; i < geometry.size(); i++) {
					result += ", " + std::to_string(geometry[i]);
				}
				return result + "}";
			}
			Shape(Shape&& p) : Primitive(std::move(p)) {  }
			// Create from polyline
			Shape(Scene& s, const SETTINGHANDLE& setting, const std::vector<GHANDLE>& points)
				: Primitive(s, setting) {
				size_t N = points.size();
				for (size_t i = 0U; i < N; i++) {
					geometry.push_back(LineSegment(s, s.defaultLine, points[i], points[(i + 1) % N]).getHandle());
				}

				addSelfToContext();
			}
			Shape(Scene& s, const SETTINGHANDLE& setting, Scene& lines_context, const std::vector<GHANDLE>& lines);
			virtual std::vector<GHANDLE> getChildren() const {
				auto v = std::vector<GHANDLE>(geometry);
				return v;
			}
			virtual GHANDLE init() {
				RebuildBSP();
				return getHandle();
			}
			virtual void applyTransform(const matrix4x4& m) {
				std::set<GHANDLE> vertice;
				for (size_t i = 0U; i < geometry.size(); i++) {
					auto& g = getContext().get<Geometry>(geometry[i]);
					g.applyTransform(m);
					auto c = g.getChildren();
					vertice.insert(c.begin(), c.end());
				}
				for (auto i : vertice) {
					auto& g = getContext().get<Geometry>(geometry[i]);
					g.applyTransform(m);
				}
			}
			virtual bool isConvex() const;
			virtual int classify(const vector3 &p) const {
				return bsp->classify(p);
			}
			virtual vector3 middle() const {
				throw std::runtime_error("Unimplemented");
			}
			virtual std::vector<GHANDLE> getGeometry() const {
				return geometry;
			}

			virtual inline rect getBoundingRect() const {
				auto r = rect{};
				for (size_t i = 0U; i < geometry.size(); i++) {
					r.add_rect(getConstContext().get<Geometry>(geometry[i]).getBoundingRect());
				}
				return r;
			}
		protected:
			virtual void addSelfToContext() {
				_handle = getContext().add(std::move(*this), geometry);
			}

		public:
			// копирует объект на контекст
			virtual GHANDLE instantiate(Scene& s) const {
				return copy(s);
			}

			virtual void replace(GHANDLE o, GHANDLE n) {
				if (_handle == o) {
					_handle = n;
				}
				for (auto& i : geometry) {
					if (i == o) i = n;
					else getContext().get_ptr(i)->replace(o, n);
				}
				RebuildBSP();
			}
			virtual GHANDLE copy() const {
				return copy(getConstContext());
			}
			virtual GHANDLE copy(Scene &context) const {
				//try {
				//	getConstContext().get_ptr(bsp_root)->copy(context);
				//}
				//catch (BSPNode<ILine>::CopyResultException e) {
				//	std::vector<GHANDLE> copys;
				//	context.get<BSPNode<ILine>&>(e.first).getLines(copys);
				//	//for(auto i : copys){
				//    context.registerChild(getHandle(), i);
				//}
				//
				//	//for(auto i = 0U; i<geometry.size(); i++){
				//    if(e.second.count(geometry[i]))
				//        copys[i] = e.second[geometry[i]];
				//    else
				//        FGException("Unable to copy polygon");
				//}
				//TransformPtr t = (context == getConstContext()) ? getParentTransform() : context.getRootTransform();
				//	return CSGPolygon(context, getSetting()->copy(), e.first, std::move(copys)).getHandle();
				//}
				//throw FGException("Unable to copy polygon");
				std::map<GHANDLE, GHANDLE> new_points;
				for (auto i : geometry) {
					auto& l = getConstContext().get<ILine>(i);
					for (auto p : l.getChildren()) {
						if (new_points.count(p) == 0) {
							auto& point = getConstContext().get<Vertex>(p);
							new_points[p] = Vertex(context, point.getSetting(), point.position()).getHandle();
						}
					}
				}
				std::vector<GHANDLE> new_lines;
				for (auto i : geometry) {
					auto& l = getConstContext().get<ILine>(i);
					std::vector<GHANDLE> remap;
					for (auto j : l.getChildren())
						remap.push_back(new_points[j]);
					new_lines.push_back(l.createSame(context, l.getSetting(), remap));
				}
				return Shape(context, getSetting(), context, new_lines).getHandle();
			}
			virtual double getDistance(const ray &v) const {
				// [TODO]
				return 0.0f;
			}
			virtual bool isInPlane(const plane &p) const {
				for (auto i : geometry) {
					if (!getConstContext().get_ptr(i)->isInPlane(p)) return false;
				}
				return true;
			}
			virtual bool equals(const IGeometry &geom) const {
				const Primitive* g = dynamic_cast<const Primitive*>(&geom);

				bool result = g && g->getConstContext() == getConstContext();
				if (result) {
					std::multimap<GHANDLE, std::pair<ILine*, bool>> allLines;
					for (auto i : g->getChildren()) {
						ILine* l = dynamic_cast<ILine*>(getConstContext().get_ptr(i));
						if (!l) return false;
						allLines.insert(std::make_pair(l->p0Handle(), std::make_pair(l, false)));
						allLines.insert(std::make_pair(l->p1Handle(), std::make_pair(l, true)));
					}
					for (auto i : geometry) {
						ILine* l = dynamic_cast<ILine*>(getConstContext().get_ptr(i));
						bool hasEq = false;
						for (auto i = allLines.lower_bound(l->p1Handle()); i != allLines.upper_bound(l->p1Handle()); i++) {
							if (i->second.first->equals(*l)) {
								hasEq = true;
								break;
							}
						}
						if (!hasEq) return false;
					}
					return true;
				}
				return false;
			}

			// IPrimitive interface
		public:
			virtual SETTINGHANDLE inside(const vector3 point)const {
				if (classify(point) == -1) return getSetting();

				return NoSetting;
			}
		};
	}
}
