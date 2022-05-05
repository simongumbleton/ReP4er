#pragma once
#ifdef _WIN32
#include <windows.h>
#else
#include "../../WDL/swell/swell.h"
#endif
#include <Commctrl.h>
#include <string>
#include "stdio.h"
#include "stdlib.h"

#include <iostream> 
#include <sstream>

#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"

#define REAPERAPI_DECL
REAPER_PLUGIN_HINSTANCE g_hInst;

gaccel_register_t action01 = { { 0, 0, 0 }, "Do action 01." };

extern "C"
{
REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
	g_hInst = hInstance;
	if (rec)
	{
		rec->Register;
		HWND main = rec->hwnd_main;
		MessageBox(main, "Hello World","Reaper Basic Extension", MB_OK);

	}
	return 1;
}
}