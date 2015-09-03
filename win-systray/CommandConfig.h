#pragma once

#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#define GetCurrentDir _getcwd

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using SectionNameList = std::vector < std::string > ;
using ProgramRegistry = std::unordered_map< std::string, std::string >;
using RegKey = const std::string&;
using RegEntry = const std::string&;

class CommandConfig
{
private:
	const int BUFF_SIZE = 1024;
	LPSTR section_name_buffer;

	SectionNameList section_names;
	ProgramRegistry cmd_reg;

public:
	CommandConfig();
	~CommandConfig();

	SectionNameList GetSectionNames();
	ProgramRegistry GetRegistry();
	RegEntry GetRegistryEntry(RegKey s);
};

