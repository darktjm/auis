/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("textref.H")
#include <textref.H>

ATKdefineRegistryNoInit(textref, fnote);

const char * textref::ViewName()
{
    return "textrefv";
}
 char *textref::GetRef(long  size,char  *buf)
{
    char *c,*name;
    long realsize,i;
    i = 0;
    realsize = (this)->GetLength();
    while(realsize > 0 && (this)->GetChar(i) == ' ') {i++; realsize--;}
    if(size > realsize) size = realsize;
    (this)->CopySubString(i,size,buf, FALSE);
    c = buf;
    if(*c == '#'){
	while(*c != ' ' && *c != '\t' && *c != '\0')c++;
	if (*c == '\0') c = buf;
	else c++;
	if (*c == '\0') c = buf;
    }
    for(name = c; *c != '\0'; c++) if(*c == ' ' || *c == '\t') *c = '-';
    c--;
    while(*c == '-') *c-- = '\0';
    return name;
}
