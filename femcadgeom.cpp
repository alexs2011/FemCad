// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "MeshDrawer.hpp"
#include "CSGDrawer.hpp"
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
	Scene main_scene;

	SETTINGHANDLE vs = std::make_shared<VertexSetting>(VertexSetting());
#ifdef _testCurve
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	ls_1->setParameter("N", DoubleParameter(1));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting());
	ls_2->setParameter("N", DoubleParameter(5));
	ls_2->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());

	GHANDLE v0 = Vertex(s, vs, { -5,-5,0 }).getHandle();
	GHANDLE v1 = Vertex(s, vs, { 5,-5,0 }).getHandle();
	GHANDLE v2 = Vertex(s, vs, { -5,5,0 }).getHandle();
	GHANDLE v3 = Vertex(s, vs, { 5,5,0 }).getHandle();
	GHANDLE v4 = Vertex(s2, vs, { 0,0,0 }).getHandle();
	GHANDLE v5 = Vertex(s2, vs, { -2,0,0 }).getHandle();
	GHANDLE v6 = Vertex(s2, vs, { 0,2,0 }).getHandle();
	GHANDLE v7 = Vertex(s2, vs, { 1.0,2,0 }).getHandle();
	GHANDLE c = Vertex(s2, vs, { -3,3,0 }).getHandle();

	GHANDLE l0 = LineSegment(s, ls_1, v0, v1).getHandle();
	GHANDLE l1 = LineSegment(s, ls_1, v1, v3).getHandle();
	GHANDLE l2 = LineSegment(s, ls_1, v3, v2).getHandle();
	GHANDLE l3 = LineSegment(s, ls_1, v2, v0).getHandle();
	GHANDLE l4 = LineSegment(s2, ls_2, v4, v5).getHandle();
	GHANDLE l5 = EllipticSegment(s2, ls_2, v5, v6, c).getHandle();
	GHANDLE l6 = LineSegment(s2, ls_2, v6, v7).getHandle();
	GHANDLE l7 = LineSegment(s2, ls_2, v7, v4).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l0, l1, l2, l3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l4, l5, l6, l7 }).getHandle();

	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, v0, v1, v2, v3 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v5, v4, v6, v7 };
	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };

	std::shared_ptr<RectMeshView> mesh_base{ std::make_shared<RectMeshView>(rect_base) };
	MeshCombiner combiner{ RectMeshView(rect_base) };
	combiner.SetCriterion<OnePointCriterion>();

	combiner.AddMesh(mesh_form0);
	MeshElementSizeIsoMaxEdgeLength size(mesh_form0->mesh());
	combiner.AdjustMeshInitialization(size);
	size_t mode = 1;
	onSpacePress = ([&]()->void {
		if (combiner.AdjustIteration(size, mode) == 0)
			std::cout << "Mesh optimization complete" << std::endl;
	});

	globalMeshDrawer.draw(combiner);
	//globalMeshDrawer.draw(*mesh_form0);

	globalMeshDrawer.init();
