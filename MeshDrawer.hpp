#pragma once
#include "stdafx.h"
#include "g_meshing.h"
#include "g_meshview.h"
#include "Drawer.hpp"
#include "fg_math.h"

//extern double xChange, yChange, scale;

void display(void);

class MeshDrawer : public Drawer
{
private:
	std::vector<const fg::IMeshGeometryView *> mesh_views;
	std::vector<const fg::IGeometryView *> mesh_material_guides;
	std::vector<fg::vector3> colors;
public:
	bool draw_wires = false;

	MeshDrawer() = default;
	void init() {
		this->Drawer::init(display);
	}

	~MeshDrawer() = default;

	int draw(const fg::IMeshGeometryView& _mesh, const fg::IGeometryView* _material_guide = nullptr, fg::vector3 color = fg::vector3{})
	{
		mesh_views.push_back(&_mesh);
		mesh_material_guides.push_back(_material_guide);
		colors.push_back(color);
		return mesh_views.size() - 1;
	}
	const std::vector<const fg::IMeshGeometryView*>& getMeshViews() const
	{
		return mesh_views;
	}
	void toggleWireframe() { draw_wires = !draw_wires; }
	const std::vector<const fg::IGeometryView*>& getMeshGuides() const
	{
		return mesh_material_guides;
	}
	
	const std::vector<fg::vector3>& getColors() const
	{
		return colors;
	}
};

// √лобальный экзепл¤р класса
static MeshDrawer globalMeshDrawer;

void display(void)
{
	//Sleep(100);
	if (started) {
		onSpacePress();
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& meshes = globalMeshDrawer.getMeshViews();
	auto& guides = globalMeshDrawer.getMeshGuides();
	auto& colors = globalMeshDrawer.getColors();

	// enable wire mode

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glScaled(scale, scale, 0.0);
	glTranslatef(-xChange, -yChange, 0.0f);
	glColor3d(0.0, 0.0, 0.0);
	//double xMax = 0.0, xMin = 0.0, yMax = 0.0, yMin = 0.0;
	//for (auto meshview : meshes) {
	std::vector<fg::vector3> poly_colors{ fg::vector3(0.2, 0.6,0.1),
		fg::vector3(0.1, 0.1, 0.1), // iron
		fg::vector3(0.6, 0.6, 0.6),
		fg::vector3(0.4, 0.4, 0.4), //iron?
		fg::vector3(0.8, 0.8, 0.2),
		fg::vector3(0.9, 0.9, 0.9), // air
		fg::vector3(0.1, 0.4, 0.2),
		fg::vector3(0.6, 0.6, 0.6), // ne left coil
		fg::vector3(0.6, 0.6, 0.6), // right coil
		fg::vector3(0.6, 0.6, 0.6) };
	for (size_t i{}; i<meshes.size(); ++i){
		auto& meshview = meshes[i];
		auto guide = guides[i] ? guides[i] : meshes[i];
		const auto& _mesh = meshview->mesh();

		if (!globalMeshDrawer.draw_wires) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			for (size_t i{}; i < _mesh.TrianglesLength(); ++i) {
				glBegin(GL_TRIANGLES);
				auto vrts = _mesh.triangleVertices(i);
				auto color = guide->materialId(_mesh.sample_triangle(i, fg::vector3{ .33333,.33333,.33333 }));
				glColor3dv(poly_colors[color % poly_colors.size()].data);
				glVertex3dv(_mesh.point(std::get<0>(vrts)).data);
				glVertex3dv(_mesh.point(std::get<1>(vrts)).data);
				glVertex3dv(_mesh.point(std::get<2>(vrts)).data);
				glEnd();
			}
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glLineWidth(1);

			for (size_t i = 0; i < _mesh.edgesCount(); ++i)
			{
				glLineWidth(1.0);
				if (meshview->isBoundary(i)) {
					//glLineWidth(7.0);
					glColor3f(1.0f, 0.2f, 0.2f);
				}
				else
					//glColor3dv(colors[i].data);
					glColor3f(0.2f, 0.2f, 0.2f);
				if (meshview->isToBeProcessed(i))
				{
					glLineWidth(10.0);
					glColor3f(0.0f, 0.0f, 1.0f);
				}

				auto e = _mesh.edge(i);
				auto draw_p = [&](double x, double y, double z) {
					glVertex3d(x, y, z);
					/*if (x > xMax) xMax = x;
					if (x < xMin) xMin = x;
					if (y > yMax) yMax = y;
					if (y < yMin) yMin = y;*/
				};
				// рисуем ребро
				glBegin(GL_LINES);
				auto p = _mesh.getCoordsByPointIdx(e.first);
				draw_p(p.x, p.y, p.z);
				p = _mesh.getCoordsByPointIdx(e.second);
				draw_p(p.x, p.y, p.z);
				glEnd();
				// рисуем вершины ребра
				//glPointSize(7.0);
				//glColor3f(0.2f, 0.2f, 0.2f);
				//glBegin(GL_POINTS);
				//p = _mesh.getCoordsByPointIdx(e.first);
				//glVertex3d(p.x, p.y, p.z-1.0);
				//p = _mesh.getCoordsByPointIdx(e.second);
				//glVertex3d(p.x, p.y, p.z - 1.0);
				//glEnd();
			}
		}
	}
	//glPopMatrix();



	// определим размеры сцены (т.е. какой прмоугльоник сцены захватывает наша камера)
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	double xMax, xMin, yMax, yMin, x, y;
	calcBoundaries(w, h, xChange, yChange, scale, xMin, xMax, yMin, yMax);
	// рисуем координатные оси
	glColor3f(0.3f, 0.3f, 0.3f);
	glLineWidth(1);
	//glPushMatrix();
	//glTranslatef(-xChange, -yChange, 0.0f);
	glBegin(GL_LINES);
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(2 * xMin, (0.0), 1.0);
	glVertex3d(2 * xMax, (0.0), 1.0);
	glVertex3d(0.0, 2 * yMin, 1.0);
	glVertex3d(0.0, 2 * yMax, 1.0);
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();
}
