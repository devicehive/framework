#pragma once

#include <string>
#include <unistd.h>


std::string exec(const std::string& cmd) 
{
	std::string result;

	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) return "ERROR";
	
	char buffer[128];
	while(!feof(pipe)) {
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);

	return result;
}
