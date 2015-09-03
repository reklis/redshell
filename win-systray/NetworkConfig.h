#pragma once

#include <Windows.h>
#include <direct.h>
#include <stdio.h>
#define GetCurrentDir _getcwd

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

class NetworkConfig
{
private:
	std::string socket_binding;

public:
	NetworkConfig();

	const char* GetSocketBinding();
};

