#pragma once
#include "g_splitter.h"
namespace fg {
	template<class Element>
	class FEMCADGEOMSHARED_EXPORT BSPTree : public IClassifiable {
		std::unique_ptr<Splitter<Element>> splitter;
		std::unique_ptr<BSPTree<Element>> front, back;
		BSPTree<Element>* parent;
		std::vector<Element*> mid;
	public:
		BSPTree(const std::vector<Element*>& geometry, BSPTree<Element>* parent = nullptr) : parent{ parent } {
			pickSplitter(geometry);
			std::vector<Element*> f, b;

			for (auto i : geometry) {
				//if(i->getHandle() == splitter->getHandle()) conti
				auto r = splitter->classify(*i);
				if (r == ClassificationState::Incident) { mid.emplace_back(i); continue; }
				if (r != ClassificationState::Back) { f.emplace_back(i); }
				if (r != ClassificationState::Front) { b.emplace_back(i); }
			}
			if (f.size()) front = std::make_unique<BSPTree<Element>>(BSPTree(f, this));
			if (b.size()) back = std::make_unique<BSPTree<Element>>(BSPTree(b, this));
		}
		BSPTree(BSPTree<Element>&&) = default;
		virtual int classify(const vector3 & p) const override
		{
			int res = splitter->classify(p);

			int f = 1;
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
			return 1;
		}
		virtual vector3 middle() const override
		{
			return vector3{};
		}
	protected:
		void pickSplitter(const std::vector<Element*>& geometry) {
			for (auto i : geometry) {
				Splitter<Element> s{ *i };
				if (!s.special()) { splitter = std::make_unique<Splitter<Element>>(std::move(s)); return; }
			}
			splitter = std::make_unique<Splitter<Element>>(Splitter<Element>{ *geometry.back() });
		}
	};
}
