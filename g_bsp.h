#pragma once
#include "g_splitter.h"
namespace fg {
	//
	// Рекурсивная структрура данных, задающая дерево BSP-разбиения
	//
	template<class Element>
	class FEMCADGEOMSHARED_EXPORT BSPTree : public IClassifiable {
		// некоторое выражение, которое подразбивает пространство на две части
		std::unique_ptr<Splitter<Element>> splitter;
		// bsp-поддеревья, которые подразбивают пространство спереди и сзади сплиттера
		std::unique_ptr<BSPTree<Element>> front, back;
		// указатель на родительский узел
		BSPTree<Element>* parent;
		// линии, которые совпадают со сплиттером (лежат не сзади и не спереди)
		std::vector<Splitter<Element>> mid;
	public:
		BSPTree(const std::vector<Splitter<Element>>& geometry, BSPTree<Element>* parent = nullptr) : parent{ parent } {
			// пока что мы по сути берём первую попавшуюся линию из списка линий и используем её как сплиттер
			pickSplitter(geometry);
			// f- front, b - back
			std::vector<Splitter<Element>> f, b;
			// перебираем все линии и классифицируем их: линия лежит на сплиттере (Incident), сзади (Back), спереди (Front)
			for (auto i : geometry) {
				auto r = splitter->classify(i);
				if (r == ClassificationState::Incident) { mid.emplace_back(i); continue; }
				else if (r == ClassificationState::Front) { f.emplace_back(i); }
				else if (r == ClassificationState::Back) { b.emplace_back(i); }
				else if (r == ClassificationState::Cross) {
					splitter->intersect(i, f, b);
				}
			}
			// и кладем их в соответствующие поддеревья
			if (f.size()) front = std::make_unique<BSPTree<Element>>(BSPTree(f, this));
			if (b.size()) back = std::make_unique<BSPTree<Element>>(BSPTree(b, this));
		}

		BSPTree(BSPTree<Element>&&) = default;
		virtual int classify(const vector3 & p) const override
		{
			int res = splitter->classify(p);
			if (res == 0) for (auto i : mid) if (i.classify_point(p) == 0) return 0;

			if (res > 0) {
				if (front != nullptr) {
					return front->classify(p);
				}
				return 1;
			}
			else {
				if (back != nullptr) {
					return back->classify(p);
				}
				return -1;
			}
			/*int f = 1;
			int b = 1;
			if (res == 0) for (auto i : mid) if (i->pointCast(p)) return 0;
			if (front == back) return res;
			if (res < 0 && back == nullptr) return -1;
			if (res > 0 && front == nullptr) return 1;
			if (back != nullptr && res <= 0) {
				b = back->classify(p); if (b <= 0) return b;
			}
			if (front != nullptr && res >= 0) {
				f = front->classify(p); if (f <= 0) return f;
			}
			return 1;*/
		}
		virtual vector3 middle() const override
		{
			throw FGException("Invalid operation");
			return vector3{};
		}
	protected:
		void pickSplitter(const std::vector<Splitter<Element>>& geometry) {
			// пока что мы по сути берём первую попавшуюся линию и используем её как сплиттер
			for (auto i : geometry) {
				// сплиттер копирует линию в себя
				//Splitter<Element> s{ *i };
				// special - особенный сплиттер, должен рассматриваться после обычных (эллиптическая кривая)
				if (!i.special()) { splitter = std::make_unique<Splitter<Element>>(Splitter<Element>{ i })/*std::move(s))*/; return; }
			}
			splitter = std::make_unique<Splitter<Element>>(Splitter<Element>{ geometry.back() });
		}
	};
}
