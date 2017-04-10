#pragma once
#include "g_meshview.h"
#include "g_criteria.h"

namespace fg {
	class MeshCombiner : public virtual IMeshView {
	private:
		MeshView2d base;
		std::unique_ptr<ICriterion> criterion;
		std::vector<std::shared_ptr<IMeshView>> meshes;
		// переменные для AdjustIteration
		std::vector<size_t> edges_list;
		std::vector<size_t> edges_list_new;
		size_t adjustMeshIter;
		size_t i_flip;
	public:
		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const { return base.boundary(index); }
		virtual const size_t boundary_size() const { return base.boundary_size(); }
		virtual const Mesh2& mesh() const { return base.mesh(); }
		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const { return base.isBoundary(i); }
		MeshCombiner(const IMeshView& base_mesh) : base{ base_mesh } {}
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
		void AdjustMeshInitialization(const IElementSize<T>& size)
		{
			adjustMeshIter = 0;
			i_flip = 0;
			for (size_t i{}; i < base.mesh().edgesCount(); i++)
				if (criterion->get(i, size) != CriterionResult::Fit)
					edges_list.push_back(i);
		}

		template<class T>
		void AdjustMesh(const IElementSize<T>& size) {
			// у каждого ребра есть приоритет. Приоритет тем больше,чем больше ошибка на этом ребре
			// чтобы ребра не обрабатывались бесконечно, новодобавленные ребра записываются в другой мультимэп
			// при добавлениив мультмэп, ребра сортируются по приоритету, поэтому наиболее приоритетные обрабатываются первее

			std::chrono::high_resolution_clock::time_point first_start;
			std::chrono::high_resolution_clock::time_point last_end;
			auto time = std::chrono::high_resolution_clock::now();

			AdjustMeshInitialization(size);
			try {
				bool flag = true;
				while (flag)
				{
					flag = AdjustIteration(size);
					if (adjustMeshIter == edges_list.size() && flag != false)
						std::cout << "\nProgress: " << edges_list.size() << "        ";
				}
			}
			catch (const char* s) {
				std::cout << s << std::endl;
			}
			catch (...) {
				std::cout << "Exceptions!!!";
			}
			std::cout << std::endl << "Time: " <<
				std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time).count();
		}

		template<class T>
		inline bool AdjustIteration(const IElementSize<T>& size)
		{
			if (adjustMeshIter == edges_list.size())
			{
				//for (; i_flip < base.mesh().edgesCount(); i_flip++)
				//{
				//	if (i_flip + 1 < base.mesh().edgesCount() && base.mesh().flippable(i_flip + 1))
				//	{
				//		this->dbg_edge_to_be_processed = i_flip + 1;
				//		if(i_flip - 1 >= 0 && base.mesh().flippable(i_flip - 1))
				//			base.Flip(i_flip - 1);
				//		i_flip++;
				//		return true;
				//	}
				//	else
				//		this->dbg_edge_to_be_processed = -1;
				//	base.Flip(i_flip);
				//}

				if (edges_list_new.empty())
				{
					this->dbg_edge_to_be_processed = -1;
					return false;
				}
				std::set<size_t> rep(edges_list_new.begin(), edges_list_new.end());
				edges_list_new = std::vector<size_t>(rep.begin(), rep.end());
				edges_list = std::move(edges_list_new);
				adjustMeshIter = 0;
				i_flip = 0;
				
			}
			if (adjustMeshIter + 1 == edges_list.size())
			{
				if (!edges_list_new.empty())
					this->dbg_edge_to_be_processed = edges_list_new[0];
				else
					this->dbg_edge_to_be_processed = -1;
			}
			else
			{
				this->dbg_edge_to_be_processed = edges_list[adjustMeshIter + 1]; // отладка
			}

			if (this->dbg_edge_to_be_processed >= base.mesh().edgesCount())
				this->dbg_edge_to_be_processed = -1;
			else
			{
				auto dbg_edge = base.mesh().edge(this->dbg_edge_to_be_processed);
				std::cout << this->dbg_edge_to_be_processed << " " << dbg_edge.first << " " << dbg_edge.second << " " << base.mesh().point(dbg_edge.first) << " " << base.mesh().point(dbg_edge.second) << std::endl;
			}

			if (edges_list[adjustMeshIter] >= base.mesh().edgesCount())
			{
				adjustMeshIter++;
				return true;
			}

			switch (criterion->get(edges_list[adjustMeshIter], size))
			{
			case CriterionResult::Short:
				//break;
				auto r = base.CollapseEdge(edges_list[adjustMeshIter]);
				// [TODO] edges_new add
				for (auto j : r) {
					if (j == Mesh2::NotAnEdge || j > adjustMeshIter)
					{
						adjustMeshIter++;
						return true;
					}
					edges_list_new.push_back(j);
				}
			default:
			case CriterionResult::Fit:
				break;
			case CriterionResult::Long:
				break;
				static std::vector<size_t> edges;
				edges.resize(0);
				base.SubdivideEdge(edges_list[adjustMeshIter], edges);
				for (auto i : edges)
					edges_list_new.push_back(i);
				break;
			}
			//if (this->dbg_edge_to_be_processed != edges_list[adjustMeshIter + 1])
			//	std::cout << std::endl << "err";
 			adjustMeshIter++;
			return true;
		}

		void AddIntersectingMesh(std::shared_ptr<IMeshView> _mesh) {
			meshes.push_back(_mesh);

			for (size_t i{}; i < meshes.back()->boundary_size(); i++) {
				base.AddLine(meshes.back()->boundary(i));
			}
			for (size_t i{}; i < meshes.back()->mesh().edgesCount(); i++) {
				auto e = meshes.back()->mesh().edge(i);
				base.AddPoint(meshes.back()->mesh().point(e.first));
				base.AddPoint(meshes.back()->mesh().point(e.second));
			}

		}
	};
};