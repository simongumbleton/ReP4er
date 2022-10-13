
#ifdef _WIN32
#include <windows.h>
#include <Commctrl.h>
#else
#include "WDL/swell/swell.h"
#endif

#include <string>
#include "stdio.h"
#include "stdlib.h"
#include <filesystem>

#include <iostream> 
#include <sstream>
#include <thread>

#define REAPERAPI_IMPLEMENT
#define REAPERAPI_DECL

#include "reaper_plugin.h"
#include "reaper_plugin_functions.h"

#include "GUI.h"

#include "reaperHelpers.h"
#include "platformhelpers.h"
#include "p4v_helper.h"
#include "ProjectLoaderHelper.h"


REAPER_PLUGIN_HINSTANCE g_hInst;


#define GET_FUNC_AND_CHKERROR(x) if (!((*((void **)&(x)) = (void *)rec->GetFunc(#x)))) ++funcerrcnt
#define REGISTER_AND_CHKERROR(variable, name, info) if(!(variable = rec->Register(name, (void*)info))) ++regerrcnt

//define globals
HWND g_parentWindow;
char reaperProjectName[256];
std::string reaperResourcePath;
int WaapiPort = 8095;

char currentProject[256];

std::unique_ptr<juce::DocumentWindow>currentActiveWindow;
bool WindowStatus = false;
juce::DocumentWindow* currentWindow = nullptr;

gaccel_register_t Checkout = { { 0, 0, 0 }, "ReP4er - Checkout all Open Projects" };
gaccel_register_t Submit = { { 0, 0, 0 }, "ReP4er - Save, Reconcile, Submit and Close all Open Projects" };
gaccel_register_t Settings = { { 0, 0, 0 }, "ReP4er - Settings" };


bool HookCommandProc(int command, int flag);
static void menuHook(const char* name, HMENU handle, const int f);
static void AddCustomMenuItems(HMENU parentMenuHandle = NULL);

extern "C"
{
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t* rec)
	{
		g_hInst = hInstance;
		if (rec)
		{
			rec->Register;
			HWND main = rec->hwnd_main;
			//MessageBox(main, "Hello World","Reaper Basic Extension", MB_OK);

			// load all Reaper API functions in one go, byebye ugly IMPAPI macro!
			int error_count = REAPERAPI_LoadAPI(rec->GetFunc);
			if (error_count > 0)
			{
				char errbuf[256];
				sprintf(errbuf, "Failed to load %d expected API function(s)", error_count);
				MessageBox(main, errbuf, "MRP extension error", MB_OK);
				return 0;
			}
			int regerrcnt = 0;
			//Register a custom command ID for an action
			REGISTER_AND_CHKERROR(Checkout.accel.cmd, "command_id", "ReP4er - Checkout all Open Projects");
			REGISTER_AND_CHKERROR(Submit.accel.cmd, "command_id", "ReP4er - Save, Reconcile, Submit and Close all Open Projects");
			REGISTER_AND_CHKERROR(Settings.accel.cmd, "command_id", "ReP4er - Settings");

			//register our custom actions
			plugin_register("gaccel", &Checkout.accel);
			plugin_register("gaccel", &Submit.accel);
			plugin_register("gaccel", &Settings.accel);

			//hook command is where we process command IDs and launch our custom actions
			rec->Register("hookcommand", (void*)HookCommandProc);
			//hook custom menu is where our custom actions are added to the main extensions submenu
			plugin_register("hookcustommenu", reinterpret_cast<void*>(menuHook));
			//Reaper API adds the Extensions main menu - we populate this in the hook custom menu
			AddExtensionsMainMenu();

#ifdef _WIN32
			// Create a custom menu to the main menu and add the available commands
			// only do this on windows, on Mac it seems to not work in the main menu,
			//so on mac we add them to the main extenstions menu only
			//(we add them to the main extensions menu anyway, but on windows its
			//nice to have them in the main menu too
			AddCustomMenuItems();
#endif
			auto CWD = P4V::GetCWD();
			P4V::setP4ConfigName();
			ProjectLoaderHelper_Start();

			// If Reaper exe is in source control then check it out so its writable
			auto reaperExePath = GetReaperResourcePath();
			P4V::SetCWD(reaperExePath);
			if (P4V::checkForP4Config(false) && (P4V::login()))
			{
				auto cl = P4V::createChangelist("Reaper Tool");
				P4V::checkoutDirectory(reaperExePath,"",cl);
			}
			P4V::SetCWD(CWD);
			return 1;
		}
		else // Reaper is being shut down
		{
			auto CWD = P4V::GetCWD();
			// If Reaper exe is in source control then check it out so its writable
			auto reaperExePath = GetReaperResourcePath();
			P4V::SetCWD(reaperExePath);
			if (P4V::checkForP4Config(false) && (P4V::login()))
			{
				auto cl = P4V::createChangelist("Reaper Tool");
				P4V::reconcileDirectory(reaperExePath,"",cl);
				P4V::submitChanges(cl);
			}
			P4V::SetCWD(CWD);
		}

		
		return 1;
	}
}

