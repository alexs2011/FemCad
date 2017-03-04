#pragma once
#include "stdafx.h"
#include "g_meshing.h"
#include "g_meshview.h"
#include "Drawer.hpp"

//extern double xChange, yChange, scale;

void display(void);

class MeshDrawer : public Drawer
{
private:
	std::vector<const fg::IMeshView *> mesh_views;

public:
	MeshDrawer() = default;
	void init() {
		this->Drawer::init(display);
	}

	~MeshDrawer() = default;

	int draw(const fg::IMeshView& _mesh)
	{
		mesh_views.push_back(&_mesh);
		return mesh_views.size() - 1;
	}
	const std::vector<const fg::IMeshView*>& getMeshViews() const
	{
		return mesh_views;
	}
};

// √лобальный экзепл€р класса
static MeshDrawer globalMeshDrawer;

void display(void)
{	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& meshes = globalMeshDrawer.getMeshViews();

	// enable wire mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);

	glLoadIdentity();
	glPushMatrix();
	glScaled(scale, scale, 0.0);
	glTranslatef(-xChange, -yChange, 0.0f);
	glColor3d(0.0, 0.0, 0.0);
	double xMax = 0.0, xMin = 0.0, yMax = 0.0, yMin = 0.0;
	for (auto meshview : meshes) {
		const auto& _mesh = meshview->mesh();
		for (size_t i = 0; i < _mesh.edgesCount(); ++i) {
			if (meshview->isBoundary(i)) {
				glColor3f(1.0f, 0.2f, 0.2f);
			}
			else {
				glColor3f(0.2f, 0.2f, 0.2f);
			}
			auto e = _mesh.edge(i);
			auto draw_p = [&](double x, double y, double z) {
				glVertex3d(x, y, z);
				if (x > xMax) xMax = x;
				if (x < xMin) xMin = x;
				if (y > yMax) yMax = y;
				if (y < yMin) yMin = y;
			};
			// рисуем ребро
			glBegin(GL_LINES);
			auto p = _mesh.getCoordsByPointIdx(e.first);
			draw_p(p.x, p.y, p.z);
			p = _mesh.getCoordsByPointIdx(e.second);
			draw_p(p.x, p.y, p.z);
			glEnd();
			// рисуем вершины ребра
			glPointSize(7.0);
			glColor3f(0.2f, 0.2f, 0.2f);
			glBegin(GL_POINTS);
			p = _mesh.getCoordsByPointIdx(e.first);
			glVertex3d(p.x, p.y, p.z-1.0);
			p = _mesh.getCoordsByPointIdx(e.second);
			glVertex3d(p.x, p.y, p.z - 1.0);
			glEnd();
		}
	}
	
	// рисуем координатные оси
	glColor3f(0.3f, 0.3f, 0.3f);
	glLineWidth(1);
	glBegin(GL_LINES);
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d((xMin - 6), (0.0), 1.0);
	glVertex3d((xMax + 6), (0.0), 1.0);
	glVertex3d((0.0), (yMin - 6), 1.0);
	glVertex3d((0.0), (yMax + 6), 1.0);
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}
