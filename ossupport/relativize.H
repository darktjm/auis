#ifndef _RELATIVIZE_H_
#define _RELATIVIZE_H_
/* Copyright 1995 Carnegie Mellon University All rights reserved. */

extern char *NewString(char *str);
extern int AddToDontList(char *path);
extern boolean Dont(char *path);
extern int AddtoDirList(char *path);
extern void AddArg(char *arg);
extern void ProcessArg(char *arg, char *dot, char *slash);
#endif
