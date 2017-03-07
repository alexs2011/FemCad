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

void DrawPixels11(int _Width, int _Height, GLenum _Format, GLenum _Type, const void * _Pixels) {
	GLuint texid;

	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // set 1-byte alignment
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _Width, _Height, 0, _Format, _Type, _Pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//glActiveTexture(GL_TEXTURE0);

	const float verts[4][2] =
	{
		{ -1.0f, -1.0f },
		{ 1.0f, -1.0f },
		{ -1.0f,  1.0f },
		{ 1.0f,  1.0f }
	};

	const float texCoord[4][2] =
	{
		{ 0.0f,  0.0f },
		{ 1.0f,  0.0f },
		{ 0.0f,  1.0f },
		{ 1.0f,  1.0f }
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, verts);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 2, texCoord);

	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, _Width, _Height);

	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDeleteTextures(1, &texid);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void display_CSG(void)
{
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);

	std::cout << "w " << w << " h " << h << std::endl;
	// определим размеры сцены (т.е. какой прмоугльоник сцены захватывает наша камера)
	double xMax, xMin, yMax, yMin, x, y;
	calcBoundaries(w, h, xChange, yChange, scale, xMin, xMax, yMin, yMax);
	//glPointSize(1.0);
	auto pixel_data = new GLfloat[w*h*3];
	for (size_t i = 0; i < h; i++) {
		for (size_t j = 0; j < w; j++) {
			x = j * (xMax - xMin) / w + xMin;
			y = i * (yMax - yMin) / h + yMin;
			auto res = globalCSGDrawer.csg->classify({ x, y, 0. });
			if (res != 1) {
				if (res == 0) {
					// граница
					pixel_data[3 * (i*w + j) + 0] = 0.f;
					pixel_data[3 * (i*w + j) + 1] = 0.f;
					pixel_data[3 * (i*w + j) + 2] = 0.f;
				}
				else {
					// внутри
					pixel_data[3 * (i*w + j) + 0] = 0.5f;
					pixel_data[3 * (i*w + j) + 1] = 0.5f;
					pixel_data[3 * (i*w + j) + 2] = 0.5f;
				}
			}
			else {
				pixel_data[3 * (i*w + j) + 0] = 1.f;
				pixel_data[3 * (i*w + j) + 1] = 1.f;
				pixel_data[3 * (i*w + j) + 2] = 1.0f;
			}
		}
	}
	
	std::cout << "xmin " << xMin << " ymin " << yMin << std::endl;
	std::cout << "xMax " << xMax << " yMax " << yMax << std::endl;

	int oldAlign, oldLen;
	glRasterPos2d(xMin, yMin);
	glPixelZoom(1.0f, 1.0f);
	glPixelTransferf(GL_ALPHA_SCALE, 1.0);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldAlign);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldLen);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glDrawPixels(w, h, GL_RGB, GL_FLOAT, pixel_data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, oldAlign);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, oldLen);

	// рисуем координатные оси
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//glPushMatrix();
	//glScaled(scale, scale, 0.0);
	//glTranslatef(-xChange, -yChange, 0.0f);
	
	glColor3d(0.0, 1.0, 0.0);
	glLineWidth(0.01);
	glBegin(GL_LINES);
	for (double x = std::floor(xMin); x < std::ceil(xMax); x++)
	{
		glVertex3d(x, yMin, 1.0);
		glVertex3d(x, yMax, 1.0);
	}
	for (double y = std::floor(yMin); y < std::ceil(yMax); y++)
	{
		glVertex3d(xMin, y, 1.0);
		glVertex3d(xMax, y, 1.0);
	}
	glEnd();
	glLineWidth(3.0);
	glBegin(GL_LINES);
	glVertex3d((xMin), (0.0), 1.0);
	glVertex3d((xMax), (0.0), 1.0);
	glVertex3d((0.0), (yMin), 1.0);
	glVertex3d((0.0), (yMax), 1.0);
	glEnd();

	//glPopMatrix();

	glutSwapBuffers();

	delete[] pixel_data;
}