std::string GetReaperResourcePath()
{
	return GetResourcePath();
}

std::string GetCurrentReaperProject()
{
	char projName[256];
	EnumProjects(-1, currentProject, MAX_PATH);
	GetProjectName(EnumProjects(-1, nullptr, 0), projName, 256);
	return std::string(projName);
}

std::string GetCurrentReaperProjectName()
{
	return PLATFORMHELPERS::filenameFromPathString(GetCurrentReaperProject());
}

ReaProject* GetCurrentReaProject()
{
	return EnumProjects(-1, NULL, NULL);
}

std::string GetCurrentReaperProjectPath()
{
	char projPath[256];
	EnumProjects(-1, projPath, MAX_PATH);
	return std::string(projPath);
}

std::string GetCurrentReaperProjectDirectory()
{
	auto projPath = std::filesystem::path(GetCurrentReaperProjectPath());
	//PrintToConsole((projPath.string()));
	auto dir = projPath.parent_path();
	return dir.string();
}



std::unordered_map<ReaProject*, std::string> GetAllOpenProjects()
{
	std::unordered_map<ReaProject*, std::string> projects;
	ReaProject* proj;
	char projPath[256];
	int i = 0;
	while ((proj = EnumProjects(i++, projPath, 256)))
	{
		projects.emplace(proj, projPath);
	}
	return projects;
}


void SaveProject(ReaProject* inProj)
{
	Main_SaveProject(inProj, false);
}
void SaveProject()
{
	Main_SaveProject(GetCurrentReaProject(), false);
}

void saveProjExState(std::string Key, std::string Value, std::string extName, ReaProject* inProj)
{
	if (inProj == nullptr) { inProj = GetCurrentReaProject(); }
	SetProjExtState(inProj, extName.c_str(), &Key[0], &Value[0]);
	SaveProject(inProj);
}

void deleteProjExtState(std::string Key, std::string extName, ReaProject* inProj)
{
	saveProjExState(Key.c_str(), "", extName.c_str(), inProj);
}

std::string getProjExState(std::string Key, std::string extName, ReaProject* inProj)
{
	//std::string KeyOutValue;
	//char bufferK[256] = "x";
	//char* K;
	//int KeyOutSize = 256;
	//std::string OutValue;
	//char* V;
	//char bufferV[256] = "a";
	//int OutSize = 256;

	//int i = 0;
	//while (EnumProjExtState(GetCurrentReaProject(), "CSGREGIONPROPERTIES", i, bufferK, KeyOutSize, bufferV, OutSize))
	//{
	//	KeyOutValue = std::string(bufferK);
	//	OutValue = std::string(bufferV);
	//	PrintToConsole(KeyOutValue);
	//	PrintToConsole(OutValue);
	//	i++;
	//}
	const int ValOutSize = 25600;/// Need to find a safer way of doing this!
	char bufferValue[ValOutSize] = "";

	//std::string OutValue;
	//int OutSize;
	std::string keyUC = PLATFORMHELPERS::stringToUpper(Key);
	//needs a char* for outvalue
	if (inProj == nullptr) { inProj = GetCurrentReaProject(); }
	GetProjExtState(inProj, extName.c_str(), &keyUC[0], bufferValue, ValOutSize);
	return std::string(bufferValue);
}

void saveGlobalExtState(std::string Key, std::string Value, bool persist, std::string extName)
{
	SetExtState(extName.c_str(), Key.c_str(), Value.c_str(), persist);
}

void deleteGlobalExtState(std::string Key, bool persist, std::string extName)
{
	DeleteExtState(extName.c_str(), Key.c_str(), persist);
}

