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

#include <stdio.h>
#include <errprntf.h>
#include <mailconf.h>
#include <mail.h>
#include <util.h>
#include <ms.h>

/* The functions NonfatalBizarreError and CriticalBizarreError are in separate files to make it easier for clients to override them  */

extern char home[];
extern FILE *fopen();

static char *ErrLevel(level)
int level;
{
    switch(level) {
	case ERR_CRITICAL:
	    return("CRITICAL");
	case ERR_WARNING:
	    return("warning");
	default:
	    return("odd level");
    }
}

BizarreError(text, level)
char *text;
int level;
{
    static char ProgName[12] = "";
    char Fname[1+MAXPATHLEN];
    static int IsPostman = -1;
    FILE *fp = NULL;

    if (!ProgName[0]) {
	if (IsPostman < 0) {
	    struct CellAuth *ca; char *PMName;
	    ca = NULL;
	    FindAMSHomeCell(&ca);
	    PMName = NULL;
	    if (ca != NULL) {FillInCell(ca); PMName = CheckAMSPMName(ca->CellName);}
	    if (ca != NULL && ca->UserName != NULL && PMName != NULL && strcmp(ca->UserName, PMName) == 0) IsPostman = 1;
	    else IsPostman = 0;
	}
	if (IsPostman > 0) {
	    sprintf(ProgName, "ms-%d", getpid());
	} else {
	    strcpy(ProgName, "ms");
	}
    }
    if (AMS_DevConsoleIsSacred) {
	strcpy(Fname, home);
	strcat(Fname, "/MS-Errors"); /* This string also appears in ams/messages/msgsbe.c */
	fp = fopen(Fname, "a");
    }
    if (fp) {
	long foo;
	foo =time(0);
	fprintf(fp, "Message server %s message reported on %s: %s", ErrLevel(level), ctime(&foo), text);
	fclose(fp);
    } else {
	errprintf(ProgName, level, 0, 0, "%s", text);
    }
}

