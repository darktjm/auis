#if defined(vax) || defined(mips)
#define VOID int
#else
#define VOID void
#endif

#define NEW(t) ((t *)malloc(sizeof(t)))
#define FREE(x) free((char *)x)

#ifndef NULL
#define NULL 0
#endif

#define bool int
#define TRUE 1
#define FALSE 0

char *getenv();
