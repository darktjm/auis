/* Copyright 1996 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */
#define IN_ATKOS_LIB 1
#include <system.h>
#include <relativize.H>
 
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
