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
#include "g_csg.h"

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
	GHANDLE c = Vertex(s2, vs, { 3,-3,0 }).getHandle();
	
	
	GHANDLE v10 = Vertex(s2, vs, { -4.0,0.5,0 }).getHandle();
	GHANDLE v11 = Vertex(s2, vs, { 3.0,0.5,0 }).getHandle();
	GHANDLE v12 = Vertex(s2, vs, { 3.0,1.0,0 }).getHandle();
	GHANDLE v13 = Vertex(s2, vs, { -4.0,1.0,0 }).getHandle();

	GHANDLE l0 = LineSegment(s, ls_1, v0, v1).getHandle();
	GHANDLE l1 = LineSegment(s, ls_1, v1, v3).getHandle();
	GHANDLE l2 = LineSegment(s, ls_1, v3, v2).getHandle();
	GHANDLE l3 = LineSegment(s, ls_1, v2, v0).getHandle();
	GHANDLE l4 = LineSegment(s2, ls_2, v5, v4).getHandle();
	GHANDLE l5 = EllipticSegment(s2, ls_2, v6, v5, c).getHandle();
	GHANDLE l6 = LineSegment(s2, ls_2, v7, v6).getHandle();
	GHANDLE l7 = LineSegment(s2, ls_2, v4, v7).getHandle();

	GHANDLE l10 = LineSegment(s2, ls_2, v10, v11).getHandle();
	GHANDLE l11 = LineSegment(s2, ls_1, v11, v12).getHandle();
	GHANDLE l12 = LineSegment(s2, ls_2, v12, v13).getHandle();
	GHANDLE l13 = LineSegment(s2, ls_1, v13, v10).getHandle();


	GHANDLE shape_base = primitive::Shape(s, ps, s, { l0, l1, l2, l3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l4, l5, l6, l7 }).getHandle();
	GHANDLE shape_form1 = primitive::Shape(s2, ps, s2, { l10, l11, l12, l13 }).getHandle();

	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, v0, v1, v2, v3 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v5, v4, v6, v7 };
	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };

	auto& sh_form1 = s2.get<primitive::Shape>(shape_form1);
	RectView rect_form1{ sh_form1, v11, v10, v12, v13 };
	std::shared_ptr<RectMeshView> mesh_form1{ std::make_shared<RectMeshView>(rect_form1) };

	std::shared_ptr<RectMeshView> mesh_base{ std::make_shared<RectMeshView>(rect_base) };
	MeshCombiner combiner{ RectMeshView(rect_base) };
	combiner.SetCriterion<OnePointCriterion>(0.00000001);

	mesh_form0->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_form1->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();

	std::vector<std::pair<std::shared_ptr<ElementGeometry>, CSGOperation>> els = {
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_form0), CSGOperation::Union),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_form1), CSGOperation::Union) };

	std::shared_ptr<CSG> csg = std::make_shared<CSG>(ps, els);

	csg->setIsoSize<LambdaElementSize<double>>([](const vector3&) {return 0.5f; });
	//combiner.AddMesh(mesh_form0);
	//combiner.AddMesh(mesh_form1);
	combiner.AddMesh(csg);

	combiner.AdjustMeshInitialization();
	size_t mode = 1;
	onSpacePress = ([&]()->void {
		if (combiner.AdjustIteration((IElementSize<double>&)*csg, mode) == 0)
			std::cout << "Mesh optimization complete" << std::endl;
	});

	//globalMeshDrawer.draw(*mesh_form0, vector3(0.5f, 0.2f, 0.7f));
	//globalMeshDrawer.draw(*mesh_form1, vector3(0.5f, 0.7f, 0.2f));
	globalMeshDrawer.draw(combiner);

	globalMeshDrawer.init();
