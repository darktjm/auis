#define IN_ATKOS
#include <system.h>
#undef bcmp
#undef memcmp
int
bcmp(s1, s2, len)
char *s1, *s2;
int len;
{
    return(memcmp(s1, s2, len));
}
