#include "p4v_helper.h"
#include "platformhelpers.h"

#include <iostream>
#include <filesystem>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <iostream>
#include <iomanip>
#include <ctime>





namespace P4V
{
    //global variable for the current changelist number(so all p4 operations in a pipeline can use the same changelist
    int currentChangelist = 0;
    //global changelist description, used for findingand creating the current used changelist for the pipeline
    std::string defaultdesc = "Automated changelist";
}



std::string P4V::getDateAndTimeNow()
{
    std::stringstream ss;
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    return ss.str();
}

int P4V::execCmd(std::string in_command)
{
    //"D:\\CASoundP4\\CASound\\Show_P4_Info.bat"
    return std::system(in_command.c_str());
}

std::string P4V::execCmd_GetOutput(std::string in_command) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = _popen(in_command.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
            result += buffer;
    }
    catch (...)
    {
        _pclose(pipe);
        throw;
    }
    _pclose(pipe);
    return result;
}


void P4V::setGlobalChangelistDescription(std::string in_description)
{
    using namespace P4V;
    //"""Called from other parts of the pipeline to allign operations into the same changelist. Also updates the global ghangelist number"""
    defaultdesc = in_description;
    currentChangelist = findChangelistByDescription(defaultdesc);
    if (currentChangelist == 0)
    {
        currentChangelist = createChangelist(defaultdesc);
    }
}

void P4V::SetCWD(std::string in_cwd)
{
    namespace fs = std::filesystem;
    const fs::path path = in_cwd;
    fs::current_path(path);
}

std::string P4V::GetCWD()
{
    namespace fs = std::filesystem;
    fs::path oldcwd = fs::current_path();
    return oldcwd.string();
}

void P4V::showP4Info()
{
    //print("Show P4 info")
    //    oldDir, newDir = setCWD()
    execCmd("p4 info");
    //restoreCWD(oldDir)
}

p4Info* P4V::getP4Info()
{
    using namespace P4V;
    auto output = execCmd_GetOutput("p4 info");
    auto lines = PLATFORMHELPERS::stringSplitToList(output, "\n");
    for (auto line : lines)
    {
        auto KeyVal = PLATFORMHELPERS::stringSplitToList(line, ": ");
        if (KeyVal.size() == 2)
        {
            auto key = KeyVal[0];
            auto val = KeyVal[1];
            P4INFO.properties[key] = val;
        }
    }
    return &P4INFO;
}

int P4V::createChangelist(std::string description)
{
    using namespace P4V;
    currentChangelist = findChangelistByDescription(description);
    if (currentChangelist != 0)
    {
        return currentChangelist;
    }
    std::string desc = description + std::string(" - ") + getDateAndTimeNow();
    std::string command1 = R"(p4 --field "Description=)" 
        + desc 
        + R"(" --field "Files=" change -o)"
        + R"( | p4 change -i)";
    std::string result = execCmd_GetOutput(command1);
    auto splitResult = PLATFORMHELPERS::stringSplitToList(result, " ");
    if (splitResult.size() == 3) //Change 2831 created.
    {
        int CL = std::stoi(splitResult[1]);
        return CL;
    }
    return 0;
}

int P4V::findChangelistByDescription(std::string description)
{
    using namespace P4V;
    if (P4INFO.properties.find("User name") == P4INFO.properties.end())
    {
        getP4Info();
        if (P4INFO.properties.find("User name") == P4INFO.properties.end())
        {
            return 0;
        }
    }
    std::string command1 = R"(p4 changes -s pending -u )"
        + P4INFO.properties["User name"];
    if (P4INFO.properties.find("Client name") != P4INFO.properties.end())
    {
        command1 += " -c " + P4INFO.properties["Client name"];
    }
    std::string result = execCmd_GetOutput(command1);
    auto results = PLATFORMHELPERS::stringSplitToList(result, "Change ");
    for (auto &change : results)
    {
        if (change.find(description) != change.npos)
        {
            int CL = std::stoi(PLATFORMHELPERS::stringSplitToList(change, " ")[0]);
            return CL;
        }
    }
    return 0;
}

void P4V::checkoutDirectory(std::string dirPath, std::string extenstion)
{
    using namespace P4V;
}

void P4V::checkoutFiles(std::vector<std::string> fileList)
{
    using namespace P4V;
}

void P4V::reconcileDirectory(std::string dirPath, std::string extenstion)
{
    using namespace P4V;
}

void P4V::reopenDirectory(std::string dirPath, std::string extenstion)
{
    using namespace P4V;
}

void P4V::submitChanges(int changeList, bool deleteIfEmpty)
{
    using namespace P4V;
}

bool P4V::doesChangelistHaveFiles(int changeList)
{
    using namespace P4V;
    return false;
}

void P4V::deleteChangelist(int changeList)
{
    using namespace P4V;
}

void P4V::revertUnchangedFilesInDir(std::string path)
{
    using namespace P4V;
}

void P4V::revertUnchangedFiles(std::vector<std::string> fileList)
{
    using namespace P4V;
}

std::vector<std::string> P4V::getFilesInChangelist(int changeList, std::string extension)
{
    using namespace P4V;
    return std::vector<std::string>();
}
