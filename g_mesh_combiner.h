#pragma once
#include "g_meshview.h"
#include "g_criteria.h"

namespace fg {
	class MeshCombiner : public virtual IMeshView {
		MeshView2d base;
		std::unique_ptr<ICriterion> criterion;
		std::vector<std::shared_ptr<IMeshView>> meshes;
	public:
		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const { return base.boundary(index); }
		virtual const size_t boundary_size() const { return base.boundary_size(); }
		virtual const Mesh2& mesh() const { return base.mesh(); }
		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const { return base.isBoundary(i); }
		MeshCombiner(const IMeshView& base_mesh) : base{base_mesh}{}
		template<class Criterion>
		void SetCriterion() {
			criterion = (std::make_unique<Criterion>(Criterion{ base.mesh() }));
		}
		void AddMesh(std::shared_ptr<IMeshView> _mesh) {
			meshes.push_back(_mesh);

			for (size_t i{}; i < meshes.back()->boundary_size(); i++) {
				base.AddLine(meshes.back()->boundary(i));
			}
		}
		void AddPoint(const vector3& point) {
			base.AddPoint(point);
		}
		void AddLine(const MeshedLine& line) {
			base.AddLine(line);
		}

		template<class T>
		void AdjustMesh(const IElementSize<T>& size) {
			// у каждого ребра есть приоритет. Приоритет тем больше,чем больше ошибка на этом ребре
			// чтобы ребра не обрабатывались бесконечно, новодобавленные ребра записываются в другой мультимэп
			// при добавлениив мультмэп, ребра сортируются по приоритету, поэтому наиболее приоритетные обрабатываются первее
			std::multimap<double, size_t> edges_list;//(base.mesh().edgesCount());
			std::multimap<double, size_t> edges_list_new;//(base.mesh().edgesCount());
			for (size_t i{}; i < base.mesh().edgesCount(); i++) edges_list.insert(std::make_pair(1e300, i));
			//size_t index = 0;
			size_t control = 0;

			// TODO: может быть исходным ребрам тоже нужно изначально присвоить приоритет ???
			for(;;){
			//for (; control < 5; control++) {
				while (edges_list.size()) {
					auto edge = edges_list.end();
					edge--;
					switch (criterion->get(edge->second, size))
					{
					case CriterionResult::Short:
						// [TODO] collapse
					default:
					case CriterionResult::Fit:
						break;
					case CriterionResult::Long:
						static std::vector<size_t> edges;
						edges.resize(0);
						base.SubdivideEdge(edge->second, edges);
						for (auto i : edges) {
							edges_list_new.insert(std::make_pair(criterion->get_error(i, size), i));
						}
						break;
					}
					edges_list.erase(edge);
				}
				if (edges_list_new.size() == 0) break;
				edges_list = std::move(edges_list_new);
			}
		}
	};
}