#define IN_ATKOS
#include <system.h>
#undef bcopy
#undef memmove


void
bcopy(s1, s2, len)
char *s1, *s2;
int len;
{
    memmove(s2, s1, len);
}
