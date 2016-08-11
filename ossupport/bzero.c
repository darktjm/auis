#define IN_ATKOS
#include <system.h>
#undef bzero
#undef memset
void
bzero(sp, len)
char *sp;
int len;
{
    memset(sp, 0, len);
}
