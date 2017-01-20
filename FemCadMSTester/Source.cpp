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