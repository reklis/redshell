#include "CommandConfig.h"

CommandConfig::CommandConfig()
{
	section_name_buffer = new CHAR[BUFF_SIZE];

	char full_path[FILENAME_MAX];
	GetCurrentDir(full_path, sizeof(full_path));
	strcat(full_path, "\\commands.ini");
	DWORD data_read_ok = GetPrivateProfileSectionNames(section_name_buffer, BUFF_SIZE, full_path);

	if (data_read_ok)
	{
		// section_name_buffer now contains string1\0string2\0...

		for (LPSTR p = section_name_buffer; *p; ++p)
		{
			if (!p) {
				break;
			}

			std::string section_name(p);
			std::cout << section_name.c_str() << '\n';

			section_names.push_back(section_name);

			// each section has an exe key/value pair
			// [section]
			// exe=c:\foo\bar.exe
			// the section name is the prefix that will be used over the wire
			// the exe value is the command that will be executed
			TCHAR exe_value[FILENAME_MAX];
			LPCSTR app_name = section_name.c_str();
			
			DWORD exe_value_read_ok = GetPrivateProfileString(
				app_name,
				"exe",
				"c:\\windows\\system32\\notepad.exe",
				exe_value,
				FILENAME_MAX,
				full_path
			);

			if (exe_value_read_ok)
			{
				cmd_reg.emplace(section_name, exe_value);
			}

			p += section_name.size();  // push pointer past the end of the string
		}
	}
	
}

SectionNameList CommandConfig::GetSectionNames()
{
	return section_names;
}

ProgramRegistry CommandConfig::GetRegistry()
{
	return cmd_reg;
}

RegEntry CommandConfig::GetRegistryEntry(RegKey s)
{
	return cmd_reg.at(s);
}

CommandConfig::~CommandConfig()
{
	delete section_name_buffer;
}
