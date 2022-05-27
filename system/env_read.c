#include <stdio.h>
#include <stdlib.h>

// Read environment variable from shell

int main()
{
	printf("Read PATH environment variable from shell\n");

	const char* s = getenv("PATH");
	printf("PATH :%s\n", (s != NULL) ? s : "getenv returned NULL");

	return 0;
}
