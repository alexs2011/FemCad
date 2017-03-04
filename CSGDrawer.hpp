#pragma once

#include "stdafx.h"
#include "g_geometry.h"
#include "Drawer.hpp"

void display_CSG(void);

class CSGDrawer : public Drawer
{
public:
	fg::IClassifiable * csg;

	CSGDrawer() = default;
	void init() {
		this->Drawer::init(display_CSG);
	}
	~CSGDrawer() = default;

	void draw(fg::IClassifiable * csg)
	{
		this->csg = csg;
	}
};

static CSGDrawer globalCSGDrawer;

void display_CSG(void)
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);

	glLoadIdentity();
	glPushMatrix();
	glScaled(scale, scale, 0.0);
	glTranslatef(-xChange, -yChange, 0.0f);
	glColor3d(0.0, 0.0, 0.0);

	// определим размеры сцены (т.е. какой прмоугльоник сцены захватывает наша камера)
	int w = glutGet(GLUT_SCREEN_WIDTH);
	int h = glutGet(GLUT_SCREEN_HEIGHT);
	double xMax, xMin, yMax, yMin, x, y;
	calcBoundaries(w, h, xChange, yChange, scale, xMin, xMax, yMin, yMax);
	for (size_t i = 0; i < w; i++) {
		for (size_t j = 0; j < h; j++) {
			x = i * (xMax - xMin) / w + xMin;
			y = j * (yMax - yMin) / h + yMin;
			if (globalCSGDrawer.csg->classify({ x, y, 0. }) != 1) {
				glPointSize(7.0);
				glColor3f(1., 0., 0.);
				glBegin(GL_POINTS);
				glVertex3f(x, y, 0.);
				glEnd();
			}
		}
	}
	//glDrawPixels(w, h, GL_RGB, GL_FLOAT)

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
