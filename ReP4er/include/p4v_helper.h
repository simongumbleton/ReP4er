#pragma once
#include <cstdlib>
#include <string>
#include <unordered_map>

struct p4Info
{
	std::unordered_map<std::string, std::string> properties;
};

static p4Info P4INFO;

class ReaProject;

//Defined in Main
void LaunchCheckout();
void CheckoutCurrentProject();
void LaunchSubmit();
void LaunchSettings();


namespace P4V
{
	

	int execCmd(std::string in_command);

	std::string execCmd_GetOutput(std::string in_command);

	std::string getDateAndTimeNow();

	void setGlobalChangelistDescription(std::string in_description);

	void SetCWD(std::string in_cwd);

	std::string GetCWD();

	void showP4Info();

	p4Info* getP4Info();

	int createChangelist(std::string description);

	int findChangelistByDescription(std::string description);

	void checkoutDirectory(std::string dirPath, std::string extenstion = "",int changelist = -1);

	void checkoutFiles(std::vector<std::string> fileList, int changelist = -1);

	void reconcileDirectory(std::string dirPath, std::string extenstion = "", int changelist = -1);

	void reopenDirectory(std::string dirPath, std::string extenstion = "", int changelist = -1);

	void submitChanges(int changeList=0, bool deleteIfEmpty=true);

	bool doesChangelistHaveFiles(int changeList = 0);

	void deleteChangelist(int changeList = 0);

	void revertUnchangedFilesInDir(std::string path);

	void revertUnchangedFiles(std::vector<std::string> fileList);

	std::vector<std::string> getFilesInChangelist(int changeList = 0, std::string extension = "");

	bool login();

	bool checkLoginStatus();

}