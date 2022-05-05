//
//  platformhelpers.h
//  reaper_wwise_csg
//
//  Created by Simon Gumbleton on 12/10/2020.
//  Copyright © 2020 My Company. All rights reserved.
//
#pragma once
#include "JuceHeader.h"

#ifndef platformhelpers_h
#define platformhelpers_h
#endif /* platformhelpers_h */

namespace PLATFORMHELPERS
{
	const char kPathSeparator =
#ifdef _WIN32
		'\\';
#else
		'/';
#endif


	//Declarations
	std::string stringToLower(std::string input);
	std::string stringToUpper(std::string input);
	bool isEqual(float A, float B);
	bool isEqual(double A, double B);
	bool stringIsNumber(std::string input);
	bool stringIsDecimalNumber(std::string input);
	bool stringIsTimecode(std::string input);
	std::string cleanWwisePathsFromMac(std::string input);
	std::string stringReplace(std::string input, std::string from, std::string to);
	std::string filenameFromPathString(std::string input, bool removeExtension = false);
	std::vector<std::string> stringSplitToList(std::string target, std::string delim);
	bool ends_with(std::string const& value, std::string const& ending);
	bool starts_with(std::string const& value, std::string const& startstring);
	File askUserForFile(std::string message = "Select EDL file", std::string extension = "*.edl");
	File askUserForDirectory(std::string message = "Select a folder");
	Array<File> GetFilesInDirectory(File dir, std::string type = "*.wav");
	std::vector<std::string> GetFilesInDirectory(std::string dir, std::string type = "*.wav");
	bool doesFilePathExist(std::string filepath);
	bool isPathDirectory(std::string filepath);
	std::vector<std::string> FindRegexMatches(std::string inString, std::string regexPattern);
	bool isPathFile(std::string filepath);
}