#endif // !_testCurve
#ifdef _magnit
	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());

	SETTINGHANDLE ls_base = std::make_shared<LineSetting>(LineSetting());
	ls_base->setParameter("N", DoubleParameter(1));
	ls_base->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting()); // bound 1
	ls_1->setParameter("N", DoubleParameter(2));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 |
	ls_2->setParameter("N", DoubleParameter(5));
	ls_2->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_3 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 /
	ls_3->setParameter("N", DoubleParameter(2));
	ls_3->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_4 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_4->setParameter("N", DoubleParameter(4));
	ls_4->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_5 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_5->setParameter("N", DoubleParameter(3));
	ls_5->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_6 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 \"
	ls_6->setParameter("N", DoubleParameter(2));
	ls_6->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_7 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_7->setParameter("N", DoubleParameter(3));
	ls_7->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_8 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 |
	ls_8->setParameter("N", DoubleParameter(7));
	ls_8->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_9 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 -
	ls_9->setParameter("N", DoubleParameter(6));
	ls_9->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_10 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 -
	ls_10->setParameter("N", DoubleParameter(6));
	ls_10->setParameter("q", DoubleParameter(1.));

	GHANDLE vBase0 = Vertex(s, vs, { -1.5,-0.5,0 }).getHandle();
	GHANDLE vBase1 = Vertex(s, vs, { 1.5,-0.5,0 }).getHandle();
	GHANDLE vBase2 = Vertex(s, vs, { 1.5, 1.5,0 }).getHandle();
	GHANDLE vBase3 = Vertex(s, vs, { -1.5,1.5,0 }).getHandle();

	GHANDLE lBase0 = LineSegment(s, ls_base, vBase0, vBase1).getHandle();
	GHANDLE lBase1 = LineSegment(s, ls_base, vBase1, vBase2).getHandle();
	GHANDLE lBase2 = LineSegment(s, ls_base, vBase2, vBase3).getHandle();
	GHANDLE lBase3 = LineSegment(s, ls_base, vBase3, vBase0).getHandle();

	GHANDLE v0 = Vertex(s2, vs, { -0.2935,0,0 }).getHandle();
	GHANDLE v1 = Vertex(s2, vs, { -0.5015,0,0 }).getHandle();
	GHANDLE v2 = Vertex(s2, vs, { -0.5015,0.301757,0 }).getHandle();
	GHANDLE v3 = Vertex(s2, vs, { -0.453,0.353,0 }).getHandle();
	GHANDLE v4 = Vertex(s2, vs, { -0.22272,0.354,0 }).getHandle();
	GHANDLE v5 = Vertex(s2, vs, { -0.111866,0.354,0 }).getHandle();
	GHANDLE v6 = Vertex(s2, vs, { -0.091866,0.351,0 }).getHandle();
	GHANDLE v7 = Vertex(s2, vs, { 0.,0.35,0 }).getHandle();
	GHANDLE v8 = Vertex(s2, vs, { 0.091866,0.351,0 }).getHandle();
	GHANDLE v9 = Vertex(s2, vs, { 0.111866,0.354,0 }).getHandle();
	GHANDLE v10 = Vertex(s2, vs, { 0.22272,0.354,0 }).getHandle();
	GHANDLE v11 = Vertex(s2, vs, { 0.453,0.353,0 }).getHandle();
	GHANDLE v12 = Vertex(s2, vs, { 0.5015,0.301757,0 }).getHandle();
	GHANDLE v13 = Vertex(s2, vs, { 0.5015,0,0 }).getHandle();
	GHANDLE v14 = Vertex(s2, vs, { 0.2935,0,0 }).getHandle();
	GHANDLE v15 = Vertex(s2, vs, { 0.2935,0.157,0 }).getHandle();
	GHANDLE v16 = Vertex(s2, vs, { 0.18,0.157,0 }).getHandle();
	GHANDLE v17 = Vertex(s2, vs, { 0.,0.154,0 }).getHandle();
	GHANDLE v18 = Vertex(s2, vs, { -0.18,0.157,0 }).getHandle();
	GHANDLE v19 = Vertex(s2, vs, { -0.2935,0.157,0 }).getHandle();

	GHANDLE l0 = LineSegment(s2, ls_1, v0, v1).getHandle(); // bound 1
	GHANDLE l1 = LineSegment(s2, ls_2, v1, v2).getHandle(); // bound 2 part 1 |
	GHANDLE l2 = LineSegment(s2, ls_3, v2, v3).getHandle(); // bound 2 part 1 /
	GHANDLE l3 = LineSegment(s2, ls_4, v3, v4).getHandle(); // bound 2 part 1 -
	GHANDLE l4 = LineSegment(s2, ls_5, v4, v5).getHandle(); // bound 2 part 1 -
	GHANDLE l5 = LineSegment(s2, ls_6, v5, v6).getHandle(); // bound 2 part 1 \"
	GHANDLE l6 = LineSegment(s2, ls_7, v6, v7).getHandle(); // bound 2 part 1 -
	GHANDLE l7 = LineSegment(s2, ls_7, v7, v8).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l8 = LineSegment(s2, ls_6, v8, v9).getHandle(); // bound 2 part 1 / (symmetry)
	GHANDLE l9 = LineSegment(s2, ls_5, v9, v10).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l10 = LineSegment(s2, ls_4, v10, v11).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l11 = LineSegment(s2, ls_3, v11, v12).getHandle(); // bound 2 part 1 \ (symmetry)
	GHANDLE l12 = LineSegment(s2, ls_2, v12, v13).getHandle(); // bound 2 part 1 | (symmetry)
	GHANDLE l13 = LineSegment(s2, ls_1, v13, v14).getHandle(); // bound 1
	GHANDLE l14 = LineSegment(s2, ls_8, v14, v15).getHandle(); // bound 2 part 2 | (symmetry)
	GHANDLE l15 = LineSegment(s2, ls_9, v15, v16).getHandle(); // bound 2 part 2 - (symmetry)
	GHANDLE l16 = LineSegment(s2, ls_10, v16, v17).getHandle(); // bound 2 part 2 - (symmetry)
	GHANDLE l17 = LineSegment(s2, ls_10, v17, v18).getHandle(); // bound 2 part 2 -
	GHANDLE l18 = LineSegment(s2, ls_9, v18, v19).getHandle(); // bound 2 part 2 -
	GHANDLE l19 = LineSegment(s2, ls_8, v19, v0).getHandle(); // bound 2 part 2 |

	GHANDLE shape_base = primitive::Shape(s, ps, s, { lBase0, lBase1, lBase2, lBase3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15, l16, l17, l18, l19 }).getHandle();

	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, v0, v1, v2, v3 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v1, v13, v0, v14 };

	std::shared_ptr<RectMeshView> mesh_base{ std::make_shared<RectMeshView>(rect_base) };
	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };

	//globalMeshDrawer.draw(*mesh_form0);

	MeshCombiner combiner{ RectMeshView(rect_base) };
	combiner.SetCriterion<OnePointCriterion>();
	
	combiner.AddMesh(mesh_form0);
	MeshElementSizeIsoMaxEdgeLength size(mesh_form0->mesh());
	size_t mode = 1;
	onSpacePress = ([&]()->void {
		if (combiner.AdjustIteration(size, mode) == 0)
			std::cout << "Mesh optimization complete" << std::endl;
	});
	
	globalMeshDrawer.draw(combiner);
	globalMeshDrawer.init();
