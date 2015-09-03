#include "NetworkConfig.h"

NetworkConfig::NetworkConfig()
{
	char full_path[FILENAME_MAX];
	GetCurrentDir(full_path, sizeof(full_path));
	strcat(full_path, "\\network.ini");

	// networking config is just one section with one value
	// example:
	// [bind]
	// addr=tcp://192.168.56.1:5555

	const char* default_value = "tcp://192.168.56.1:5555";
	TCHAR bind_value[FILENAME_MAX];
	DWORD bind_value_read_ok = GetPrivateProfileString(
		"bind",
		"addr",
		default_value,
		bind_value,
		FILENAME_MAX,
		full_path
		);

	socket_binding = std::string{ bind_value };
}

const char* NetworkConfig::GetSocketBinding()
{
	return socket_binding.c_str();
}