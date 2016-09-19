/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#define IN_ATKOS_LIB 1
#include <system.h>
#include <relativize.H>
#include <atkos.h>
 
static int lastwasout=0;
void ATKUseExportsFiles(char *arg, char *dot, char *slash) {
    if(!Dont(arg)) {
	if(dot && !lastwasout && (strcmp(dot+1, "a")==0 || strcmp(dot+1, ATKDYNEXT)==0)) {
	    char filename[MAXPATHLEN];
	    *dot='\0';
	    if(slash) {
		*slash='\0';
		strcpy(filename, arg);
		strcat(filename, "/");
		if(strncmp(slash+1, "lib", 3)==0) strcat(filename, slash+4);
		else strcat(filename, slash+1);
		*slash='/';
	    } else {
		if(strncmp(arg, "lib", 3)==0) strcpy(filename, arg+3);
		else strcpy(filename, arg);
	    }
	    *dot='.';
	    strcat(filename, ".exp");
	    if(access(filename, R_OK)==0) {
		AddArg(filename);
		return;
	    }
	}
    }
    if(strcmp(arg, "-o")==0) lastwasout=1;
    else lastwasout=0;
    AddArg(arg);
}