#endif

#ifndef _testBSP
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	ls_1->setParameter("N", DoubleParameter(1));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting());
	ls_2->setParameter("N", DoubleParameter(15));
	ls_2->setParameter("q", DoubleParameter(1.));
	SETTINGHANDLE ls_3 = std::make_shared<LineSetting>(LineSetting());
	ls_3->setParameter("N", DoubleParameter(15));
	ls_3->setParameter("q", DoubleParameter(1 / 1.));
	SETTINGHANDLE ls_4 = std::make_shared<LineSetting>(LineSetting());
	ls_4->setParameter("N", DoubleParameter(15));
	ls_4->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());

	GHANDLE v0 = Vertex(s, vs, { -1,-3,0 }).getHandle();
	GHANDLE v1 = Vertex(s, vs, { 5,-3,0 }).getHandle();
	GHANDLE v2 = Vertex(s, vs, { -1,3,0 }).getHandle();
	GHANDLE v3 = Vertex(s, vs, { 5,3,0 }).getHandle();
	GHANDLE v4 = Vertex(s2, vs, { 0,-1,0 }).getHandle();
	GHANDLE v5 = Vertex(s2, vs, { 4,-1,0 }).getHandle();
	GHANDLE v6 = Vertex(s2, vs, { 0,1,0 }).getHandle();
	GHANDLE v7 = Vertex(s2, vs, { 4.0,1,0 }).getHandle();
	GHANDLE c = Vertex(s2, vs, { 2,0,0 }).getHandle();

	GHANDLE l0 = LineSegment(s, ls_1, v0, v1).getHandle();
	GHANDLE l1 = LineSegment(s, ls_1, v1, v3).getHandle();
	GHANDLE l2 = LineSegment(s, ls_1, v3, v2).getHandle();
	GHANDLE l3 = LineSegment(s, ls_1, v2, v0).getHandle();

	GHANDLE l4 = EllipticSegment(s2, ls_2, v4, v5, c).getHandle();
	GHANDLE l5 = EllipticSegment(s2, ls_4, v5, v7, c).getHandle();
	GHANDLE l6 = EllipticSegment(s2, ls_3, v7, v6, c).getHandle();
	GHANDLE l7 = EllipticSegment(s2, ls_4, v6, v4, c).getHandle();

	//GHANDLE v8 = Vertex(s, vs, { -2,-2,0 }).getHandle();
	//GHANDLE v9 = Vertex(s, vs, { 1,-2,0 }).getHandle();
	//GHANDLE v10 = Vertex(s, vs, { -2,2,0 }).getHandle();
	//GHANDLE v11 = Vertex(s, vs, { 1,2,0 }).getHandle();
	//
	//GHANDLE l8 = LineSegment(s, ls_1, v0, v1).getHandle();
	//GHANDLE l9 = LineSegment(s, ls_1, v1, v3).getHandle();
	//GHANDLE l10 = LineSegment(s, ls_1, v3, v2).getHandle();
	//GHANDLE l11 = LineSegment(s, ls_1, v2, v0).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l0, l1, l2, l3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l4, l5, l6, l7 }).getHandle();
	//GHANDLE shape_form1 = primitive::Shape(s2, ps, s2, { l8, l9, l10, l11 }).getHandle();


	// всё то делает rect - это задаёт противоположные границы геометрии, что нужно для построения сетки
	// также задает отступы разрядки на всей границе, а также на отдельных её линиях
	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, v0, v1, v2, v3 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v4, v5, v6, v7 };

	//auto& sh_form1 = s2.get<primitive::Shape>(shape_form1);
	//RectView rect_form1{ sh_form1, v8, v9, v10, v11 };

	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };

	std::shared_ptr<RectMeshView> mesh_base{ std::make_shared<RectMeshView>(rect_base) };

	//auto& rsh = main_scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(main_scene, CSGOperation::Intersect, sh_form0, sh_base));
	//RectView rect_form1{ sh_form1, v8, v9, v10, v11 };
	//std::shared_ptr<RectMeshView> mesh_form1{ std::make_shared<RectMeshView>(rect_form1) };

	//globalCSGDrawer.draw(&rsh);
	//globalCSGDrawer.init();

	MeshCombiner combiner{ RectMeshView(rect_base) };
	combiner.SetCriterion<OnePointCriterion>();

	combiner.AddMesh(mesh_form0);
	MeshElementSizeIsoMaxEdgeLength size(mesh_form0->mesh());

	//LambdaElementSize<double> size([](const vector3& p) {return 0.25; });
	//combiner.AdjustMesh(size);
	//combiner.AddIntersectingMesh(mesh_form0);

	combiner.AdjustMeshInitialization(size);
	size_t mode = 1; 
	onSpacePress = ([&]()->void { 
		if (combiner.AdjustIteration(size, mode) == 0) 
			std::cout << "Mesh optimization complete" << std::endl; 
	});

	globalMeshDrawer.draw(combiner);
	//globalMeshDrawer.draw(*mesh_base);
	//globalMeshDrawer.draw(*rsh);

	globalMeshDrawer.init();

