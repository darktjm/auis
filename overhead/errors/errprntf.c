/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* *************************************************************** 

 errprntf.c:  Routine for printing Andrew-standard errors.

		********************************************
		**** For documentation, see errprntf.h **** 
		********************************************

*/
#include <andrewos.h>
#include <stdio.h>
#include "errprntf.h"

#define CONTROLMAX 1000  /* Longest printf control string */


int safefprintf(FILE *fp, const char *control, ...)
{
    va_list ap;

    va_start(ap, control);
    vfprintf(fp, control, ap);
    va_end(ap);
    
    fflush(fp);
    if (ferror(fp)) {
	fp = freopen("/dev/console", "w", fp);
	if (fp == NULL) {
	    fp=freopen("/dev/null", "w", fp);
	    return(-1);
	}
	
	va_start(ap, control);
	vfprintf(fp, control, ap);
	va_end(ap);
	
	fflush(fp);
	if (ferror(fp)) {
	    return(-1);
	}
    }
    return(0);
}
static const char formatstr[]="<%s%s%s%s%s%s%s>";

int
errprintf(const char *application, int type, const char *log, const char *id, const char *format, ...)
{
    int ret;
    va_list ap;
    va_start(ap, format);
    ret = errprintv(application, type, log, id, format, ap);
    va_end(ap);
    return ret;
}

int
errprintv(const char *application, int type, const char *log, const char *id, const char *format, va_list ap)
{
    const char *typestr;
    int numfields;
    static FILE *fp=NULL;
    
    if (type < 0 || type > 9) type = 0;
    numfields = 1;
    if (application) numfields = 2;
    if (log) numfields = 3;
    if (id) numfields = 4;
    if (type == ERR_CRITICAL) {
	typestr = "critical";
    } else if (type <= ERR_WARNING) {
	typestr = "warning";
    } else if (type <= ERR_MONITOR) {
	typestr = "monitor";
    } else {
	typestr = "debug";
    }
    if(!fp)
	fp = stderr;
    fprintf(fp, formatstr, typestr, 
	(numfields > 1) ? ":" : "",
	application ? application : "",
	(numfields > 2) ? ":" : "",
	log ? log : "",
	(numfields > 3) ? ":" : "",
	    id ? id : "");
    if(ferror(fp)) {
	fp=freopen("/dev/console", "w", fp);
	if(fp==NULL) {
	    fp=freopen("/dev/null", "w", fp);
	    return -1;
	}
	fprintf(fp, formatstr, typestr, 
	(numfields > 1) ? ":" : "",
	application ? application : "",
	(numfields > 2) ? ":" : "",
	log ? log : "",
	(numfields > 3) ? ":" : "",
	    id ? id : "");
    }
    vfprintf(fp, format, ap);
    if(ferror(fp)) {
	fp=freopen("/dev/console", "w", fp);
	if(fp==NULL) {
	    fp=freopen("/dev/null", "w", fp);
	    return -1;
	}
	vfprintf(fp, format, ap);
    }
    fprintf(fp, "\n");
    if(ferror(fp)) {
	fp=freopen("/dev/console", "w", fp);
	if(fp==NULL) {
	    fp=freopen("/dev/null", "w", fp);
	    return -1;
	}
	fprintf(fp, "\n");
    }
    fflush(fp);
    return ferror(fp);
}
