#pragma once
#include <cstdlib>
#include <string>
#include <unordered_map>

struct p4Info
{
	std::unordered_map<std::string, std::string> properties;
};

static p4Info P4INFO;

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

	void checkoutDirectory(std::string dirPath, std::string extenstion = "");

	void checkoutFiles(std::vector<std::string> fileList);

	void reconcileDirectory(std::string dirPath, std::string extenstion = "");

	void reopenDirectory(std::string dirPath, std::string extenstion = "");

	void submitChanges(int changeList=0, bool deleteIfEmpty=true);

	bool doesChangelistHaveFiles(int changeList);

	void deleteChangelist(int changeList);

	void revertUnchangedFilesInDir(std::string path);

	void revertUnchangedFiles(std::vector<std::string> fileList);

	std::vector<std::string> getFilesInChangelist(int changeList, std::string extension = "");

	bool login();

}