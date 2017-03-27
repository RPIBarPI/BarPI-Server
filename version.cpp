#include "version.h"

std::string header()
{
	std::string vString="";
	vString+="+-------------------------+\n";
	vString+="| BarPI                   |\n";
	vString+="| Version "+getVersion()+"            |\n";
	vString+="| Author Robert Carneiro  |\n";
	vString+="+-------------------------+\n";
	return vString;
}