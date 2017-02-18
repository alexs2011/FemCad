#pragma once
#include "stdafx.h"
#include "g_meshing.h"
#include "g_meshview.h"

void display(void);
void reshape(int w, int h);
void processSpecialKeys(int key, int xx, int yy);
void processUsualKeys(unsigned char key, int xx, int yy);

class MeshDrawer
{
private:
	std::vector<const fg::IMeshView *> mesh_views;

	int window_x;
	int window_y;

	int window_width = 480;
	int window_height = 480;

public:
	MeshDrawer() = default;
	void init() {
		int argc = 0;
		char **argv = NULL;
		glutInit(&argc, argv);

		window_x = (glutGet(GLUT_SCREEN_WIDTH) - window_width) / 2;
		window_y = (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2;
		glutInitWindowSize(window_width, window_height);
		glutInitWindowPosition(window_x, window_y);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
		glutCreateWindow("FemCad");
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutIdleFunc(display);
		glutSpecialFunc(processSpecialKeys);
		glutKeyboardFunc(processUsualKeys);
		glutMainLoop();
	}

	~MeshDrawer() = default;

	/*void draw(const fg::Mesh2 *mesh)
	{
		this->mesh = mesh;
		int argc = 0;
		char **argv = NULL;
		glutInit(&argc, argv);

		window_x = (glutGet(GLUT_SCREEN_WIDTH) - window_width) / 2;
		window_y = (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2;
		glutInitWindowSize(window_width, window_height);
		glutInitWindowPosition(window_x, window_y);
		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
		glutCreateWindow("FemCad");
		glClearColor(1.0, 1.0, 1.0, 0.0);
		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutIdleFunc(display);
		glutSpecialFunc(processSpecialKeys);
		glutKeyboardFunc(processUsualKeys);
		glutMainLoop();
	}*/
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

static MeshDrawer globalMeshDrawer;
double xChange = 0.0, yChange = 0.0, scale = 1.0;

void processSpecialKeys(int key, int xx, int yy)
{
	double fraction = 0.1;
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
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double  ww = w, hh = h;

	double viewWidth = 2.0, viewHeight = 2.0;
	if (w > h)
		glOrtho(-viewWidth * (ww / hh), viewWidth * (ww / hh), -viewHeight, viewHeight, -2.0, 2.0);
		//gluOrtho2D(-viewWidth * (ww / hh), viewWidth * (ww / hh), -viewHeight, viewHeight);
	else
		glOrtho(-viewWidth, viewWidth, -viewHeight * (hh / ww), viewHeight * (hh / ww), -2.0, 2.0);
		//gluOrtho2D(-viewWidth, viewWidth, -viewHeight * (hh / ww), viewHeight * (hh / ww));
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//if (h == 0)
	//	h = 1;
	//double ratio = 1.0 * w / h;
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glViewport(0, 0, w, h);
	//gluPerspective(45, ratio, 1, 1000);
	//glMatrixMode(GL_MODELVIEW);
}

void display(void)
{
	Sleep(57);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	auto& meshes = globalMeshDrawer.getMeshViews();

	// enable wire mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);

	glLoadIdentity();
	glPushMatrix();
	glScaled(scale, scale, 0.0);

	glBegin(GL_LINES);
	glColor3d(0.0, 0.0, 0.0);

	double xMax = 0.0, xMin = 0.0, yMax = 0.0, yMin = 0.0;
	/*for each(auto point_id in mesh->)
	{
		auto x = point_id[0];
		auto y = point_id[1];
		if (x > xMax) xMax = x;
		if (x < xMin) xMin = x;
		if (y > yMax) yMax = y;
		if (y < yMin) yMin = y;
	}*/

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
				glVertex3d((x + xChange) * scale, (y + yChange) * scale, z);
				if (x > xMax) xMax = x;
				if (x < xMin) xMin = x;
				if (y > yMax) yMax = y;
				if (y < yMin) yMin = y;
			};
			auto p = _mesh.getCoordsByPointIdx(e.first);
			draw_p(p.x, p.y, p.z);
			p = _mesh.getCoordsByPointIdx(e.second);
			draw_p(p.x, p.y, p.z);
		}
		//for (size_t i = 0; i < mesh.TrianglesLength(); i++)
		//{
		//	auto point_ids = mesh.triangleVertices(i);
		//	fg::vector3 coords[3];
		//	coords[0] = mesh.getCoordsByPointIdx(std::get<0>(point_ids));
		//	coords[1] = mesh.getCoordsByPointIdx(std::get<1>(point_ids));
		//	coords[2] = mesh.getCoordsByPointIdx(std::get<2>(point_ids));
		//	for (int j = 0; j < 3; j++)
		//	{
		//		auto x = coords[j][0];
		//		auto y = coords[j][1];
		//		auto z = coords[j][2];
		//		glVertex3d((x + xChange) * scale, (y + yChange) * scale, z);
		//
		//		if (x > xMax) xMax = x;
		//		if (x < xMin) xMin = x;
		//		if (y > yMax) yMax = y;
		//		if (y < yMin) yMin = y;
		//	}
		//}
	}
	glPopMatrix();

	glEnd();

	glPushMatrix();
	//glLoadIdentity();
	glLineWidth(1);
	//glBegin(GL_LINES);

	glColor3f(0.3f, 0.3f, 0.3f);
	//for (int i = xMin - 6; i < xMax + 6; i++)
	//{
	//	glVertex3d((i + xChange) * scale, (yMin - 6 + yChange) * scale, -1.0);
	//	glVertex3d((i + xChange) * scale, (yMax + 6 + yChange) * scale, -1.0);
	//}
	//
	//for (int i = yMin - 6; i < yMax + 6; i++)
	//{
	//	glVertex3d((xMin - 6 + xChange) * scale, (i + yChange) * scale, -1.0);
	//	glVertex3d((xMax + 6 + xChange) * scale, (i + yChange) * scale, -1.0);
	//}

	//glEnd();


	//glPopMatrix();

	//glPushMatrix();
	//glLoadIdentity();
	glLineWidth(2);
	glBegin(GL_LINES);

	glColor3d(0.0, 0.0, 0.0);
	glVertex3d((xMin - 6 + xChange) * scale, (0.0 + yChange) * scale, 1.0);
	glVertex3d((xMax + 6 + xChange) * scale, (0.0 + yChange) * scale, 1.0);

	glVertex3d((0.0 + xChange) * scale, (yMin - 6 + yChange) * scale, 1.0);
	glVertex3d((0.0 + xChange) * scale, (yMax + 6 + yChange) * scale, 1.0);

	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}
