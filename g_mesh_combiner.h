#pragma once
#include "g_meshview.h"
#include "g_criteria.h"

namespace fg {
	class MeshCombiner : public virtual IMeshGeometryView {
	private:
		MeshView2d base;
		std::unique_ptr<ICriterion> criterion;
		std::vector<std::shared_ptr<IGeometryView>> meshes;
		// ���������� ��� AdjustIteration
		std::vector<size_t> edges_list;
		std::vector<size_t> edges_list_new;
		size_t adjustMeshIter;
		//size_t i_flip;
	public:
		const MeshView2d& getBase() const { return base; }
		virtual MeshedLine boundary(Mesh2::EdgeIndex index) const { return base.boundary(index); }
		virtual const size_t boundary_size() const { return base.boundary_size(); }
		virtual const Mesh2& mesh() const { return base.mesh(); }
		virtual inline bool isBoundary(Mesh2::EdgeIndex i) const { return base.isBoundary(i); }
		MeshCombiner(const IMeshGeometryView& base_mesh) : base{ base_mesh }, adjustMeshIter{} {}
		template<class Criterion>
		void SetCriterion() {
			criterion = (std::make_unique<Criterion>(Criterion{ base.mesh() }));
		}
		template<class Criterion, class P>
		void SetCriterion(P param) {
			criterion = (std::make_unique<Criterion>(Criterion{ base.mesh(), param }));
		}
		void AddMesh(std::shared_ptr<IGeometryView> _mesh) {
			meshes.push_back(_mesh);

		}
		void AddGeometry() {
			for (size_t i{}; i < meshes.back()->boundary_size(); i++) {
				base.AddLine(meshes.back()->boundary(i));
			}
		}

		void ForciblyFlip() {
			std::vector<GHANDLE> edges;
			edges.resize(0);
			std::cout << "\nGoing flip\n";
			for (size_t i{}; i < base.mesh().edgesCount(); i++)
				edges.push_back(i);
			std::cout << "Edges list = " << edges.size() << std::endl;
			while (!edges.empty()) {
				auto b = edges.back();
				edges.pop_back();
				base.Flip(b, edges);
			}
		}
		/*void ForciblyCollapse() {
			std::vector<GHANDLE> edges;
			edges.resize(0);
			for (size_t i{}; i < base.mesh().edgesCount(); i++)
				edges.push_back(i);
			while (!edges.empty()) {
				auto b = edges.back();
				edges.pop_back();
				base.CollapseEdge(b);
			}
		}*/

		void AddPoint(const vector3& point) {
			base.AddPoint(point);
		}
		void AddLine(const MeshedLine& line) {
			base.AddLine(line);
		}

		void AdjustMeshInitialization()
		{
			adjustMeshIter = 0;
			//i_flip = 0;
			for (size_t i{}; i < base.mesh().edgesCount(); i++)
				//if (criterion->get(i, size) != CriterionResult::Fit)
				edges_list.push_back(i);
		}