#endif // !_testCurve
#ifndef _magnit
	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());
	ps->addParameter("Name", SettingParameter<std::string>("Iron"));
	SETTINGHANDLE ps_air = std::make_shared<GeometrySetting>(GeometrySetting());
	ps_air->addParameter("Name", SettingParameter<std::string>("Air"));
	SETTINGHANDLE ps_air_hole = std::make_shared<GeometrySetting>(GeometrySetting());
	ps_air_hole->addParameter("Name", SettingParameter<std::string>("Air hole"));
	SETTINGHANDLE ps_coil_lft = std::make_shared<GeometrySetting>(GeometrySetting());
	ps_coil_lft->addParameter("Name", SettingParameter<std::string>("Coil left"));
	SETTINGHANDLE ps_coil_rgt = std::make_shared<GeometrySetting>(GeometrySetting());
	ps_coil_rgt->addParameter("Name", SettingParameter<std::string>("Coil right"));
	SETTINGHANDLE ps_shim = std::make_shared<GeometrySetting>(GeometrySetting());
	ps_shim->addParameter("Name", SettingParameter<std::string>("Shimm"));

	SETTINGHANDLE ls_base = std::make_shared<LineSetting>(LineSetting());
	ls_base->setParameter("N", DoubleParameter(1));
	ls_base->setParameter("q", DoubleParameter(1));


	SETTINGHANDLE ls_air_top = std::make_shared<LineSetting>(LineSetting());
	ls_air_top->addParameter("Name", SettingParameter<std::string>("Zero value"));
	ls_air_top->setParameter("N", DoubleParameter(20));
	ls_air_top->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_air_bottom1 = std::make_shared<LineSetting>(LineSetting());
	ls_air_bottom1->addParameter("Name", SettingParameter<std::string>("Zero flow"));
	ls_air_bottom1->setParameter("N", DoubleParameter(10));
	ls_air_bottom1->setParameter("q", DoubleParameter(1.1));

	SETTINGHANDLE ls_air_bottom2 = std::make_shared<LineSetting>(LineSetting());
	ls_air_bottom2->addParameter("Name", SettingParameter<std::string>("Zero flow"));
	ls_air_bottom2->setParameter("N", DoubleParameter(10));
	ls_air_bottom2->setParameter("q", DoubleParameter(1/1.1));

	SETTINGHANDLE ls_air_side_r = std::make_shared<LineSetting>(LineSetting());
	ls_air_side_r->addParameter("Name", SettingParameter<std::string>("Zero value"));
	ls_air_side_r->setParameter("N", DoubleParameter(20));
	ls_air_side_r->setParameter("q", DoubleParameter(1.01));

	SETTINGHANDLE ls_air_side_l = std::make_shared<LineSetting>(LineSetting());
	ls_air_side_l->addParameter("Name", SettingParameter<std::string>("Zero value"));
	ls_air_side_l->setParameter("N", DoubleParameter(20));
	ls_air_side_l->setParameter("q", DoubleParameter(1/1.01));

	SETTINGHANDLE ls_cut_circle = std::make_shared<LineSetting>(LineSetting());
	ls_cut_circle->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_cut_circle->setParameter("N", DoubleParameter(14));
	ls_cut_circle->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting()); // bound 1
	ls_1->addParameter("Name", SettingParameter<std::string>("Zero flow"));
	ls_1->setParameter("N", DoubleParameter(2));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 |
	ls_2->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_2->setParameter("N", DoubleParameter(5));
	ls_2->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_3 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 /
	ls_3->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_3->setParameter("N", DoubleParameter(2));
	ls_3->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_4 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_4->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_4->setParameter("N", DoubleParameter(4));
	ls_4->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_5 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_5->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_5->setParameter("N", DoubleParameter(3));
	ls_5->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_6 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 \"
	ls_6->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_6->setParameter("N", DoubleParameter(2));
	ls_6->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_7 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 1 -
	ls_7->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_7->setParameter("N", DoubleParameter(3));
	ls_7->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_8 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 |
	ls_8->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_8->setParameter("N", DoubleParameter(7));
	ls_8->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_9 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 -
	ls_9->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_9->setParameter("N", DoubleParameter(6));
	ls_9->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_10 = std::make_shared<LineSetting>(LineSetting()); // bound 2 part 2 -
	ls_10->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_10->setParameter("N", DoubleParameter(6));
	ls_10->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_winding = std::make_shared<LineSetting>(LineSetting());
	ls_winding->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_winding->setParameter("N", DoubleParameter(6));
	ls_winding->setParameter("q", DoubleParameter(1.));

	//proj setting
	SETTINGHANDLE ls_11 = std::make_shared<LineSetting>(LineSetting()); // bound 1 down
	ls_11->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_11->setParameter("N", DoubleParameter(6));
	ls_11->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_12 = std::make_shared<LineSetting>(LineSetting()); // bound 1 up
	ls_12->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_12->setParameter("N", DoubleParameter(3));
	ls_12->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_13 = std::make_shared<LineSetting>(LineSetting()); // bound 2
	ls_13->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_13->setParameter("N", DoubleParameter(1));
	ls_13->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_14 = std::make_shared<LineSetting>(LineSetting()); // bound 2
	ls_14->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_14->setParameter("N", DoubleParameter(2));
	ls_14->setParameter("q", DoubleParameter(1.));

	SETTINGHANDLE ls_15 = std::make_shared<LineSetting>(LineSetting()); // bound 2
	ls_15->addParameter("Name", SettingParameter<std::string>("Internal boundary"));
	ls_15->setParameter("N", DoubleParameter(3));
	ls_15->setParameter("q", DoubleParameter(1.));

	GHANDLE vBase0 = Vertex(s, vs, { -1.02,-0.02,0 }).getHandle();
	GHANDLE vBase1 = Vertex(s, vs, { 1.02,-0.02,0 }).getHandle();
	GHANDLE vBase2 = Vertex(s, vs, { 1.02, 1.02,0 }).getHandle();
	GHANDLE vBase3 = Vertex(s, vs, { -1.02,1.02,0 }).getHandle();

	GHANDLE lBase0 = LineSegment(s, ls_base, vBase0, vBase1).getHandle();
	GHANDLE lBase1 = LineSegment(s, ls_base, vBase1, vBase2).getHandle();
	GHANDLE lBase2 = LineSegment(s, ls_base, vBase2, vBase3).getHandle();
	GHANDLE lBase3 = LineSegment(s, ls_base, vBase3, vBase0).getHandle();

	GHANDLE vTank0 = Vertex(s, vs, { -1.,0.,0 }).getHandle();
	GHANDLE vTankc = Vertex(s, vs, { 0.,0.,0 }).getHandle();
	GHANDLE vTank1 = Vertex(s, vs, { 1.,0.,0 }).getHandle();
	GHANDLE vTank2 = Vertex(s, vs, { 1., 1.,0 }).getHandle();
	GHANDLE vTank3 = Vertex(s, vs, { -1.,1.,0 }).getHandle();

	GHANDLE lTank0a = LineSegment(s, ls_air_bottom1, vTank0, vTankc).getHandle();
	GHANDLE lTank0b = LineSegment(s, ls_air_bottom2, vTankc, vTank1).getHandle();
	GHANDLE lTank1 = LineSegment(s, ls_air_side_r, vTank1, vTank2).getHandle();
	GHANDLE lTank2 = LineSegment(s, ls_air_top, vTank2, vTank3).getHandle();
	GHANDLE lTank3 = LineSegment(s, ls_air_side_l, vTank3, vTank0).getHandle();

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

	GHANDLE l0 =  LineSegment(s2, ls_1, v1 , v0 ).getHandle(); // bound 1
	GHANDLE l1 =  LineSegment(s2, ls_2, v2 , v1 ).getHandle(); // bound 2 part 1 |
	GHANDLE l2 =  LineSegment(s2, ls_3 , v3 , v2 ).getHandle(); // bound 2 part 1 /
	GHANDLE l3 =  LineSegment(s2, ls_4 , v4 , v3 ).getHandle(); // bound 2 part 1 -
	GHANDLE l4 =  LineSegment(s2, ls_5 , v5 , v4 ).getHandle(); // bound 2 part 1 -
	GHANDLE l5 =  LineSegment(s2, ls_6 , v6 , v5 ).getHandle(); // bound 2 part 1 \"
	GHANDLE l6 =  LineSegment(s2, ls_7 , v7 , v6 ).getHandle(); // bound 2 part 1 -
	GHANDLE l7 =  LineSegment(s2, ls_7 , v8 , v7 ).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l8 =  LineSegment(s2, ls_6 , v9 , v8 ).getHandle(); // bound 2 part 1 / (symmetry)
	GHANDLE l9 =  LineSegment(s2, ls_5 , v10, v9 ).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l10 = LineSegment(s2, ls_4 , v11, v10).getHandle(); // bound 2 part 1 - (symmetry)
	GHANDLE l11 = LineSegment(s2, ls_3 , v12, v11).getHandle(); // bound 2 part 1 \ (symmetry)
	GHANDLE l12 = LineSegment(s2, ls_2 , v13, v12).getHandle(); // bound 2 part 1 | (symmetry)
	GHANDLE l13 = LineSegment(s2, ls_1 , v14, v13).getHandle(); // bound 1
	GHANDLE l14 = LineSegment(s2, ls_8 , v15, v14).getHandle(); // bound 2 part 2 | (symmetry)
	GHANDLE l15 = LineSegment(s2, ls_9 , v16, v15).getHandle(); // bound 2 part 2 - (symmetry)
	GHANDLE l16 = LineSegment(s2, ls_10, v17, v16).getHandle(); // bound 2 part 2 - (symmetry)
	GHANDLE l17 = LineSegment(s2, ls_10, v18, v17).getHandle(); // bound 2 part 2 -
	GHANDLE l18 = LineSegment(s2, ls_9 , v19, v18).getHandle(); // bound 2 part 2 -
	GHANDLE l19 = LineSegment(s2, ls_8 , v0 , v19).getHandle(); // bound 2 part 2 |

	// left cut triangle
	GHANDLE vLeftTri0 = Vertex(s2, vs, { -0.480633,0.325,0 }).getHandle();
	GHANDLE vLeftTri1 = Vertex(s2, vs, { -0.475,0.325,0 }).getHandle();
	GHANDLE vLeftTri2 = Vertex(s2, vs, { -0.475,0.330524,0 }).getHandle();
	GHANDLE vLeftTri3 = Vertex(s2, vs, { -0.4778165,0.327762,0 }).getHandle();

	GHANDLE lLeftTri0 = LineSegment(s2, ls_base, vLeftTri0, vLeftTri1).getHandle();
	GHANDLE lLeftTri1 = LineSegment(s2, ls_base, vLeftTri1, vLeftTri2).getHandle();
	GHANDLE lLeftTri2 = LineSegment(s2, ls_base, vLeftTri2, vLeftTri3).getHandle();
	GHANDLE lLeftTri3 = LineSegment(s2, ls_base, vLeftTri3, vLeftTri0).getHandle();

	// right cut triangle
	GHANDLE vRightTri0 = Vertex(s2, vs, { 0.480633,0.325,0 }).getHandle();
	GHANDLE vRightTri1 = Vertex(s2, vs, { 0.475,0.325,0 }).getHandle();
	GHANDLE vRightTri2 = Vertex(s2, vs, { 0.475,0.330524,0 }).getHandle();
	GHANDLE vRightTri3 = Vertex(s2, vs, { 0.4778165,0.327762,0 }).getHandle();

	GHANDLE lRightTri0 = LineSegment(s2, ls_base, vRightTri1, vRightTri0).getHandle();
	GHANDLE lRightTri1 = LineSegment(s2, ls_base, vRightTri2, vRightTri1).getHandle();
	GHANDLE lRightTri2 = LineSegment(s2, ls_base, vRightTri3, vRightTri2).getHandle();
	GHANDLE lRightTri3 = LineSegment(s2, ls_base, vRightTri0, vRightTri3).getHandle();

	// cut quadrangle
	GHANDLE vQuad0 = Vertex(s2, vs, { -0.005,0.34,0 }).getHandle();
	GHANDLE vQuad1 = Vertex(s2, vs, { 0.005,0.34,0 }).getHandle();
	GHANDLE vQuad2 = Vertex(s2, vs, { 0.005,0.4,0 }).getHandle();
	GHANDLE vQuad3 = Vertex(s2, vs, { -0.005,0.4,0 }).getHandle();

	GHANDLE lQuad0 = LineSegment(s2, ls_base, vQuad0, vQuad1).getHandle();
	GHANDLE lQuad1 = LineSegment(s2, ls_base, vQuad1, vQuad2).getHandle();
	GHANDLE lQuad2 = LineSegment(s2, ls_base, vQuad2, vQuad3).getHandle();
	GHANDLE lQuad3 = LineSegment(s2, ls_base, vQuad3, vQuad0).getHandle();

	// cut circle
	GHANDLE vCircle0 = Vertex(s2, vs, { -0.008,0.065,0 }).getHandle();
	GHANDLE vCircle1 = Vertex(s2, vs, { 0.,0.057,0 }).getHandle();
	GHANDLE vCircle2 = Vertex(s2, vs, { 0.008,0.065,0 }).getHandle();
	GHANDLE vCircle3 = Vertex(s2, vs, { 0.,0.073,0 }).getHandle();
	GHANDLE vCircleCenter = Vertex(s2, vs, { 0.,0.065,0 }).getHandle();
	
	GHANDLE lCircle0 = EllipticSegment(s2, ls_cut_circle, vCircle0, vCircle1, vCircleCenter).getHandle();
	GHANDLE lCircle1 = EllipticSegment(s2, ls_cut_circle, vCircle1, vCircle2, vCircleCenter).getHandle();
	GHANDLE lCircle2 = EllipticSegment(s2, ls_cut_circle, vCircle2, vCircle3, vCircleCenter).getHandle();
	GHANDLE lCircle3 = EllipticSegment(s2, ls_cut_circle, vCircle3, vCircle0, vCircleCenter).getHandle();

	// left winding
	GHANDLE vLeftWinding0 = Vertex(s2, vs, { -0.287,0.052,0 }).getHandle();
	GHANDLE vLeftWinding1 = Vertex(s2, vs, { -0.185,0.052,0 }).getHandle();
	GHANDLE vLeftWinding2 = Vertex(s2, vs, { -0.185,0.156,0 }).getHandle();
	GHANDLE vLeftWinding3 = Vertex(s2, vs, { -0.287,0.156,0 }).getHandle();

	GHANDLE lLeftWinding0 = LineSegment(s2, ls_winding, vLeftWinding0, vLeftWinding1).getHandle();
	GHANDLE lLeftWinding1 = LineSegment(s2, ls_winding, vLeftWinding1, vLeftWinding2).getHandle();
	GHANDLE lLeftWinding2 = LineSegment(s2, ls_winding, vLeftWinding2, vLeftWinding3).getHandle();
	GHANDLE lLeftWinding3 = LineSegment(s2, ls_winding, vLeftWinding3, vLeftWinding0).getHandle();

	// right winding
	GHANDLE vRightWinding0 = Vertex(s2, vs, { 0.287,0.052,0 }).getHandle();
	GHANDLE vRightWinding1 = Vertex(s2, vs, { 0.185,0.052,0 }).getHandle();
	GHANDLE vRightWinding2 = Vertex(s2, vs, { 0.185,0.156,0 }).getHandle();
	GHANDLE vRightWinding3 = Vertex(s2, vs, { 0.287,0.156,0 }).getHandle();

	GHANDLE lRightWinding0 = LineSegment(s2, ls_winding, vRightWinding1, vRightWinding0).getHandle();
	GHANDLE lRightWinding1 = LineSegment(s2, ls_winding, vRightWinding2, vRightWinding1).getHandle();
	GHANDLE lRightWinding2 = LineSegment(s2, ls_winding, vRightWinding3, vRightWinding2).getHandle();
	GHANDLE lRightWinding3 = LineSegment(s2, ls_winding, vRightWinding0, vRightWinding3).getHandle();

	// projection
	GHANDLE vProj0 = Vertex(s2, vs, { -0.119765,0.036,0 }).getHandle();
	GHANDLE vProj1 = Vertex(s2, vs, { 0.119765,0.036,0 }).getHandle();
	GHANDLE vProj2 = Vertex(s2, vs, { 0.167748,0.078694,0 }).getHandle();
	GHANDLE vProj3 = Vertex(s2, vs, { 0.18,0.115737,0 }).getHandle();
	GHANDLE vProj4 = Vertex(s2, vs, { 0.18,0.157,0 }).getHandle();
	GHANDLE vProj5 = Vertex(s2, vs, { 0.,0.154,0 }).getHandle();
	GHANDLE vProj6 = Vertex(s2, vs, { -0.18,0.157,0 }).getHandle();
	GHANDLE vProj7 = Vertex(s2, vs, { -0.18,0.115737,0 }).getHandle();
	GHANDLE vProj8 = Vertex(s2, vs, { -0.167748,0.078694,0 }).getHandle();

	auto pl0 = s2.get<Vertex>(vProj1).position();
	auto pl1 = s2.get<Vertex>(vProj2).position();
	auto t0 = s2.get<Vertex>(vProj3).position() - pl0;
	auto normal = (pl0 - pl1) ^ vector3::Z();
	auto center = (pl0 + pl1);

	// (2 * l0.x + normal.x * t + center.x) * t0.x = (2 * l0.y + normal.y * t + center.y) * t0.y
	// t * (normal.x * t0.x - normal.y * t0.y) = (2 * l0.y + center.y) * t0.y - (2 * l0.x + center.x) * t0.x
	auto t = ((2 * pl0.y + center.y) * t0.y - (2 * pl0.x + center.x) * t0.x) / (normal.x * t0.x - normal.y * t0.y);
	auto cent = (normal * t + center) / 2;

	GHANDLE vProjCenterR = Vertex(s2, vs, cent).getHandle();
	GHANDLE vProjCenterL = Vertex(s2, vs, { -cent.x,cent.y,0 }).getHandle();

	GHANDLE lProj0 = LineSegment(s2, ls_11, vProj0, vProj1).getHandle(); // bound 1
	GHANDLE lProj1 = EllipticSegment(s2, ls_15, vProj1, vProj2, vProjCenterR).getHandle(); // bound 2
	//GHANDLE lProj1 = LineSegment(s2, ls_15, vProj1, vProj2).getHandle(); // bound 2
	GHANDLE lProj2 = LineSegment(s2, ls_14, vProj2, vProj3).getHandle(); // bound 2
	GHANDLE lProj3 = LineSegment(s2, ls_13, vProj3, vProj4).getHandle(); // bound 2
	GHANDLE lProj4 = LineSegment(s2, ls_12, vProj4, vProj5).getHandle(); // bound 1
	GHANDLE lProj5 = LineSegment(s2, ls_12, vProj5, vProj6).getHandle(); // bound 1
	GHANDLE lProj6 = LineSegment(s2, ls_13, vProj6, vProj7).getHandle(); // bound 2
	GHANDLE lProj7 = LineSegment(s2, ls_14, vProj7, vProj8).getHandle(); // bound 2
	GHANDLE lProj8 = EllipticSegment(s2, ls_15, vProj8, vProj0, vProjCenterL).getHandle(); // bound 2
	//GHANDLE lProj8 = LineSegment(s2, ls_15, vProj8, vProj0).getHandle(); // bound 2

	// vProj0, vProj1 = (-0.119765, 0.036) - 0.119765, 0.036



	// left little thing
	GHANDLE vLeftThingy0 = Vertex(s2, vs, { -0.11,0.0352,0 }).getHandle();
	GHANDLE vLeftThingy1 = Vertex(s2, vs, { -0.1008,0.0352,0 }).getHandle();
	//GHANDLE vLeftThingy2 = Vertex(s2, vs, { -0.1,0.0356,0 }).getHandle();
	GHANDLE vLeftThingy3 = Vertex(s2, vs, { -0.0992,0.036,0 }).getHandle();
	GHANDLE vLeftThingy4 = Vertex(s2, vs, { -0.119765,0.036,0 }).getHandle();
	//GHANDLE vLeftThingyCenter_1 = Vertex(s2, vs, { -0.1008,0.036,0 }).getHandle();
	//GHANDLE vLeftThingyCenter_2 = Vertex(s2, vs, { -0.0991506,0.0349011,0 }).getHandle();

	
	GHANDLE lLeftThingy0 = LineSegment(s2, ls_base, vLeftThingy0, vLeftThingy1).getHandle();
	GHANDLE lLeftThingy1 = LineSegment(s2, ls_base, vLeftThingy1, vLeftThingy3).getHandle();
	//GHANDLE lLeftThingy2 = LineSegment(s2, ls_base, vLeftThingy2, vLeftThingy3).getHandle();
	GHANDLE lLeftThingy3 = LineSegment(s2, ls_base, vLeftThingy3, vLeftThingy4).getHandle();
	//GHANDLE lLeftThingy4 = EllipticSegment(s2, ls_base, vLeftThingy4, vLeftThingy0, vProjCenterL).getHandle();
	GHANDLE lLeftThingy4 = LineSegment(s2, ls_base, vLeftThingy4, vLeftThingy0).getHandle();

	// right little thing
	GHANDLE vRightThingy0 = Vertex(s2, vs, { 0.1008,0.0352,0 }).getHandle();
	GHANDLE vRightThingy1 = Vertex(s2, vs, { 0.11,0.0352,0 }).getHandle();
	GHANDLE vRightThingy2 = Vertex(s2, vs, { 0.119765,0.036,0 }).getHandle();
	GHANDLE vRightThingy3 = Vertex(s2, vs, { 0.0992,0.036,0 }).getHandle();
	//GHANDLE vRightThingy4 = Vertex(s2, vs, { 0.1,0.0356,0 }).getHandle();
	//GHANDLE vRightThingyCenter_1 = Vertex(s2, vs, { -0.1008,0.036,0 }).getHandle();
	//GHANDLE vRightThingyCenter_2 = Vertex(s2, vs, { -0.0991506,0.0349011,0 }).getHandle();


	//GHANDLE lRightThingy1 = EllipticSegment(s2, ls_base, vRightThingy1, vRightThingy2, vProjCenterR).getHandle();
	GHANDLE lRightThingy0 = LineSegment(s2, ls_base, vRightThingy0, vRightThingy1).getHandle();
	GHANDLE lRightThingy1 = LineSegment(s2, ls_base, vRightThingy1, vRightThingy2).getHandle();
	GHANDLE lRightThingy2 = LineSegment(s2, ls_base, vRightThingy2, vRightThingy3).getHandle();
	GHANDLE lRightThingy3 = LineSegment(s2, ls_base, vRightThingy3, vRightThingy0).getHandle();
	//GHANDLE lRightThingy4 = LineSegment(s2, ls_base, vRightThingy4, vRightThingy0).getHandle();

		
	//GHANDLE shape_base = GeometryUtility::CreateContourShape(s, ps, s, { lBase0, lBase1, lBase2, lBase3 });


	GHANDLE shape_base = primitive::Shape(s, ps, s, { lBase0, lBase1, lBase2, lBase3 }).getHandle();
	GHANDLE shape_tank = primitive::Shape(s, ps, s, { lTank0a, lTank0b, lTank1, lTank2, lTank3 }).getHandle();
	GHANDLE shape_form0 = primitive::Shape(s2, ps, s2, { l0, l1, l2, l3, l4, l5, l6, l7, l8, l9, l10, l11, l12, l13, l14, l15, l16, l17, l18, l19 }).getHandle();
	GHANDLE shape_leftTri = primitive::Shape(s2, ps, s2, { lLeftTri0, lLeftTri1, lLeftTri2, lLeftTri3 }).getHandle();
	GHANDLE shape_rightTri = primitive::Shape(s2, ps, s2, { lRightTri0, lRightTri1, lRightTri2, lRightTri3 }).getHandle();
	GHANDLE shape_quad = primitive::Shape(s2, ps, s2, { lQuad0, lQuad1, lQuad2, lQuad3 }).getHandle();
	GHANDLE shape_circle = primitive::Shape(s2, ps, s2, { lCircle0, lCircle1, lCircle2, lCircle3 }).getHandle();
	GHANDLE shape_leftWinding = primitive::Shape(s2, ps, s2, { lLeftWinding0, lLeftWinding1, lLeftWinding2, lLeftWinding3 }).getHandle();
	GHANDLE shape_rightWinding = primitive::Shape(s2, ps, s2, { lRightWinding0, lRightWinding1, lRightWinding2, lRightWinding3 }).getHandle();
	GHANDLE shape_proj = primitive::Shape(s2, ps, s2, { lProj0, lProj1, lProj2, lProj3, lProj4, lProj5, lProj6, lProj7, lProj8 }).getHandle();
	GHANDLE shape_leftThingy = primitive::Shape(s2, ps, s2, { lLeftThingy0, lLeftThingy1, lLeftThingy3, lLeftThingy4 }).getHandle();
	GHANDLE shape_rightThingy = primitive::Shape(s2, ps, s2, { lRightThingy0, lRightThingy1, lRightThingy2, lRightThingy3 }).getHandle();

	auto& sh_base = s.get<primitive::Shape>(shape_base);
	RectView rect_base{ sh_base, vBase0, vBase1, vBase3, vBase2 };

	auto& sh_tank = s.get<primitive::Shape>(shape_tank);
	RectView rect_tank{ sh_tank, vTank0, vTank1, vTank3, vTank2 };

	auto& sh_form0 = s2.get<primitive::Shape>(shape_form0);
	RectView rect_form0{ sh_form0, v1, v0, v13, v14 };

	auto& sh_leftTri = s2.get<primitive::Shape>(shape_leftTri);
	RectView rect_leftTri{ sh_leftTri, vLeftTri0, vLeftTri1, vLeftTri3, vLeftTri2 };

	auto& sh_rightTri = s2.get<primitive::Shape>(shape_rightTri);
	RectView rect_rightTri{ sh_rightTri, vRightTri0, vRightTri1, vRightTri3, vRightTri2 };

	auto& sh_quad = s2.get<primitive::Shape>(shape_quad);
	RectView rect_quad{ sh_quad, vQuad0, vQuad1, vQuad3, vQuad2 };

	auto& sh_circle = s2.get<primitive::Shape>(shape_circle);
	RectView rect_circle{ sh_circle, vCircle0, vCircle1, vCircle3, vCircle2 };

	auto& sh_leftWinding = s2.get<primitive::Shape>(shape_leftWinding);
	RectView rect_leftWinding{ sh_leftWinding, vLeftWinding0, vLeftWinding1, vLeftWinding3, vLeftWinding2 };

	auto& sh_rightWinding = s2.get<primitive::Shape>(shape_rightWinding);
	RectView rect_rightWinding{ sh_rightWinding, vRightWinding0, vRightWinding1, vRightWinding3, vRightWinding2 };

	auto& sh_proj = s2.get<primitive::Shape>(shape_proj);
	RectView rect_proj{ sh_proj, vProj0, vProj1, vProj6, vProj4 };

	auto& sh_leftThingy = s2.get<primitive::Shape>(shape_leftThingy);
	RectView rect_leftThingy{ sh_leftThingy, vLeftThingy0, vLeftThingy1, vLeftThingy4, vLeftThingy3 };

	auto& sh_rightThingy = s2.get<primitive::Shape>(shape_rightThingy);
	RectView rect_rightThingy{ sh_rightThingy, vRightThingy0, vRightThingy1, vRightThingy3, vRightThingy2 };

	std::shared_ptr<RectMeshView> mesh_base{ std::make_shared<RectMeshView>(rect_base) };
	std::shared_ptr<RectMeshView> mesh_tank{ std::make_shared<RectMeshView>(rect_tank) };
	std::shared_ptr<RectMeshView> mesh_form0{ std::make_shared<RectMeshView>(rect_form0) };
	std::shared_ptr<RectMeshView> mesh_leftTri{ std::make_shared<RectMeshView>(rect_leftTri) };
	std::shared_ptr<RectMeshView> mesh_rightTri{ std::make_shared<RectMeshView>(rect_rightTri) };
	std::shared_ptr<RectMeshView> mesh_quad{ std::make_shared<RectMeshView>(rect_quad) };
	std::shared_ptr<RectMeshView> mesh_circle{ std::make_shared<RectMeshView>(rect_circle) };
	std::shared_ptr<RectMeshView> mesh_leftWinding{ std::make_shared<RectMeshView>(rect_leftWinding) };
	std::shared_ptr<RectMeshView> mesh_rightWinding{ std::make_shared<RectMeshView>(rect_rightWinding) };
	std::shared_ptr<RectMeshView> mesh_proj{ std::make_shared<RectMeshView>(rect_proj) };
	std::shared_ptr<RectMeshView> mesh_leftThingy{ std::make_shared<RectMeshView>(rect_leftThingy) };
	std::shared_ptr<RectMeshView> mesh_rightThingy{ std::make_shared<RectMeshView>(rect_rightThingy) };


	//globalMeshDrawer.draw(*mesh_tank);
	//globalMeshDrawer.init();
	
	MeshCombiner combiner{ RectMeshView(rect_base) };
	//combiner.SetCriterion<OnePointCriterion>();
	//
	//combiner.AddMesh(mesh_form0);
	//MeshElementSizeIsoMaxEdgeLength size(mesh_form0->mesh());
	//size_t mode = 1;
	//onSpacePress = ([&]()->void {
	//	if (combiner.AdjustIteration(size, mode) == 0)
	//		std::cout << "Mesh optimization complete" << std::endl;
	//});


	combiner.SetCriterion<OnePointCriterion>(0.00000001);
	
	mesh_tank->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_form0->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_leftTri->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_rightTri->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_quad->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_circle->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_leftWinding->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_rightWinding->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_proj->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_leftThingy->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	mesh_rightThingy->setIsoSize<MeshElementSizeIsoMaxEdgeLength>();
	
	std::vector<std::pair<std::shared_ptr<ElementGeometry>, CSGOperation>> els = {
		//std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_tank), CSGOperation::Union),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_form0), CSGOperation::Union),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_proj), CSGOperation::Union),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_leftTri), CSGOperation::Subtract),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_rightTri), CSGOperation::Subtract),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_circle), CSGOperation::Subtract),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_quad), CSGOperation::Subtract)
	};
	
	std::shared_ptr<CSG> csg0 = std::make_shared<CSG>(ps, els);
	std::vector<std::pair<std::shared_ptr<ElementGeometry>, CSGOperation>> els1 = {
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_rightThingy), CSGOperation::Union),
		std::make_pair(std::dynamic_pointer_cast<ElementGeometry>(mesh_leftThingy), CSGOperation::Union)
	};
	std::shared_ptr<CSG> csg1 = std::make_shared<CSG>(ps_shim, els1);
	
	std::vector<std::tuple<std::shared_ptr<ElementGeometry>, CSGOperation, SETTINGHANDLE>> els2 = {
		std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(csg0), CSGOperation::Union, ps),
		//std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(mesh_circle), CSGOperation::Subtract, ps_air_hole),
		std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(csg1), CSGOperation::Union, ps_shim),
		std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(mesh_leftWinding), CSGOperation::Union, ps_coil_lft),
		std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(mesh_rightWinding), CSGOperation::Union, ps_coil_rgt),
		std::make_tuple(std::dynamic_pointer_cast<ElementGeometry>(mesh_tank), CSGOperation::Union, ps_air)
	};
	std::shared_ptr<CSG> csg = std::make_shared<CSG>(els2);


	csg->setIsoSize<LambdaElementSize<double>>([](const vector3&) {return 0.5f; });

	combiner.AddMesh(csg);
	bool use_runtime_mesh_building = true;
	if (!use_runtime_mesh_building) {
		combiner.AdjustMesh((IElementSize<double>&)*csg);
		onSpacePress = ([&]()->void {});
	}
	else {
		combiner.AdjustMeshInitialization();
		size_t mode = 1;
		onSpacePress = ([&]()->void {
			if (combiner.AdjustIteration((IElementSize<double>&)*csg, mode) == 0)
				std::cout << "Mesh optimization complete" << std::endl;
		});
	}
	onWireframeToggle = [&]() -> void{ globalMeshDrawer.toggleWireframe(); };
	onExportPressed = [&]() -> void { combiner.export_msh("test.msh", *csg.get()); };
	onGeometryAdd = [&]() -> void { combiner.AddGeometry(); };
	//  mesh_base
	//  mesh_tank
	//  mesh_form0
	//  mesh_leftTri
	//  mesh_rightTri
	//  mesh_quad
	//  mesh_circle
	//  mesh_leftWinding
	//  mesh_rightWinding
	//  mesh_proj
	//  mesh_leftThingy
	//  mesh_rightThingy
	const bool draw_meshes = false;
	if (draw_meshes) {
		globalMeshDrawer.draw(*mesh_base);
		globalMeshDrawer.draw(*mesh_tank);
		globalMeshDrawer.draw(*mesh_form0);
		globalMeshDrawer.draw(*mesh_leftTri);
		globalMeshDrawer.draw(*mesh_rightTri);
		globalMeshDrawer.draw(*mesh_quad);
		globalMeshDrawer.draw(*mesh_circle);
		globalMeshDrawer.draw(*mesh_leftWinding);
		globalMeshDrawer.draw(*mesh_rightWinding);
		globalMeshDrawer.draw(*mesh_proj);
		globalMeshDrawer.draw(*mesh_leftThingy);
		globalMeshDrawer.draw(*mesh_rightThingy);
	}
	else {
		globalMeshDrawer.draw(combiner, csg.get());
	}
	globalMeshDrawer.init();
