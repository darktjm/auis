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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/console/lib/RCS/venusmon.C,v 1.7 1996/09/03 19:10:25 robr Exp $";
#endif


 

/* 
 ***************************************************************
 * Routines swiped from mariner for Instrument Console
 * These routines monitor file system traffic.
 ***************************************************************
 */

#include <andyenv.h>

#include <consoleClass.H>
#include <im.H>
#include <fontdesc.H>
#include <graphic.H>
#include <rect.h>
#include <console.h>
#include <environ.H>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#ifdef AFS_ENV
#include <afs/param.h>
#include <afs/vice.h>
#include <afs/errors.h>
#include <afs/prs_fs.h>
#define bool_t int
#include <afs/afsint.h>
#endif /* AFS_ENV */
#include <sitevars.h>

#include <util.h>


extern char Pstring1[],Pstring2[],Pstring3[];

 /* something like this seems to be needed - ghoti */
int venusSocket=-1;		/* socket to Venus */
boolean FetchInProgress, StoreInProgress;
char LastFetchMsg[128],  LastStoreMsg[128];
extern class fontdesc *console10font;
char OtherVenusStr[150], FetchVenusStr[150], FinishedVenusStr[150];
extern boolean NonViceHost, NonAFSDHost;

#ifndef NORCSID
#endif
#ifdef AFS_ENV
#endif /* AFS_ENV */
void IsViceRunning();
void InitializeMariner(class consoleClass  *self);
extern "C" int Bind (int  service, char  *host);
void LogMarinerFetchInfo(struct display  *disp);
void VenusNovelty(class consoleClass  *self, char  *rock);
int IsViceError(int  n);


void IsViceRunning()
{
    const char *s;
    
    mydbg(("entering: IsViceRunning\n"));
    NonViceHost = (ViceIsRunning() != 0 ? FALSE : TRUE);
    if (! NonViceHost) {
	if ((s = environ::GetConfiguration("AMS_NonViceHost")) != NULL){
	    if (s[0] == 'y' || s[0] == 'Y' || s[0] == 't' || s[0] == 'T' || s[0] == '1'){
		NonViceHost = TRUE;
	    }
	    else if (s[0] == 'n' || s[0] == 'N' || s[0] == 'f' || s[0] == 'F' || s[0] == '0'){
		NonViceHost = FALSE;
	    }
	}
    }
     /* find out if AFSD is not running */
    if (! NonViceHost){
	if ((s = environ::GetConfiguration("VENUS")) != NULL){
	    if (s[0] == 'y' || s[0] == 'Y' || s[0] == 't' || s[0] == 'T' || s[0] == '1'){
		NonAFSDHost = TRUE;
	    }
	    else if (s[0] == 'n' || s[0] == 'N' || s[0] == 'f' || s[0] == 'F' || s[0] == '0'){
		NonAFSDHost = FALSE;
	    }
	}
	else {
	    NonAFSDHost = FALSE;
	}
    }
}

extern void CheckMariner(FILE  *ActiveVenus, char *self);

extern "C" int make_socket(int  port);

void InitializeMariner(class consoleClass  *self)
    {
#ifdef AFS_ENV
    char *p;
    int flags;

/* 
 * Look for initialized name of venus -- could have used initstring on any of
 * three instruments.  "Both" is out of date but we try to handle it robustly
 * since lots of console files use it.
 */

    mydbg(("entering: InitializeMariner\n"));
    IsViceRunning();
    if(!NonViceHost){
	Numbers[MARINEROTHER].RawText = OtherVenusStr;
	Numbers[MARINERFETCH].RawText = FetchVenusStr;
	Numbers[MARINERFINISHED].RawText = FinishedVenusStr;
	FetchInProgress = StoreInProgress = FALSE;
	LastFetchMsg[0] = '\0';
	LastStoreMsg[0] = '\0';
	/* Find Venus and start talking to it. */
	if (NonAFSDHost){
	    if ((venusSocket = Bind(_SITE_VENUS_ITC_SOCKET, NULL)) < 0) {
		ReportInternalError(self, "console: Initial bind to venus.itc (2106) failed; trying 2107...");
		if ((venusSocket = Bind(_SITE_VENUS_ITC_SOCKET_ALT, NULL)) < 0) {
		    ReportInternalError(self, "console: Can't find venus.itc (210[67]): can't report AFS traffic.");
		    VenusIn = NULL;
		    return;
		}
	    }
	    flags = fcntl(venusSocket, F_GETFL, 0);
#if POSIX_ENV
	    flags=fcntl(venusSocket, F_SETFL, flags | O_NONBLOCK);
#else
	    flags=fcntl(venusSocket, F_SETFL, flags | FNDELAY);
#endif
	    if(flags!=0) {
		fprintf(stderr, "console: Couldn't set non-blocking mode on Venus monitor.\n");
	    }
	    VenusIn = fdopen(venusSocket, "r");
	    /* Set the options we want. */
	    p = "set:fetch\n";
	    write(venusSocket, p, strlen(p));
	    im::AddFileHandler(VenusIn, CheckMariner, (char *) self, 1);
	}
	else{
	    venusSocket = 0;

	    if ((venusSocket = make_socket(_SITE_VENUS_ITC_SOCKET)) < 0){
		ReportInternalError(self, "Venus will not be monitored");
		return;/* should I be setting some global flag ? */
	    }
	    flags = fcntl(venusSocket, F_GETFL, 0);
#if POSIX_ENV
	    flags=fcntl(venusSocket, F_SETFL, flags | O_NONBLOCK);
#else
	    flags=fcntl(venusSocket, F_SETFL, flags | FNDELAY);
#endif
	    if(flags!=0) {
		fprintf(stderr, "console: Couldn't set non-blocking mode on Venus monitor.\n");
	    }
	    VenusIn = fdopen(venusSocket, "r");
	    if (VenusIn == NULL){
		ReportInternalError(self, "fdopen of VenusIn failed");
		return; /*global flags anyone? */
	    }
	    im::AddFileHandler(VenusIn, CheckMariner, (char *) self, 1);
	}
    }
#endif /* AFS_ENV */
}