		template<class T>
		void AdjustMesh(const IElementSize<T>& size) {
			// � ������� ����� ���� ���������. ��������� ��� ������,��� ������ ������ �� ���� �����
			// ����� ����� �� �������������� ����������, ��������������� ����� ������������ � ������ ���������
			// ��� ����������� ��������, ����� ����������� �� ����������, ������� �������� ������������ �������������� ������

			std::chrono::high_resolution_clock::time_point first_start;
			std::chrono::high_resolution_clock::time_point last_end;
			auto time = std::chrono::high_resolution_clock::now();

			AdjustMeshInitialization();
			try {
				size_t flag = 1;
				size_t repeats = 100, pre_state = 0;
				while (flag && repeats > 0)
				{
					if (pre_state != flag) {
						repeats--;
						pre_state = flag;
					}
					AdjustIteration(size, flag);
					if (adjustMeshIter == edges_list.size() && flag != 0)
						std::cout << "\nProgress: " << edges_list.size() << "        ";
				}
			}
			catch (const char* s) {
				std::cout << "Exceptions!!!" << s << std::endl;
			}
			catch (...) {
				std::cout << "Exceptions!!!";
			}
			std::cout << std::endl << "Time: " <<
				std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time).count();
		}
		bool first = true;
		template<class T>
		inline size_t AdjustIteration(const IElementSize<T>& size, size_t& mode)
		{
			/*if (mode == 0) {
				AdjustMeshInitialization();
			}*/
			if (edges_list.empty() && edges_list_new.empty())
			{
				this->dbg_edge_to_be_processed = -1;
				return (mode = 0);
			}

			if (mode == 1) { // collapse
				auto b = edges_list.back();
				edges_list.pop_back();
				bool condition = false;
				if (b < base.mesh().edgesCount()) {
					if (base.isBoundary(b)) {
						for (auto i : base._edgeGeometry[b]) {
							condition = criterion->get(b, BoundaryElementSize<T>(base.geometry[i].line)) == CriterionResult::Short;
							if (condition) break;
						}
					}
					else {
						condition = criterion->get(b, size) == CriterionResult::Short;
					}
					if (condition) {
						auto r = base.CollapseEdge(b);
						for (auto j : r) {
							//std::cout << j;
							edges_list.push_back(j);
							if (j < base.mesh().edgesCount()) 
								base.Flip(j, edges_list);
							//edges_list_new.push_back(j);
						}
					}
				}
				if (edges_list.empty()) {
					if (first) {
						for (size_t i{}; i < base.mesh().edgesCount(); i++)
							edges_list.push_back(i);
						//first = false;
					}
					else {
						edges_list = std::move(edges_list_new);
					}
					edges_list_new.clear();
					std::cout << "Going subdivide\n";
					this->dbg_edge_to_be_processed = edges_list.back();
					return (mode = 2); // go to subdivide;
				}
				this->dbg_edge_to_be_processed = edges_list.back();
				return (mode = 1);
			}
			if (mode == 2) { // subdivide
				if (edges_list.empty()) {
					std::cout << "New edges list = " << edges_list_new.size() << std::endl;
					std::set<size_t> rep(edges_list_new.begin(), edges_list_new.end());
					edges_list_new = std::vector<size_t>(rep.begin(), rep.end());
					edges_list = std::move(edges_list_new);
					edges_list_new.clear();
					this->dbg_edge_to_be_processed = -1;
					std::cout << "Going flip\n";
					return mode = 3;
				}
				//if (edges_list.empty()) return mode = 1;
				auto b = edges_list.back();
				edges_list.pop_back();
				bool condition = false;
				if (base.isBoundary(b)) {
					for (auto i : base._edgeGeometry[b]) {
						condition = criterion->get(b, BoundaryElementSize<T>(base.geometry[i].line)) == CriterionResult::Long;
						if (condition) break;
					}
					condition = !condition ? criterion->get(b, size) == CriterionResult::Long : condition;
				}
				else {
					condition = criterion->get(b, size) == CriterionResult::Long;
				}
				if (condition) {
					auto end = edges_list_new.size();
					base.SubdivideEdge(b, edges_list_new);
					for (auto j = end; j < edges_list_new.size(); ++j) {
						base.Flip(edges_list_new[j], edges_list);
					}
				}
				this->dbg_edge_to_be_processed = (edges_list.empty()) ? -1 : edges_list.back();
				return mode = 2;
			}
			if (mode == 3) { // flip
				auto b = edges_list.back();
				edges_list.pop_back();
				auto end = edges_list.size();
				base.Flip(b, edges_list);
				edges_list_new.push_back(b);
				if (end < edges_list.size())
					edges_list_new.insert(edges_list_new.end(), edges_list.begin() + end, edges_list.end());
				if (edges_list.empty()) {
					//for (size_t i{}; i < base.mesh().edgesCount(); i++)
					edges_list = std::move(edges_list_new);
					this->dbg_edge_to_be_processed = -1;
					std::cout << "Going collapse\n";
					return mode = 1; // go to collapse;
				}
				this->dbg_edge_to_be_processed = (edges_list.empty()) ? -1 : edges_list.back();
				return mode = 3;

			}
			return mode;
			//if (adjustMeshIter == edges_list.size())
			//{
			//	for (; i_flip < base.mesh().edgesCount(); i_flip++)
			//	{
			//		if (i_flip + 1 < base.mesh().edgesCount() && base.mesh().flippable(i_flip + 1))
			//		{
			//			this->dbg_edge_to_be_processed = i_flip + 1;
			//			if (i_flip - 1 >= 0 && base.mesh().flippable(i_flip - 1))
			//				base.Flip(i_flip - 1);
			//			i_flip++;
			//			return true;
			//		}
			//		else
			//			this->dbg_edge_to_be_processed = -1;
			//		base.Flip(i_flip);
			//	}

			//	if (edges_list_new.empty())
			//	{
			//		this->dbg_edge_to_be_processed = -1;
			//		return false;
			//	}
			//	std::set<size_t> rep(edges_list_new.begin(), edges_list_new.end());
			//	edges_list_new = std::vector<size_t>(rep.begin(), rep.end());
			//	edges_list = std::move(edges_list_new);
			//	adjustMeshIter = 0;
			//	i_flip = 0;

			//}
			//if (adjustMeshIter + 1 == edges_list.size())
			//{
			//	if (!edges_list_new.empty())
			//		this->dbg_edge_to_be_processed = edges_list_new[0];
			//	else
			//		this->dbg_edge_to_be_processed = -1;
			//}
			//else
			//{
			//	this->dbg_edge_to_be_processed = edges_list[adjustMeshIter + 1]; // �������
			//}

			//if (this->dbg_edge_to_be_processed >= base.mesh().edgesCount())
			//	this->dbg_edge_to_be_processed = -1;
			//else
			//{
			//	auto dbg_edge = base.mesh().edge(this->dbg_edge_to_be_processed);
			//	std::cout << this->dbg_edge_to_be_processed << " " << dbg_edge.first << " " << dbg_edge.second << " " << base.mesh().point(dbg_edge.first) << " " << base.mesh().point(dbg_edge.second) << std::endl;
			//}

			//if (edges_list[adjustMeshIter] >= base.mesh().edgesCount())
			//{
			//	adjustMeshIter++;
			//	return true;
			//}
			//static std::vector<size_t> r;
			//r.clear();
			//switch (criterion->get(edges_list[adjustMeshIter], size))
			//{
			//case CriterionResult::Short:
			//	//break;
			//	r = base.CollapseEdge(edges_list[adjustMeshIter]);
			//	// [TODO] edges_new add
			//	for (auto j : r) {
			//		if (j == Mesh2::NotAnEdge || j > adjustMeshIter)
			//		{
			//			adjustMeshIter++;
			//			return true;
			//		}
			//		edges_list_new.push_back(j);
			//	}
			//	break;
			//default:
			//case CriterionResult::Fit:
			//	break;
			//case CriterionResult::Long:
			//	//break;
			//	static std::vector<size_t> edges;
			//	edges.resize(0);
			//	base.SubdivideEdge(edges_list[adjustMeshIter], edges);
			//	for (auto i : edges)
			//		edges_list_new.push_back(i);
			//	break;
			//}
			////if (this->dbg_edge_to_be_processed != edges_list[adjustMeshIter + 1])
			////	std::cout << std::endl << "err";
			//adjustMeshIter++;
			//return true;
		}

