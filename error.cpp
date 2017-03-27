#include "error.h"

void err(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}