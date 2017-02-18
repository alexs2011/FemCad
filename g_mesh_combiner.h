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
		template<class T>
		void AdjustMesh(const IElementSize<T>& size) {
			std::vector<size_t> edges_list(base.mesh().edgesCount());
			for (size_t i{}; i < edges_list.size(); i++) edges_list[i] = i;
			size_t index = 0;
			for (; index < edges_list.size(); index++) {
				switch (criterion->get(edges_list[index], size))
				{
				case CriterionResult::Short:
					// [TODO] collapse
				default:
				case CriterionResult::Fit:
					continue;
				case CriterionResult::Long:
					base.SubdivideEdge(edges_list[index], edges_list);
					break;
				}
			}
		}
	};
}