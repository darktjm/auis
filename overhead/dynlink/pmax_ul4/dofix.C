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

/* 
	dofix.c - convert .o file into .do file

	Author:  Zalman Stern July 1989
 */

/* This version of dofix is much more simpler than ones for other machine types
 * since MIPS' ld does what we want...
 */
#include <andrewos.h>
#include <stdio.h> /* For NULL in absence of stddef.h */
#include <ATKSymTab.H>
#include <fstream>

using namespace std;

static char *ComputeOutputFileName (char *InputFileName, char *extension)
{

    static char name[256];
    register char  *p, *q;
    char   *ext;

 /* copy the input name and look for the last '.' */

    for (p = InputFileName, q = name, ext = NULL; *p != '\0';) {
	if (*p == '/')		/* ignore period if '/' follows */
	    p++, q = name, ext = NULL;
	else
	    if ((*q++ = *p++) == '.')
		ext = q - 1;
    }
    if (ext == NULL)
	ext = q;
    *ext = '\0';

 /* overwrite the extension with new extension */

    strncat(name, extension, 255);
    if (strcmp(InputFileName, name) == 0)
	strncat(name, extension, 255);
    return name ;
}

char *ATKDoFix(char *p, ATKSymTab *ast, boolean dyndebug) {
    char *InputFileName;
    char *OutputFileName;
    int gotcha = 0;
    char CommandBuffer[2048];
    InputFileName = p;
    OutputFileName = ComputeOutputFileName(InputFileName, ".+");

    sprintf(CommandBuffer, "ld -x -r -e main %s -o %s",
	    InputFileName, OutputFileName);
    if(system(CommandBuffer)!=0) return NULL;

    if(ast) {
	sprintf(CommandBuffer,"nm -d -u %s|awk '$1==0 && $2==\"U\" { print $3}'", OutputFileName);

	FILE *fp=popen(CommandBuffer, "r");
	char symbuf[1024];
	if(fp==NULL) return NULL;
	while(!feof(fp) && fgets(symbuf,sizeof(symbuf)-1,fp)!=NULL) {
	    char *p=strchr(symbuf, '\n');
	    if(p) *p='\0';
	    if(!ast->FindSymbol(symbuf)) {
		cerr<<"dofix: undefined symbol "<<symbuf<<"."<<endl;
		pclose(fp);
		return NULL;
	    }
	}
	pclose(fp);
    }
    if(!dyndebug) unlink(InputFileName);
    return OutputFileName;
}
