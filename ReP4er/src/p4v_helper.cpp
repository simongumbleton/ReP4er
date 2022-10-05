#include "p4v_helper.h"
#include "platformhelpers.h"
#include "reaperHelpers.h"

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
    std::string defaultdesc = "ReP4er changelist";
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

bool P4V::login()
{
    using namespace P4V;
    if (checkLoginStatus())
    {
        return true;
    }
    else
    {
        //user is not logged in, so provide a cmd prompt for them to login
        PrintToConsole("User is not logged in to P4 - Please enter password to login");
        int result = execCmd("p4 login");
        return result == 0;
    }
}
    

bool P4V::checkLoginStatus()
{
    using namespace P4V;
    int result = execCmd("p4 login -s");
    return result == 0;
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
        currentChangelist = std::stoi(splitResult[1]);
        return currentChangelist;
    }
    currentChangelist = 0;
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

void P4V::checkoutDirectory(std::string dirPath, std::string extenstion, int changelist)
{
    using namespace P4V;
    if (changelist <= 0)
    {
        changelist = createChangelist(defaultdesc);
    }
    if (extenstion.empty())
    {
        dirPath += R"(...)";
    }
    else
    {
        auto type = PLATFORMHELPERS::stringReplace(extenstion, ".", "");
        dirPath += R"(.../*.)" + type;
    }
    std::string command1 = R"(p4 edit -c )"
        + std::to_string(changelist)
        + R"( ")" + dirPath + R"(")";
    execCmd(command1);
}

void P4V::checkoutFiles(std::vector<std::string> fileList, int changelist)
{
    using namespace P4V;
    if (changelist <= 0)
    {
        changelist = createChangelist(defaultdesc);
    }
    for (auto file : fileList)
    {
        std::string command1 = R"(p4 edit -c )"
            + std::to_string(changelist)
            + R"( ")" + file + R"(")";
        execCmd(command1);
        std::string command2 = R"(p4 reconcile -c )"
            + std::to_string(changelist)
            + R"( ")" + file + R"(")";
        execCmd(command2);
        std::string command3 = R"(p4 reopen -c )"
            + std::to_string(changelist)
            + R"( ")" + file + R"(")";
        execCmd(command3);
    }
}

void P4V::reconcileDirectory(std::string dirPath, std::string extenstion, int changelist)
{
    using namespace P4V;
    if (changelist <= 0)
    {
        changelist = createChangelist(defaultdesc);
    }
    if (extenstion.empty())
    {
        dirPath += R"(...)";
    }
    else
    {
        auto type = PLATFORMHELPERS::stringReplace(extenstion, ".", "");
        dirPath += R"(.../*.)" + type;
    }
    std::string command1 = R"(p4 reconcile -c )"
        + std::to_string(changelist)
        + R"( ")" + dirPath + R"(")";
    execCmd(command1);
}

void P4V::reopenDirectory(std::string dirPath, std::string extenstion, int changelist)
{
    using namespace P4V;
    if (changelist <= 0)
    {
        changelist = createChangelist(defaultdesc);
    }
    if (extenstion.empty())
    {
        dirPath += R"(...)";
    }
    else
    {
        auto type = PLATFORMHELPERS::stringReplace(extenstion, ".", "");
        dirPath += R"(.../*.)" + type;
    }
    std::string command1 = R"(p4 reopen -c )"
        + std::to_string(changelist)
        + R"( ")" + dirPath + R"(")";
    execCmd(command1);
}

void P4V::submitChanges(int changeList, bool deleteIfEmpty)
{
    using namespace P4V;
    if (changeList == 0)
    {
        if (currentChangelist == 0)
        {
            createChangelist(defaultdesc);
        }
        changeList = currentChangelist;
    }
    if (doesChangelistHaveFiles(changeList))
    {
        std::string command1 = "p4 submit -c " + std::to_string(changeList);
        execCmd(command1);
        currentChangelist = 0;
    }
    else if (deleteIfEmpty)
    {
        deleteChangelist(changeList);
    }
}

bool P4V::doesChangelistHaveFiles(int changeList)
{
    using namespace P4V;
    bool hasFiles = false;
    if (changeList == 0)
    {
        if (currentChangelist == 0)
        {
            createChangelist(defaultdesc);
        }
        changeList = currentChangelist;
    }
    std::string command1 = "p4 opened -c " + std::to_string(changeList);
    auto result = execCmd_GetOutput(command1);
    if (result.find("File(s) not opened on this client.") != result.npos)
    {
        hasFiles = false;
    }
    else {
        hasFiles = true;
    }

    return hasFiles;
}

void P4V::deleteChangelist(int changeList)
{
    using namespace P4V;
    if (changeList == 0)
    {
        if (currentChangelist == 0)
        {
            createChangelist(defaultdesc);
        }
        changeList = currentChangelist;
    }
    if (!doesChangelistHaveFiles(changeList))
    {
        std::string command1 = "p4 change -d " + std::to_string(changeList);
        execCmd(command1);
        currentChangelist = 0;
    }
}

void P4V::revertUnchangedFilesInDir(std::string path)
{
    using namespace P4V;
    path += R"(...)";
    std::string command1 = R"(p4 revert -a ")" + path + R"(")";
    execCmd(command1);

}

void P4V::revertUnchangedFiles(std::vector<std::string> fileList)
{
    using namespace P4V;
    for (auto file : fileList)
    {
        std::string command1 = R"(p4 revert -a ")" + file + R"(")";
        execCmd(command1);
    }
}

std::vector<std::string> P4V::getFilesInChangelist(int changeList, std::string extension)
{
    using namespace P4V;
    std::vector<std::string> files;
    if (changeList == 0)
    {
        if (currentChangelist == 0)
        {
            createChangelist(defaultdesc);
        }
        changeList = currentChangelist;
    }
    if (doesChangelistHaveFiles(changeList))
    {
        std::string command1 = R"(p4 opened -s -c )" + std::to_string(changeList);
        auto result = execCmd_GetOutput(command1);
        auto lines = PLATFORMHELPERS::stringSplitToList(result, "\n");
        for (auto line : lines)
        {
            auto split = "change " + std::to_string(changeList) + " by";
            if (line.find(split) != line.npos)
            {
                auto file = PLATFORMHELPERS::stringSplitToList(line, split)[0];
                //file = file.substr(0, file.size() - 3); // removes " - " from the end
                std::size_t found = file.rfind(" - ");
                if (found != std::string::npos)
                {
                    file = file.substr(0, found);
                    std::string command1 = R"(p4 fstat -T clientFile ")" + file + R"(")";
                    auto result = execCmd_GetOutput(command1);
                    if (result.find("clientFile ") != result.npos)
                    {
                        auto localFile = PLATFORMHELPERS::stringSplitToList(result, "clientFile ")[1];
                        localFile = PLATFORMHELPERS::stringReplace(localFile, "\n","");
                        if (extension.empty())
                        {
                            files.push_back(localFile);
                        }
                        else
                        {
                            auto type = PLATFORMHELPERS::stringReplace(extension, ".", "");
                            if (PLATFORMHELPERS::ends_with(localFile, "."+type))
                            {
                                files.push_back(localFile);
                            }
                        }

                    }
                }
            }
        }
    }

    return files;
}