		void export_msh(const std::string& file_name, const IGeometryView& materials) const {
			std::ofstream file(file_name);
			auto& _mesh = base.mesh();
			file << "$MeshFormat\n2.2 0 8\n$EndMeshFormat\n";
			file << "$PhysicalNames\n";
			auto s = materials.listSettings();
			std::map<size_t, size_t> remap;
			std::map<std::string, std::pair<size_t, size_t>> material;
			size_t p{1};
			for (auto i : s) {
				//remap[i.second->getID()] = p;
				try {
					auto cur = std::string{ *i.second->getParameterByName("Name") };
					if (material.count(cur)) {
						if (cur == "") {
							remap[i.second->getID()] = 0xFFFFFFFF;
						}
						else
							remap[i.second->getID()] = material[cur].second;
					}
					else {
						remap[i.second->getID()] = p;
						material[cur] = std::make_pair(i.second->dim, p);
						p++;
					}
				}
				catch (const FGException&) {
					remap[i.second->getID()] = 0xFFFFFFFF;
				}
			}
			file << material.size() << "\n";
			for (auto i : material) {
				file << i.second.first << ' ' << i.second.second << " \"" << i.first << "\"" << std::endl;
			}
			file << "$EndPhysicalNames\n";

			std::map<size_t, size_t> nodes;
			std::map<size_t, std::pair<size_t, std::pair<size_t, size_t>>> edges;
			std::map<size_t, std::pair<size_t, std::tuple<size_t, size_t, size_t>>> triangles;
			p = 1U;
			for (size_t i{}; i < _mesh.edgesCount(); ++i) {
				auto s = base.materialId(i);
				if (s == 0xFFFFFFFF || remap.count(s) == 0) continue;
				else {
					edges[p] = std::make_pair(remap[s], _mesh.edge(i));
					p++;
				}
			}
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				auto s = materials.materialId(_mesh.sample_triangle(i, fg::vector3{ .33333,.33333,.33333 }));
				if (s == 0xFFFFFFFF || remap.count(s) == 0) continue;
				else {
					auto a = _mesh.triangleVertices(i);

					triangles[p] = std::make_pair(remap[s], a);
					nodes[std::get<0>(a)] = std::get<0>(a);
					nodes[std::get<1>(a)] = std::get<1>(a);
					nodes[std::get<2>(a)] = std::get<2>(a);
					p++;
				}
			}