std::string getGlobalExtState(std::string Key, std::string extName)
{
	std::string value = "";
	std::string keyUC = PLATFORMHELPERS::stringToUpper(Key);
	bool HasState = HasExtState(extName.c_str(), keyUC.c_str());
	if (HasState)
	{
		value = GetExtState(extName.c_str(), keyUC.c_str());
	}
	return value;
}



void Reaper_RenderAllQuedJobs()
{
	Main_OnCommand(41207, 0);
}

void GetReaperGlobals()
{
	//get open project and compare
	EnumProjects(-1, currentProject, MAX_PATH);
	GetProjectName(EnumProjects(-1, nullptr, 0), reaperProjectName, 256);
	reaperResourcePath = GetResourcePath();
}

void PrintToConsole(std::string text)
{
	std::string debugText = text + "\n";
	ShowConsoleMsg(debugText.c_str());
}

int countRegions()
{
	int markerCount;
	int regionCount;
	CountProjectMarkers(GetCurrentReaProject(), &markerCount, &regionCount);
	return regionCount;
}

int countMarkers()
{
	int markerCount;
	int regionCount;
	CountProjectMarkers(GetCurrentReaProject(), &markerCount, &regionCount);
	return markerCount;
}



std::vector<std::string> getNonMasterProjectRegionNames()
{
	int total = countRegions() + countMarkers();
	std::vector<std::string> results;
	int i = 0;
	while (i < total)
	{
		bool isRegion;
		double pos;
		double regEnd;
		//std::string name;
		const char* name;
		int index;
		EnumProjectMarkers(i, &isRegion, &pos, &regEnd, &name, &index);
		if (isRegion)
		{
			std::string namestr = name;
			if (!namestr.empty())
			{
				if (!PLATFORMHELPERS::ends_with(PLATFORMHELPERS::stringToLower(name), "_master"))
				{
					results.push_back(namestr);
				}
			}
		}
		i++;
	}
	return results;
}

void PrintToConsole(int text)
{

	std::string debugText = std::to_string(text) + "\n";
	ShowConsoleMsg(debugText.c_str());

}

void LaunchSettings()
{
	initialiseJuce_GUI();
	MessageManagerLock mml(Thread::getCurrentThread());


	bool useTab = true;
	String wName = "My Reaper Action";

	//int commandID = NamedCommandLookup("_S&M_HIDECCLANES_ME");
	//reaper.Main_OnCommand(commandID, 0)
	//std::string name = ReverseNamedCommandLookup(commandID);
	//int commandID = AddRemoveReaScript(true, 0, reaper.GetResourcePath().."/Scripts/12000.eel", true);

	if (WindowStatus)
	{
		currentActiveWindow->toFront(true);
	}
	else
	{
		currentActiveWindow.reset(new P4GuiWindow(wName, &WindowStatus));
	}
	return;
}

void LaunchCheckout()
{
	auto cwd = P4V::GetCWD();//save the working directoy before we change it
	{
		for (auto ReProj : GetAllOpenProjects())
		{
			auto projPath = std::filesystem::path(ReProj.second);
			if (projPath.empty())
			{
				continue;
			}
			auto projName = PLATFORMHELPERS::filenameFromPathString(projPath.string());
			if (projName.empty())
			{
				continue;
			}
			//PrintToConsole((projPath.string()));
			auto dir = projPath.parent_path();
			//PrintToConsole(dir.string());
			P4V::SetCWD(dir.string());
			if (P4V::checkForP4Config() && (P4V::login()))
			{
				int cl = P4V::createChangelist("ReP4er: " + projName);
				trackedChangeLists.insert({ cl, dir.string() });
				P4V::checkoutDirectory(dir.string(),"", cl);
				P4V::reconcileDirectory(dir.string(), ".rpp", cl);
			}
		}
	}
	P4V::SetCWD(cwd);//restore cwd once we're done
}

