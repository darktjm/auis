/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getvconf.c,v 1.9 1992/12/15 21:19:11 rr2b Stab74 $";
#endif

#include <andyenv.h>
#include <ms.h>
#include <stdio.h>

#define StrEql(a,b)(strcmp(a,b)==0)
/*
  dummy routine to get the rest of ams working again
*/
long MS_GetVConfig(key,vers,result)
char *key;
char *vers;
char *result;
{if(key==0)key="(null)";
 if(vers==0)vers="(null)";
 if (!access("/debug.getvconfig", F_OK)) /*print debugging info?*/
  fprintf(stderr,"MS_GetVConfig:key=%s,vers=%s",key,vers); 
 fflush(stderr);
 if(key==0)
  strcpy(result,"");
 else if(StrEql(key,"motd"))
  strcpy(result,"/etc/motd");
 else if(StrEql(key,"expire"))
  strcpy(result,"0,");
 else
  strcpy(result,"");
 return 0;
}
