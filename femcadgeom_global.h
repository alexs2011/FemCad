#pragma once

#ifdef FEMCADMS_EXPORTS
#define FEMCADGEOMSHARED_EXPORT /*__declspec(dllexport)*/
#else 
#define FEMCADGEOMSHARED_EXPORT /*__declspec(dllimport)*/
#endif
