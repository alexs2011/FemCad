#include "stdafx.h"
#include "femcadgeom.h"
#include <math.h>
#include "g_setting.h"
#include "g_scene.h"
#include "g_vertex.h"
#include "g_line.h"
#include "g_primitive.h"
#include "g_intersector.h"
#include "g_bsp.h"
#include "g_shape_2d.h"
#include "g_utility.h"
#include "g_meshing.h"
#include "g_line_ext.h"
#include "g_meshview.h"
#include "MeshDrawer.h"
#include "g_mesh_combiner.h"

//int fg::BSPNode2D::ID = 0;
//int fg::ISetting::ID = 0;

FemCadGeomTester::FemCadGeomTester()
{
	//return 0;
}
using namespace fg;
void FemCadGeomTester::Launch()
{
	Scene s;
	Scene s2;
	SETTINGHANDLE vs = std::make_shared<VertexSetting>(VertexSetting());
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	ls_1->setParameter("N", DoubleParameter(1));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting());
	ls_2->setParameter("N", DoubleParameter(5));
	ls_2->setParameter("q", DoubleParameter(1.5));

	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());

	GHANDLE v0 = Vertex(s, vs, { -3,-3,0 }).getHandle();
	GHANDLE v1 = Vertex(s, vs, { 3,-3,0 }).getHandle();
	GHANDLE v2 = Vertex(s, vs, { -3,3,0 }).getHandle();
	GHANDLE v3 = Vertex(s, vs, { 3,3,0 }).getHandle();
	GHANDLE v4 = Vertex(s2, vs, { 0,-1,0 }).getHandle();
	GHANDLE v5 = Vertex(s2, vs, { 2,-1,0 }).getHandle();
	GHANDLE v6 = Vertex(s2, vs, { 0,1,0 }).getHandle();
	GHANDLE v7 = Vertex(s2, vs, { 2,1,0 }).getHandle();
	//GHANDLE v6 = Vertex(s, vs, { 0,2,0 }).getHandle();
	//GHANDLE v7 = Vertex(s, vs, { -0.5,1,0 }).getHandle();
	/*GHANDLE v4 = Vertex(s, vs, { 1.5,2,0 }).getHandle();
	GHANDLE v5 = Vertex(s, vs, { 1.5,0,0 }).getHandle();
	GHANDLE v6 = Vertex(s, vs, { 1.5,1,0 }).getHandle();
	GHANDLE v7 = Vertex(s, vs, { 5,1,0 }).getHandle();*/

	//GHANDLE l0 = EllipticSegment(s, ls, v0, v1, v4).getHandle();
	GHANDLE l0 = LineSegment(s, ls_1, v0, v1).getHandle();
	GHANDLE l1 = LineSegment(s, ls_1, v1, v3).getHandle();
	GHANDLE l2 = LineSegment(s, ls_1, v3, v2).getHandle();
	GHANDLE l3 = LineSegment(s, ls_1, v2, v0).getHandle();

	GHANDLE l4 = LineSegment(s2, ls_2, v4, v5).getHandle();
	GHANDLE l5 = LineSegment(s2, ls_2, v5, v7).getHandle();
	GHANDLE l6 = LineSegment(s2, ls_2, v7, v6).getHandle();
	GHANDLE l7 = LineSegment(s2, ls_2, v6, v4).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l0, l1, l2, l3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l4, l5, l6, l7 }).getHandle();
	
	
	// всё то делает rect - это задаёт противоположные границы геометрии, что нужно для построения сетки
	// также задает отступы разрядки на всей границе, а также на отдельных её линиях
	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, v0, v1, v2, v3 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v4, v5, v6, v7 };

	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };

	MeshCombiner combiner{ RectMeshView(rect_base) };
	combiner.SetCriterion<OnePointCriterion>();
	
	combiner.AddMesh(mesh_form0);
	MeshElementSizeIsoMaxEdgeLength size(mesh_form0->mesh());
	combiner.AdjustMesh(size);

	int mi = globalMeshDrawer.draw(combiner);
	//int mi2 = globalMeshDrawer.draw(*mesh_form0);

	globalMeshDrawer.init();
	//Scene s, s1, main_scene;
	//
	//SETTINGHANDLE vs = std::make_shared<VertexSetting>(VertexSetting());
	//SETTINGHANDLE ls = std::make_shared<LineSetting>(LineSetting());
	//SETTINGHANDLE ps0 = std::make_shared<GeometrySetting>(GeometrySetting());
	//SETTINGHANDLE ps1 = std::make_shared<GeometrySetting>(GeometrySetting());
	//
	//double sq = 1.0 / sqrt(2);
	//
	//GHANDLE v10 = Vertex(s, vs, { 0,0,0.0 }).getHandle();
	//GHANDLE v11 = Vertex(s, vs, { 4,0,0.0 }).getHandle();
	//GHANDLE v12 = Vertex(s, vs, { 4,4,0 }).getHandle();
	//GHANDLE v13 = Vertex(s, vs, { 0,4,0 }).getHandle();
	//GHANDLE v20 = Vertex(s1, vs, { 5,2,0 }).getHandle();
	//GHANDLE v21 = Vertex(s1, vs, { 8,5,0 }).getHandle();
	//GHANDLE v22 = Vertex(s1, vs, { 5,8,0 }).getHandle();
	//GHANDLE v23 = Vertex(s1, vs, { 2,5,0 }).getHandle();
	//GHANDLE v2c = Vertex(s1, vs, { 5,5,0 }).getHandle();
	//
	//GHANDLE l10 = LineSegment(s, ls, v10, v11).getHandle();
	//GHANDLE l11 = LineSegment(s, ls, v11, v12).getHandle();
	//GHANDLE l12 = LineSegment(s, ls, v12, v13).getHandle();
	//GHANDLE l13 = LineSegment(s, ls, v13, v10).getHandle();
	//
	//GHANDLE l20 = EllipticSegment(s1, ls, v20, v21, v2c).getHandle();
	//GHANDLE l21 = EllipticSegment(s1, ls, v21, v22, v2c).getHandle();
	//GHANDLE l22 = EllipticSegment(s1, ls, v22, v23, v2c).getHandle();
	//GHANDLE l23 = EllipticSegment(s1, ls, v23, v20, v2c).getHandle();
	//
	//auto lines1 = std::vector<GHANDLE>{ l10, l11, l12, l13 };
	//auto lines2 = std::vector<GHANDLE>{ l20, l21, l22, l23 };
	//
	//GHANDLE contour1 = primitive::Shape(s, ps0, s, std::move(lines1)).getHandle();
	//GHANDLE contour2 = primitive::Shape(s1, ps1, s1, std::move(lines2)).getHandle();
	//
	//auto& sh1 = s.get<primitive::Shape>(contour1);
	//auto& sh2 = s1.get<primitive::Shape>(contour2);
	//
	//auto& rsh = main_scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(main_scene, CSGOperation::Subtract, sh1, sh2));
	//
	//
	//int N = 100;
	//double a = -0.5;
	//double h = 8.5;
	//double b = h / N; 
	//{
	//	std::ofstream file("display.txt");
	//	for (int i = 0; i < N; i++) {
	//		for (int j = 0; j < N; j++) {
	//			auto p = vector3(b * j + a, h - b * i + a, 0);
	//			auto res = rsh.classify(p);
	//			file << (res == -1 ? '*' : (res == 0 ? 'X' : ' '));
	//		}
	//		file << std::endl;
	//	}
	//}
	//

	int i;
	std::cin >> i;
	std::cout << i;
	//return 0;
}


