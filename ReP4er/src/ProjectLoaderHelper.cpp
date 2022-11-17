
#pragma once

//#define REAPERAPI_IMPLEMENT
#include "ProjectLoaderHelper.h"
#include "reaper_plugin_functions.h"
#include "reaperHelpers.h"
#include "p4v_helper.h"
#include "platformhelpers.h"
#include <cstdio>

#define VERSION_STRING "1.0.0"

#define SECTION_ID "reP4er"
#define KEY_ID "onprojectload"
#define KEY_FM_ID "onprojectload_frontmost"

static int infoCommandId = 0;
static int setCommandId = 0;
static int showCommandId = 0;
static int clearCommandId = 0;
static int frontmostToggleCommandId = 0;

static int actionToRun = 0;
static bool runOnFrontmostChange = false;

static ReaProject* frontmostProject = nullptr;
std::string currentProjName = "";

std::unordered_map<std::string, ReaProject*> checkedOutProjects;


static void handleFrontmostTimer();


static void runAction();
static void activeProjectCheck();

bool allowPerProjectCheckoutOnChange = true;


int ProjectLoaderHelper_Start()
{
    plugin_register("timer", (void*)activeProjectCheck);

//    const char* wantsFrontmost = GetExtState(SECTION_ID, KEY_FM_ID);
//    runOnFrontmostChange = true;// (!strcmp(wantsFrontmost, "1")) ? true : false;
//    handleFrontmostTimer();

    return 1;
}


void activeProjectCheck()
{
    if (allowPerProjectCheckoutOnChange)
    {
        std::string newCurrentProj = GetCurrentReaperProject();
        if (newCurrentProj.empty())
        {
            return;
        }
        if (!PLATFORMHELPERS::ends_with(newCurrentProj, ".rpp"))
        {
            return;
        }
        ReaProject* newCurrentReaProj = GetCurrentReaProject();
        if (newCurrentProj != currentProjName) {
            currentProjName = newCurrentProj;
            if (!currentProjName.empty())
            {
                if (checkedOutProjects.find(currentProjName) == checkedOutProjects.end())
                {
                    CheckoutCurrentProject();
                    checkedOutProjects[currentProjName] = newCurrentReaProj;
                }
            }
        }
    }
}

void handleFrontmostTimer()
{
    if (runOnFrontmostChange) {
        plugin_register("timer", (void*)activeProjectCheck);
    }
    else {
        plugin_register("-timer", (void*)activeProjectCheck);
    }
}

void runAction()
{
    PrintToConsole("Project Changed....");
    PrintToConsole(currentProjName);
    if (!currentProjName.empty())
    {
        CheckoutCurrentProject();
    }
}


void ClearProjectCheckoutMap()
{
    checkedOutProjects.clear();
}

