#include <fjson.h>

char *indent_str(unsigned int indent)
{
	static char buffer[4096];

	unsigned int i = 0;
	for (i = 0; i < indent; i++)
	{
		buffer[i] = ' ';
	}

	buffer[i] = '\0';

	return buffer;
}