#endif
#ifdef _test
	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	GHANDLE v0 = Vertex(s, vs, { -2,-2,0 }).getHandle();
	GHANDLE v1 = Vertex(s, vs, { 2,-2,0 }).getHandle();
	GHANDLE v2 = Vertex(s, vs, { 2,0.5,0 }).getHandle();
	GHANDLE v3 = Vertex(s, vs, { 1,0.5,0 }).getHandle();
	GHANDLE v4 = Vertex(s, vs, { 1,0,0 }).getHandle();
	GHANDLE v5 = Vertex(s, vs, { 0,-1,0 }).getHandle();
	GHANDLE v6 = Vertex(s, vs, { -1,0,0 }).getHandle();
	GHANDLE v7 = Vertex(s, vs, { -1,1,0 }).getHandle();
	GHANDLE v8 = Vertex(s, vs, { -2,1,0 }).getHandle();

	GHANDLE l0 = LineSegment(s, ls_1, v0, v1).getHandle();
	GHANDLE l1 = LineSegment(s, ls_1, v1, v2).getHandle();
	GHANDLE l2 = LineSegment(s, ls_1, v2, v3).getHandle();
	GHANDLE l3 = LineSegment(s, ls_1, v3, v4).getHandle();
	GHANDLE l4 = LineSegment(s, ls_1, v4, v5).getHandle();
	GHANDLE l5 = LineSegment(s, ls_1, v5, v6).getHandle();
	GHANDLE l6 = LineSegment(s, ls_1, v6, v7).getHandle();
	GHANDLE l7 = LineSegment(s, ls_1, v7, v8).getHandle();
	GHANDLE l8 = LineSegment(s, ls_1, v8, v0).getHandle();


	GHANDLE shape_form0 = primitive::Shape(s, ps, s, { l0, l1, l2, l3, l4, l5, l6, l7, l8 }).getHandle();
	auto& sh_form0 = s.get<primitive::Shape>(shape_form0);

	std::cout << sh_form0.classify(vector3{});

	GHANDLE v10 = Vertex(s, vs, { 1.1,0. ,0 }).getHandle();
	GHANDLE v11 = Vertex(s, vs, { 1.9,0. ,0 }).getHandle();
	GHANDLE v12 = Vertex(s, vs, { 2,1,0 }).getHandle();
	GHANDLE v13 = Vertex(s, vs, { 1,1,0 }).getHandle();

	GHANDLE l10 = LineSegment(s, ls_1, v10, v11).getHandle();
	GHANDLE l11 = LineSegment(s, ls_1, v11, v12).getHandle();
	GHANDLE l12 = LineSegment(s, ls_1, v12, v13).getHandle();
	GHANDLE l13 = LineSegment(s, ls_1, v13, v10).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l10, l11, l12, l13 }).getHandle();
	auto& sh_base = s.get<primitive::Shape>(shape_base);

	auto& rsh = main_scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(main_scene, CSGOperation::Union, sh_base, sh_form0));

	globalCSGDrawer.draw(&rsh);
	globalCSGDrawer.init();

#endif

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
}


