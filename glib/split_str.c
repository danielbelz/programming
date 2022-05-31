#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>

#define MAX_BUF 1024

void parse_string(char* buf, int buflen, char div)
{
	char **logtag, **r;

	logtag = g_strsplit(buf, div, 0);
	if (g_strv_length(logtag) == 0) {
		return;
	}

	for (r = logtag; *r; r++) {
		if (strlen(*r) > 1) {
            printf("[%c] Found token [%s]\n", div, r);
		}
	}

	g_strfreev(logtag);
}

int main()
{
    char *str = "THIS;IS;A;TEST";
    char div = ';';
	parse_string(str, strlen(str), div);
	return 0;
}