#endif

#ifdef _testBSP
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	ls_1->setParameter("N", DoubleParameter(1));
	ls_1->setParameter("q", DoubleParameter(1));

	SETTINGHANDLE ls_2 = std::make_shared<LineSetting>(LineSetting());
	ls_2->setParameter("N", DoubleParameter(16));
	ls_2->setParameter("q", DoubleParameter(1.));
	SETTINGHANDLE ls_3 = std::make_shared<LineSetting>(LineSetting());
	ls_3->setParameter("N", DoubleParameter(16));
	ls_3->setParameter("q", DoubleParameter(1 / 1.));
	SETTINGHANDLE ls_4 = std::make_shared<LineSetting>(LineSetting());
	ls_4->setParameter("N", DoubleParameter(16));
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
	//globalMeshDrawer.draw(*mesh_form0);
	//globalMeshDrawer.draw(*rsh);

	globalMeshDrawer.init();

#endif
#ifdef _test
	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	ls_1->setParameter("N", DoubleParameter(5.0));
	ls_1->setParameter("q", DoubleParameter(1.0));
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

	GHANDLE v10 = Vertex(s, vs, { -5,0.2 ,0 }).getHandle();
	GHANDLE v11 = Vertex(s, vs, { 2,0.2 ,0 }).getHandle();
	GHANDLE v12 = Vertex(s, vs, { 2,0.4,0 }).getHandle();
	GHANDLE v13 = Vertex(s, vs, { -5,0.4,0 }).getHandle();

	GHANDLE l10 = LineSegment(s, ls_1, v10, v11).getHandle();
	GHANDLE l11 = LineSegment(s, ls_1, v11, v12).getHandle();
	GHANDLE l12 = LineSegment(s, ls_1, v12, v13).getHandle();
	GHANDLE l13 = LineSegment(s, ls_1, v13, v10).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l10, l11, l12, l13 }).getHandle();
	auto& sh_base = s.get<primitive::Shape>(shape_base);

	auto& rsh = main_scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(main_scene, CSGOperation::Subtract, sh_form0, sh_base));

	globalCSGDrawer.draw(&rsh);
	globalCSGDrawer.init();