			std::map<size_t, size_t> node_remap;
			size_t node{1};

			file << "$Nodes\n";
			file << nodes.size() << std::endl;
			for (auto i : nodes) {
				auto& p = _mesh.point(i.second);
				node_remap[i.first] = node;
				file << node << ' ' << p.x << ' ' << p.y << ' ' << p.z << std::endl;
				node++;
			}
			file << "$EndNodes\n";
			file << "$Elements\n";
			file << edges.size() + triangles.size() << std::endl;

			for (auto i : edges) {
				file << i.first << ' ' << 1 << ' ' << 1 << ' ' 
					<< i.second.first << ' ' 
					<< node_remap[i.second.second.first] << ' ' << node_remap[i.second.second.second] << std::endl;
			}
			for (auto i : triangles) {
				file << i.first << ' ' << 2 << ' ' << 1 << ' '
					<< i.second.first 
					<< ' ' << node_remap[std::get<0>(i.second.second)] 
					<< ' ' << node_remap[std::get<1>(i.second.second)] 
					<< ' ' << node_remap[std::get<2>(i.second.second)] 
					<< std::endl;
			}

			file << "$EndElements\n";
			file.close();
			std::cout << "End export\n";
		}

		//void AddIntersectingMesh(std::shared_ptr<IMeshView> _mesh) {
		//	meshes.push_back(_mesh);

		//	for (size_t i{}; i < meshes.back()->boundary_size(); i++) {
		//		base.AddLine(meshes.back()->boundary(i));
		//	}
		//	for (size_t i{}; i < meshes.back()->mesh().edgesCount(); i++) {
		//		auto e = meshes.back()->mesh().edge(i);
		//		base.AddPoint(meshes.back()->mesh().point(e.first));
		//		base.AddPoint(meshes.back()->mesh().point(e.second));
		//	}

		//}
	};
};