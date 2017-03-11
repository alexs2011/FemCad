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
		void AdjustMesh(const IElementSize<T>& size) {
			// у каждого ребра есть приоритет. Приоритет тем больше,чем больше ошибка на этом ребре
			// чтобы ребра не обрабатывались бесконечно, новодобавленные ребра записываются в другой мультимэп
			// при добавлениив мультмэп, ребра сортируются по приоритету, поэтому наиболее приоритетные обрабатываются первее
			//std::multimap<double, size_t> edges_list;//(base.mesh().edgesCount());
			//std::multimap<double, size_t> edges_list_new;//(base.mesh().edgesCount());

			std::vector<size_t> edges_list;
			std::vector<size_t> edges_list_new;
			//for (size_t i{}; i < base.mesh().edgesCount(); i++)
			//	edges_list.insert(std::make_pair(criterion->get_error(i, size), i));
			for (size_t i{}; i < base.mesh().edgesCount(); i++)
				if(criterion->get(i, size) != CriterionResult::Fit)
					edges_list.push_back(i);
			//size_t index = 0;
			size_t control = 0;


			std::chrono::high_resolution_clock::time_point first_start;
			//std::chrono::high_resolution_clock::time_point first_end;
			//std::chrono::high_resolution_clock::time_point last_start;
			std::chrono::high_resolution_clock::time_point last_end;
			std::ofstream file;
			file.open("time.txt");
			auto time = std::chrono::high_resolution_clock::now();

			for (;;)
			{
				//for (; control < 5; control++) {
	//#ifdef _DEBUG
					//std::cout << edges_list.size() << std::endl;
					//std::cout << base.mesh().edgesCount() << std::endl;
	//#endif
					//if (control == 0) {
					//	first_start = std::chrono::high_resolution_clock::now();
					//}

				//while (edges_list.size())
				for(size_t i = 0U; i<edges_list.size(); i++)
				{
					//if (control % 250 == 0) {
					//	last_end = std::chrono::high_resolution_clock::now();
					//	time = std::chrono::duration_cast<std::chrono::duration<double>>(last_end - first_start).count();
					//	file << control << " " << time << std::endl;
					//}
					//auto edge = edges_list.end();
					//edge--;
					switch (criterion->get(edges_list[i], size))
					{
					case CriterionResult::Short:
						auto r = base.CollapseEdge(edges_list[i]);
						for (auto j : r) {
							if (j == Mesh2::NotAnEdge || j > i) continue;
							edges_list_new.push_back(j);
						}
						// [TODO] collapse
					default:
					case CriterionResult::Fit:
						break;
					case CriterionResult::Long:
						//control++;
						static std::vector<size_t> edges;
						edges.resize(0);

						//last_start = std::chrono::high_resolution_clock::now();
						base.SubdivideEdge(edges_list[i], edges);
						/*if (control == 0) {
							first_end = std::chrono::high_resolution_clock::now();
							control++;
						}*/
						for (auto i : edges) {
							edges_list_new.push_back(i);
						}
						break;
					}
					//edges_list.erase(edge);
				}
				//if (edges_list_new.size() == 0) break;
				std::set<size_t> rep(edges_list_new.begin(), edges_list_new.end());
				edges_list_new = std::vector<size_t>(rep.begin(), rep.end());
				edges_list = std::move(edges_list_new);
				std::cout << "\nProgress: " << edges_list.size() << "        ";
			}
			std::cout << std::endl << base.mesh().get_tree_debug_info();
			std::cout << std::endl << meshes[0]->mesh().get_tree_debug_info();
			std::cout << std::endl << "Time: " <<
				std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time).count();

			

			//std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(first_end - first_start).count() << std::endl;
			//std::cout << std::chrono::duration_cast<std::chrono::duration<double>>(last_end - last_start).count() << std::endl;
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