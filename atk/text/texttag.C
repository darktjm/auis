/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("texttag.H")
#include <texttag.H>

ATKdefineRegistryNoInit(texttag, fnote);

const char * texttag::ViewName()
{
    return "texttagv";
}
 char *texttag::GetTag(long  size,char  *buf)
{
    long i;
    char *c;
    long realsize;
    realsize = (this)->GetLength();
    i = 0;
    while(realsize > 0 && (this)->GetChar(i) == ' ') {i++; realsize--;}
    if(size > realsize) size = realsize;
    (this)->CopySubString(i,size,buf, FALSE);
    c = buf;
    for(c = buf; *c != '\0'; c++) if(*c == ' ' || *c == '\t') *c = '-'; 
    c--;
    while(*c == '-') *c-- = '\0';
    return buf;
}
