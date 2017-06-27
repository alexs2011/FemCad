#pragma once

#include "stdafx.h"

void reshape(int w, int h);
void processSpecialKeys(int key, int xx, int yy);
void processUsualKeys(unsigned char key, int xx, int yy);

int window_id;
double xChange = 0.0, yChange = 0.0, scale = 1.0;
bool started = false;

std::function<void()> onSpacePress;
std::function<void()> onWireframeToggle;
std::function<void()> onExportPressed;
std::function<void()> onGeometryAdd;
std::function<void()> onForciblyFlip; 
std::function<void()> onForciblyCollapse;
class Drawer
{
private:
	int window_x;
	int window_y;

	int window_width = 480;
	int window_height = 480;

public:
	static const double viewWidth, viewHeight;

	//std::function<void()> onSpacePress;
public:
	Drawer() = default;
	~Drawer() = default;

public:
	void init(void(*display) (void))
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

// вычисляет границы предметной области, на которую попадает камера
void calcBoundaries(int w, int h, double xChange, double yChange, double scale,
	double& xMin, double& xMax, double& yMin, double& yMax)
{
	//xChange = 0.0;
	//yChange = 0.0;
	//scale = 1.0;
	double ww = w, hh = h;
	xChange /= scale;
	yChange /= scale;
	double viewWidth = Drawer::viewWidth / scale;
	double viewHeight = Drawer::viewHeight / scale;
	if (w > h)
	{
		xMin = -viewWidth * (ww / hh) + xChange;
		xMax = viewWidth * (ww / hh) + xChange;
		yMin = -viewHeight + yChange;
		yMax = viewHeight + yChange;
	}
	else
	{
		xMin = -viewWidth + xChange;
		xMax = viewWidth + xChange;
		yMin = -viewHeight * (hh / ww) + yChange;
		yMax = viewHeight * (hh / ww) + yChange;
	}
	//std::cout << "xChange " << xChange << " yChange " << yChange << " scale " << scale << std::endl;
}

void set_perspective()
{
	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double xMin, xMax, yMin, yMax;
	calcBoundaries(w, h, xChange, yChange, scale, xMin, xMax, yMin, yMax);
	//glOrtho(-w/200, w/200, -h/200, h/200, -2.0, 2.0);
	glOrtho(xMin, xMax, yMin, yMax, -2.0, 2.0);
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
	set_perspective();
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
	case 32: // space key
		onSpacePress();
		break;
	case 'y':
		started = true;
		break;
	case 'u':
		started = false;
		break;
	case 'w':
		onWireframeToggle();
		break;
	case 'e':
		onExportPressed();
		break;
	case 'g':
		onGeometryAdd();
		break;
	case 'f':
		onForciblyFlip();
		break;
	case 'c':
		onForciblyCollapse();
		break;
	}
	set_perspective();
	//glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	set_perspective();
}