#endif

#ifdef _BSP_cube
	SETTINGHANDLE ps = std::make_shared<GeometrySetting>(GeometrySetting());
	SETTINGHANDLE ps2 = std::make_shared<GeometrySetting>(GeometrySetting());
	SETTINGHANDLE ls_1 = std::make_shared<LineSetting>(LineSetting());
	SETTINGHANDLE csgs = std::make_shared<GeometrySetting>(GeometrySetting());
	GHANDLE v0 = Vertex(s, vs, { -3,0,0 }).getHandle();
	GHANDLE v1 = Vertex(s, vs, { 0,-3,0 }).getHandle();
	GHANDLE v2 = Vertex(s, vs, { 3,0,0 }).getHandle();
	GHANDLE v3 = Vertex(s, vs, { 0,3,0 }).getHandle();
	GHANDLE c1 = Vertex(s, vs, { 0,0,0 }).getHandle();

	GHANDLE l0 = EllipticSegment(s, ls_1, v0, v1, c1).getHandle();
	GHANDLE l1 = EllipticSegment(s, ls_1, v1, v2, c1).getHandle();
	GHANDLE l2 = EllipticSegment(s, ls_1, v2, v3, c1).getHandle();
	GHANDLE l3 = EllipticSegment(s, ls_1, v3, v0, c1).getHandle();

	GHANDLE shape_form0 = primitive::Shape(s, ps, s, { l0, l1, l2, l3 }).getHandle();
	auto& sh_form0 = s.get<primitive::Shape>(shape_form0);

	//std::cout << sh_form0.classify(vector3{});

	GHANDLE v10 = Vertex(s, vs, { 1,-2. ,0 }).getHandle();
	GHANDLE v11 = Vertex(s, vs, { 4,-5. ,0 }).getHandle();
	GHANDLE v12 = Vertex(s, vs, { 7,-2,0 }).getHandle();
	GHANDLE v13 = Vertex(s, vs, { 4,1,0 }).getHandle();
	GHANDLE c2 = Vertex(s, vs, { 4,-2,0 }).getHandle();


	GHANDLE l10 = EllipticSegment(s, ls_1, v10, v11, c2).getHandle();
	GHANDLE l11 = EllipticSegment(s, ls_1, v11, v12, c2).getHandle();
	GHANDLE l12 = EllipticSegment(s, ls_1, v12, v13, c2).getHandle();
	GHANDLE l13 = EllipticSegment(s, ls_1, v13, v10, c2).getHandle();

	GHANDLE shape_base = primitive::Shape(s, ps, s, { l10, l11, l12, l13 }).getHandle();
	auto& sh_base = s.get<primitive::Shape>(shape_base);

	GHANDLE vin0 = Vertex(main_scene, vs, { 1,1,0 }).getHandle();
	GHANDLE vin1 = Vertex(main_scene, vs, { 2,-1,0 }).getHandle();
	GHANDLE lin0 = LineSegment(main_scene, ls_1, vin0, vin1).getHandle();

	std::vector<GHANDLE> sin{ lin0 };

	auto& rsh = main_scene.get<primitive::Shape>(GeometryUtility::ApplyCSG(main_scene, CSGOperation::Subtract, sh_base, sh_form0, &sin));

	//auto spc = rsh.classify(vector3(-3.0, -0.01));

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


