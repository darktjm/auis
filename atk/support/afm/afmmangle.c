#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/param.h>

#define MAXLEN (14)

static char *SquishAFMFileName(char *orig)
{
    static char result[MAXLEN+1];
    int total[MAXLEN];
    int ix, val, oval;

    for (ix=0; ix<MAXLEN; ix++)
	total[ix] = 0;

    for (ix=0; *orig; orig++) {

	val = *orig;
	if (val >= 'a' && val <= 'z')
	    val = val-'a'+0;
	else if (val >= 'A' && val <= 'Z')
	    val = val-'A'+26;
	else if (val >= '0' && val <= '9')
	    val = val-'0'+52;
	else val = 62;

	total[ix] = (total[ix]+val) % 52 + 1;

	ix++;
	if (ix >= MAXLEN)
	    ix = 0;
    }

    for (ix=0; ix<MAXLEN; ix++) {
	if (total[ix]==0)
	    result[ix] = '\0';
	else if (total[ix] <= 26)
	    result[ix] = total[ix]-1+'a';
	else if (total[ix] <= 52)
	    result[ix] = total[ix]-27+'A';
	else
	    result[ix] = '-';
    }

    return result;
}

main(int argc, char *argv[])
{
    char buf[MAXPATHLEN];
    char *cx;
    int ix;

    if (argc > 1) {
	for (ix=1; ix<argc; ix++) {
	    cx = SquishAFMFileName(argv[ix]);
	    printf("%s\n", cx);
	}
    }
    else {

	while (cx = fgets(buf, MAXPATHLEN, stdin)) {
	    cx = strchr(buf, '\n');
	    if (cx)
		*cx = '\0';

	    cx = SquishAFMFileName(buf);
	    printf("%s\n", cx);
	}
    }
}
