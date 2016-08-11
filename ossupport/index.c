#define IN_ATKOS
#include <system.h>
#undef index
#undef strchr
char *
index(s, c)
char *s;
char c;
{
    return(strchr(s, (int) c));
}

