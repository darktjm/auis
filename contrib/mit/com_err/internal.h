/*
 * internal include file for com_err package
 */
#include "mitcopyright.h"
#ifndef __STDC__
#undef const
#define const
#endif

#ifdef __STDC__
void perror (const char *);
#else
int perror ();
#endif