void CheckoutCurrentProject()
{
	auto cwd = P4V::GetCWD();//save the working directoy before we change it
	{
		auto projPath = std::filesystem::path(GetCurrentReaperProjectPath());
		if (projPath.empty())
		{
			return;
		}
		auto projName = GetCurrentReaperProjectName();
		if (projName.empty())
		{
			return;
		}
		auto dir = projPath.parent_path();
		P4V::SetCWD(dir.string());
		if (P4V::checkForP4Config() && (P4V::login()))
		{
			int cl = P4V::createChangelist("ReP4er: " + projName);
			trackedChangeLists.insert({ cl, dir.string() });
			P4V::checkoutDirectory(dir.string(),"",cl);
			P4V::reconcileDirectory(dir.string(), ".rpp", cl);
		}
	}
	P4V::SetCWD(cwd);//restore cwd once we're done
}


void LaunchSubmit()
{
	LaunchCheckout();//make sure all open projects are checked out

	allowPerProjectCheckoutOnChange = false;
	//std::unordered_map<int, std::string> CLs;//CLs and directories
	auto cwd = P4V::GetCWD();//save the working directoy before we change it
	for (auto ReProj : GetAllOpenProjects())
	{
		auto projPath = std::filesystem::path(ReProj.second);
		if (projPath.empty())
		{
			continue;
		}
		if (IsProjectDirty(ReProj.first))
		{
			SaveProject(ReProj.first);
		}	
	}
	Main_OnCommand(40886, 0);//File: Close all projects
	auto iterTrackedChangelists = trackedChangeLists; //make a copy for itteration because trackedChangeLists can be modified in create, submit and delete changelist calls
	for (auto CL : iterTrackedChangelists)
	{
		P4V::SetCWD(CL.second);
		//PrintToConsole(dir.string());
			
		if (P4V::checkForP4Config() && (P4V::login()))
		{
			P4V::reconcileDirectory(CL.second,"", CL.first);
			P4V::revertUnchangedFilesInDir(CL.second);
			if (P4V::doesChangelistHaveFiles(CL.first))
			{
				P4V::submitChanges(CL.first);
			}
			else
			{
				P4V::deleteChangelist(CL.first);
			}
		}	
	}
	P4V::SetCWD(cwd);//restore cwd once we're done
	ClearProjectCheckoutMap();
	trackedChangeLists.clear();
	allowPerProjectCheckoutOnChange = true;
}


void bringWindowsToFront()
{
	if (currentWindow && WindowStatus)
	{
		currentWindow->toFront(true);
	}
}



void ClearCurrentWindowPtr()
{
	currentWindow = nullptr;
}

bool HookCommandProc(int command, int flag)
{
	GetReaperGlobals();

	if (command == Checkout.accel.cmd)
	{
		LaunchCheckout();
		return true;
	}
	else if (command == Submit.accel.cmd)
	{
		LaunchSubmit();
	}
	else if (command == Settings.accel.cmd)
	{
		// Disable settings menu item while its WIP
		PrintToConsole("Settings Not Implemented");
		//LaunchSettings();
	}
	return false;
}

static void menuHook(const char* name, HMENU handle, const int f)
{
	if (strcmp(name, "Main extensions") == 0 and f == 0)
	{
		// Create a custom menu and add the available commands
		AddCustomMenuItems(handle);
	}
}

static void AddCustomMenuItems(HMENU parentMenuHandle)
{
	HMENU hMenu = CreatePopupMenu();

	MENUITEMINFO mi = { sizeof(MENUITEMINFO), };
	mi.fMask = MIIM_TYPE | MIIM_ID;
	mi.fType = MFT_STRING;

	// add each command to the popupmenu
	mi.wID = Submit.accel.cmd;
	mi.dwTypeData = (char*)"ReP4er - Save, Reconcile, Submit and Close all Open Projects";
	InsertMenuItem(hMenu, 0, true, &mi);

	mi.wID = Checkout.accel.cmd;
	mi.dwTypeData = (char*)"ReP4er - Checkout all Open Projects";
	InsertMenuItem(hMenu, 0, true, &mi);

// Disable settings menu item while its WIP
//	mi.wID = Settings.accel.cmd;
//	mi.dwTypeData = (char*)"ReP4er - Settings";
//	InsertMenuItem(hMenu, 0, true, &mi);
	

	if (!parentMenuHandle)
	{
		parentMenuHandle = GetMenu(GetMainHwnd());
	}

	mi.fMask = MIIM_SUBMENU | MIIM_TYPE;
	mi.hSubMenu = hMenu;
	mi.dwTypeData = (char*)"ReP4er";
	InsertMenuItem(parentMenuHandle, GetMenuItemCount(parentMenuHandle) - 1, TRUE, &mi);
}
