// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../femcadgeom.h"
#ifdef _DEBUG
#pragma comment(lib, "../Debug/FemCadMS.lib")
#else
#pragma comment(lib, "../Release/FemCadMS.lib")
#endif
int main() {
	FemCadGeomTester s;
	s.Launch();
	return 0;
}