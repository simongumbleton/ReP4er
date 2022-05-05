#include "platformhelpers.h"

#include <string>
#include <algorithm>

#include <cstdlib>
#include <regex>
#include <limits>


///Definitions

std::string PLATFORMHELPERS::stringToLower(std::string input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}

std::string PLATFORMHELPERS::stringToUpper(std::string input)
{
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::toupper(c); });
	return result;
}

bool PLATFORMHELPERS::isEqual(float A, float B)
{
	return abs(A - B) <= std::numeric_limits<float>::epsilon();
}

bool PLATFORMHELPERS::isEqual(double A, double B)
{
	return abs(A - B) <= std::numeric_limits<double>::epsilon();
}

bool PLATFORMHELPERS::stringIsNumber(std::string input)
{
	for (auto c : input)
	{
		if (!isdigit(c)) { return false; }
	}
	return true;
}

bool PLATFORMHELPERS::stringIsDecimalNumber(std::string input)
{
	for (auto c : input)
	{
		if (!isdigit(c) and c != '.')
		{
			return false;

		}
	}
	return true;
}

bool PLATFORMHELPERS::stringIsTimecode(std::string input)
{
	std::regex tcPattern("[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]");
	std::vector<std::string> matches;
	std::sregex_iterator iter(input.begin(), input.end(), tcPattern);
	std::sregex_iterator end;
	int count = 0;
	while (iter != end)
	{
		for (unsigned i = 0; i < iter->size(); ++i)
		{
			matches.push_back((*iter)[i]);
		}
		++iter;
		count++;
	}
	return matches.size() == 1;
}


std::string PLATFORMHELPERS::cleanWwisePathsFromMac(std::string input)
{
#ifndef _WIN32
	std::string home{ std::getenv("HOME") };
	juce::String result = input;
	return result.replace("Y:", home).replace("\\", "/").toStdString();
#else
	return input;
#endif
}

std::string PLATFORMHELPERS::stringReplace(std::string input, std::string from, std::string to)
{
	juce::String result = input;
	return result.replace(from, to).toStdString();
}

std::string PLATFORMHELPERS::filenameFromPathString(std::string input, bool removeExtension)
{
	if (input.rfind(kPathSeparator) != input.npos)
	{
		input.erase(0, input.rfind(kPathSeparator) + 1);
		if (removeExtension)
		{
			input = stringSplitToList(input, ".")[0];
		}
	}
	return input;
}

std::vector<std::string> PLATFORMHELPERS::stringSplitToList(std::string target, std::string delim)
{
	std::vector<std::string> v;
	if (!target.empty()) {
		std::string::size_type start = 0;
		do {
			size_t x = target.find(delim, start);
			if (x == target.npos)
			{
				//no more delimeters found so save the last remaining token
				std::string token = target.substr(start);
				v.push_back(token);
				break;
			}
			if (x != 0)
			{
				std::string token = target.substr(start, x - start);
				v.push_back(token);
				start += token.size() + delim.size();
			}
			else
			{//found delim at the start so skip adding an empty token and just move start forward by the delim size
				start += delim.size();
			}


		} while (true);
	}
	return v;
}
//...
bool PLATFORMHELPERS::ends_with(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	//return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	return std::equal(ending.begin(), ending.end(), value.end()-ending.size());
}

bool PLATFORMHELPERS::starts_with(std::string const& value, std::string const& startstring)
{
	if (startstring.size() > value.size()) return false;
	return std::equal(startstring.begin(), startstring.end(), value.begin());
}

File PLATFORMHELPERS::askUserForFile(std::string message, std::string extension)
{
	std::unique_ptr<FileChooser> myChooser;
	myChooser = std::make_unique<FileChooser>(message,
		File::getSpecialLocation(File::userHomeDirectory),
		extension);
	myChooser->browseForFileToOpen();
	return myChooser->getResult();
}

File PLATFORMHELPERS::askUserForDirectory(std::string message)
{
	std::unique_ptr<FileChooser> myChooser;
	myChooser = std::make_unique<FileChooser>(message,
		File::getSpecialLocation(File::userHomeDirectory));
	myChooser->browseForDirectory();
	return myChooser->getResult();
}

Array<File> PLATFORMHELPERS::GetFilesInDirectory(File dir, std::string type)
{
	if (dir.exists() && dir.isDirectory())
	{
		return dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true, type);
	}
	return Array<File>();
}

std::vector<std::string> PLATFORMHELPERS::GetFilesInDirectory(std::string dir, std::string type)
{
	std::vector<std::string> filelist;
	File folder(dir);
	for (auto file : GetFilesInDirectory(folder, type))
	{
		filelist.push_back(file.getFullPathName().toStdString());
	}
	return filelist;
}

bool PLATFORMHELPERS::doesFilePathExist(std::string filepath)
{
	File path(filepath);
	return path.exists();
}

bool PLATFORMHELPERS::isPathDirectory(std::string filepath)
{
	File path(filepath);
	return path.isDirectory();
}

bool PLATFORMHELPERS::isPathFile(std::string filepath)
{
	File path(filepath);
	return path.existsAsFile();
}


std::vector<std::string> PLATFORMHELPERS::FindRegexMatches(std::string inString, std::string regexPattern)
{
	//std::regex tcPattern("[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]\\:[0-9][0-9]");
	std::regex tcPattern(regexPattern);
	std::vector<std::string> matches;
	std::sregex_iterator iter(inString.begin(), inString.end(), tcPattern);
	std::sregex_iterator end;
	int count = 0;
	while (iter != end)
	{
		for (unsigned i = 0; i < iter->size(); ++i)
		{
			matches.push_back((*iter)[i]);
		}
		++iter;
		count++;
	}
	return matches;
}