extern boolean REVSCROLL;

void LogMarinerFetchInfo(struct display  *disp)
    {
#ifdef AFS_ENV
    int ct = 0;
    char DoneString[256];
    struct RegionLog *logptr;

    mydbg(("entering: LogMarinerFetchInfo\n"));
    if(!NonViceHost){
	if (disp->DisplayStyle == REVERSESCROLLING){
	    REVSCROLL = TRUE;
	}
	logptr = disp->AssociatedLog;
	if (FetchInProgress) {
	    AddStringToLog(LastFetchMsg, logptr);
	    ++ct;
	}
	if (StoreInProgress) {
	    AddStringToLog(LastStoreMsg, logptr);
	    ++ct;
	}
	if (ct == 0) { /* Nothing going on, so report on what is DONE */
	    if (LastFetchMsg[0]) {
		sprintf(DoneString, "%s%s", (NonAFSDHost) ? "Done " : "-->> ", LastFetchMsg);
		AddStringToLog(DoneString, logptr);
		++ct;
	    }
	    if (LastStoreMsg[0]) {
		sprintf(DoneString, "%s%s", (NonAFSDHost) ? "Done " : "-->> ", LastStoreMsg);
		AddStringToLog(DoneString, logptr);
		++ct;
	    }
	}
	if (ct == 0) {
	    AddStringToLog("There has been no file system activity.\n", logptr);
	}
    }
#endif /* AFS_ENV */
}



void VenusNovelty(class consoleClass  *self, char  *rock)
        {
    int dum;

    mydbg(("entering: VenusNovelty\n"));
    PauseEnqueuedEvents = TRUE;
    ClearWindow(self);
    InitPstrings();
    sprintf(Pstring1, "Once Upon A Time, In The AFS...");
    sprintf(Pstring2, "...When Venus Died... ");
    sprintf(Pstring3, "(click anywhere)");
    PromptToWindow(self);
    (self)->SetTransferMode( graphic_BLACK);
    (self)->SetFont( console10font);
    (self)->MoveTo( 0, (self)->GetLogicalHeight());
    (self)->DrawString( "X", graphic_ATLEFT | graphic_ATBOTTOM);
    while (! ((self)->GetIM())->GetCharacter()) {
	;
    }
    ClearWindow(self);
    InitPstrings();
    sprintf(Pstring1, "...She Would Rise From The Ashes...");
    sprintf(Pstring2, ".....Like A Phoenix.....");
    PromptToWindow(self);
    (self)->SetTransferMode( graphic_BLACK);
    (self)->SetFont( console10font);
    (self)->MoveTo(0, (self)->GetLogicalHeight());
    (self)->DrawString( "X", graphic_ATLEFT | graphic_ATBOTTOM);
    (self)->FlushGraphics();
    sleep(4);

    ClearWindow(self);
    (self)->SetTransferMode( graphic_BLACK);
    (self)->SetFont( console10font);
    (self)->MoveTo( 0, (self)->GetLogicalHeight());
    (self)->DrawString( "X", graphic_ATLEFT | graphic_ATBOTTOM);
    (self)->SetFont( console10font);
    for (dum = (self)->GetLogicalHeight(); dum > 0; --dum) {
        if (dum < (self)->GetLogicalHeight()) {
	    (self)->SetTransferMode( graphic_WHITE);
	    (self)->MoveTo( 0, dum + 1);
	    (self)->DrawString( "V", graphic_ATLEFT | graphic_ATTOP);
	    (self)->SetTransferMode( graphic_BLACK);
        }
	(self)->MoveTo( 0, dum);
	(self)->DrawString( "V", graphic_ATLEFT | graphic_ATTOP);
	(self)->FlushGraphics();
    }
    InitPstrings();
    sprintf(Pstring1, "....And Live On Again....");
    sprintf(Pstring2, "(in a somewhat confused state)");
    sprintf(Pstring3, "And Now - Back To The Show");
    PromptToWindow(self);
    (self)->FlushGraphics();
    sleep(4);
    PauseEnqueuedEvents = FALSE;
    RedrawDisplays(self);
}

int IsViceError(int  n)
    {
    mydbg(("entering: IsViceError\n"));
    switch(errno) {
        case EPIPE:
        case ENXIO:
        case ETIMEDOUT:
        case ENOTTY: case EIO: case ENODEV:
        case 0: /* Is the pioctl stuff *really* fixed?  I doubt it */
#ifdef AFS_ENV
#ifdef VOFFLINE
        case VOFFLINE:
#endif /* VOFFLINE */
#ifdef VBUSY
        case VBUSY:
#endif /* VBUSY */
#endif /* AFS_ENV */
            return(1);
        default:
            return(0);
    }
}
