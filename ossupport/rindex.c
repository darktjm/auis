#define IN_ATKOS
#include <system.h>
#undef rindex
#undef strrchr
char *
rindex(s, c)
char *s;
char c;
{
    return(strrchr(s, (int) c));
}

