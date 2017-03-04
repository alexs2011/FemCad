#pragma once

#include "stdafx.h"

void reshape(int w, int h);
void processSpecialKeys(int key, int xx, int yy);
void processUsualKeys(unsigned char key, int xx, int yy);

int window_id;
double xChange = 0.0, yChange = 0.0, scale = 1.0;

class Drawer
{
private:
	int window_x;
	int window_y;

	int window_width = 480;
	int window_height = 480;

public:
	static const double viewWidth, viewHeight;

public:
	Drawer() = default;
	~Drawer() = default;

public:
	void init(void (*display) (void))
	{
		int argc = 0;
		char **argv = NULL;
		glutInit(&argc, argv);

		window_x = (glutGet(GLUT_SCREEN_WIDTH) - window_width) / 2;
		window_y = (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2;
		glutInitWindowSize(window_width, window_height);
		glutInitWindowPosition(window_x, window_y);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
		window_id = glutCreateWindow("FemCad");
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		//glutIdleFunc(display);
		glutSpecialFunc(processSpecialKeys);
		glutKeyboardFunc(processUsualKeys);
		glutMainLoop();
	}
};

const double Drawer::viewWidth = 4.0;
const double Drawer::viewHeight = 4.0;

// ��������� ������� ���������� �������, �� ������� �������� ������
void calcBoundaries(int w, int h, double xChange, double yChange, double scale,
	double& xMin, double& xMax, double& yMin, double& yMax)
{
	xChange = 0.0;
	yChange = 0.0;
	scale = 1.0;
	double ww = w, hh = h;
	double viewWidth = Drawer::viewWidth * scale;
	double viewHeight = Drawer::viewHeight * scale;
	if (w > h)
	{
		xMin = -viewWidth * (ww / hh) - xChange;
		xMax = viewWidth * (ww / hh) - xChange;
		yMin = -viewHeight - yChange;
		yMax = viewHeight - yChange;
	}
	else
	{
		xMin = -viewWidth - xChange;
		xMax = viewWidth - xChange;
		yMin = -viewHeight * (hh / ww) - yChange;
		yMax = viewHeight * (hh / ww) - yChange;
	}
}

void processSpecialKeys(int key, int xx, int yy)
{
	double fraction = 0.1 / scale;
	switch (key) {
	case GLUT_KEY_LEFT:
		xChange -= fraction;
		break;
	case GLUT_KEY_RIGHT:
		xChange += fraction;
		break;
	case GLUT_KEY_UP:
		yChange += fraction;
		break;
	case GLUT_KEY_DOWN:
		yChange -= fraction;
		break;
	}
	glutPostRedisplay();
}

void processUsualKeys(unsigned char key, int xx, int yy)
{
	double scalingFraction = 1.1;
	switch (key) {
	case '=':
		scale *= scalingFraction;
		break;
	case '-':
		scale /= scalingFraction;
		break;
	case 27: // escape key
		glutDestroyWindow(window_id);
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double xMin, xMax, yMin, yMax;
	calcBoundaries(w, h, xChange, yChange, scale, xMin, xMax, yMin, yMax);
	glOrtho(xMin, xMax, yMin, yMax, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}