/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* 
 ***************************************************************
 * Routines for monitoring messages on console service
 * and (on some machines) /dev/console.
 * Some routines swiped from ttyscript.
 ***************************************************************
 */

#include <andrewos.h> /* sys/types.h sys/time.h sys/file.h */
#include <im.H>
#include <consoleClass.H>
#include <menulist.H>
#include <proctable.H>
#include <console.h>
/* #include <sys/tty.h> */
#include <sitevars.h>
#include <errno.h>
#include <util.h>
#include <sys/socket.h>	/* for gethostname() */

extern "C" char *inet_ntoa(unsigned long addr);
extern int ConsoleSocket;
extern struct sockaddr_in ConsoleAddr;
extern int SubChannel;
extern char ptyname[30];
extern char ThisHostName[256];
extern boolean NonAFSDHost;
extern u_long ThisHostAddr;

#define ERRBUFSIZE 300
NO_DLL_EXPORT char PrimaryErrorBuffer[ERRBUFSIZE];

/* ****************************************************************
 * CheckErrorsIn -- checks the /dev/console channel, if it exists.
 * DESIGN:  When CheckErrorsIn detects a fatal error on the ErrorsIn
 * channel,it increments a counter and tries to open it again.  If this
 * happens more than MAXERRORRESTARTS times in one running of console, it
 * then gives up and resets ErrorsIn to be the same as winin (after
 * printing an error msg).  This should prevent this routine from ever
 * waking up again, which is good if the input is unreadable, but it has
 * the unfortunate effect of turning off all /dev/console error receiving
 * mechanisms on some machines.
 *****************************************************************
 */
/* Right now, zombie processes get left around on the romp, and they
 * tie up ptys,so it wouldn't be wise to restart too many times.
 */

#define DEVCONSOLEBUFSIZE 160
int AnotherError(class consoleClass  *self, const char  *errstring, boolean  AddNewline);


void CheckErrorsIn(FILE  *ErrorsIn, char *selfl)
{
    class consoleClass  *self=(class consoleClass *)selfl;
    static char buf[DEVCONSOLEBUFSIZE];
    static char *fillptr = buf;
    int c;

    mydbg(("entering: CheckErrorsIn\n"));
    errno = 0;

    while ((c = getc(ErrorsIn)) != EOF)  {
	if (c == '\r')
	    continue;
	if (c != '\n')  {
	    *fillptr++ = c;
	}
	if (c == '\n' || fillptr >= buf + sizeof(buf) - 2)  {
	    *fillptr++ = '\n';
	    *fillptr = '\0';
	    AnotherError(self, buf, FALSE);
	    fillptr = buf;
	}
    }
	
    if (errno != EWOULDBLOCK && errno != EAGAIN)  {
	if (errno != 0)  {
	    RestartErrorMonitoring(self, strerror(errno));
	}
#if !SY_AIX221 && !defined(M_UNIX) && !defined(__hpux)
#ifdef RCH_ENV
	/* IBM Rochester uses the consoled daemon.  We lost contact. */
	else  {
	    RestartErrorMonitoring(self, "the consoled daemon apparently died");
	    im::RemoveFileHandler(ErrorsIn);	/* give up */
	}
#else
	else  {
	    RestartErrorMonitoring(self, "Odd EOF on error channel");
	}
#endif
#endif
    }

    return;
}


extern "C" int FakeCheckConsoleSocket();
extern boolean WasUDPAction;
extern char ConsoleMessage[];
/*  */
int CheckConsoleSocket(FILE  *ConsoleIn, class consoleClass  *self)
        {
    int j;

    mydbg(("entering: CheckConsoleSocket\n"));

    while ((j = FakeCheckConsoleSocket()) > 0)  {
	   AnotherError(self, ConsoleMessage, TRUE);
    }

    if (j<0) {
	if (errno != EWOULDBLOCK && errno != EAGAIN)  {
	    ReportInternalError(self, "Error message possibly lost (reading console service");
	    return(-1);
	}
    }
    return 0;
}

static const char *mailargvector[] = {_SITE_QUEUEMAIL, "-i", "bogus", 0};

static char MyHost[100]="";

int AnotherError(class consoleClass  *self, const char  *errstring, boolean  AddNewline)
            {
    int Priority = DefaultErrorPriority, i, j, max, nerr;
    const char *ErrText, *ErrPri=NULL, *Application=NULL, *LogAddress=NULL;
    char *sdum;
    struct proctable_Entry *menuProc;

    mydbg(("entering: AnotherError\n"));
    if (MyHost[0] == '\0') gethostname(MyHost, sizeof(MyHost)-1);
    ErrText = errstring;
    if (errstring[0] == '<') {  /* Find and strip message priority, etc. */
	max = strlen(errstring);
	for (i=1; i<max && errstring[i] != '>'; ++i) {
	    ;
	}
	if(i >= max)
	    i = 0;
    } else i = 0;
    char pri[i];
    if(i > 0) {
	    ErrText += i + 1;
	    memcpy(pri, errstring + 1, i);
	    pri[i - 1] = 0;
	    ErrPri = pri;
	    for (sdum=pri; *sdum && *sdum != ':'; ++sdum) { ; }
	    if (*sdum == ':') {
		*sdum++ = '\0';
		if (*sdum) Application = sdum;
	    }
	    if (*sdum) {
		while (*sdum && *sdum != ':') ++sdum;
		if (*sdum == ':') {
		    *sdum++='\0';
		    if (*sdum) LogAddress = sdum;
		}
	    }
	    if (*sdum) {
		while (*sdum && *sdum != ':') ++sdum;
		if (*sdum == ':') {
		    *sdum++='\0';
		}
	    }
	    if ((j=atoi(ErrPri)) > 0 && j < 10) {
		Priority = j;
	    } else {
		if (!lc_strcmp(ErrPri, "critical") || !lc_strcmp(ErrPri, "0")) {
		    Priority = 0;
		} else if (!lc_strcmp(ErrPri, "warning")) {
		    Priority = 4;
		} else if (!lc_strcmp(ErrPri, "monitor")) {
		    Priority = 6;
		} else if (!lc_strcmp(ErrPri, "debug")){
		    Priority = 9;
		} else {
		    ErrText = errstring;
		}
	    }
    }
    /* May eventually want to log everything at high priority with no ID,
     * but for now we require an explicit log address.
     */
    if (LogAddress && *LogAddress) {  
	/* Log everything here */
	FILE *lfp;

	if (*LogAddress == '/') {
	    /* Log by appending to file */

	    sprintf(ErrTxt, "console:  I cannot log anything to file %s", LogAddress);
	    if ((lfp = fopen(LogAddress, "a")) == NULL) {
		ReportInternalError(self, ErrTxt);
	    } else {
		fprintf(lfp, "%s (from %s at %s on %s)\n", 
		    Application ? Application : "Unknown program",  ErrText, 
		    Numbers[CLOCKALL].RawText, MyHost);
		if (fclose(lfp) != 0) {
		    ReportInternalError(self, ErrTxt);
		}
	    }	    
	} else {
	    /* Log by sending mail */
	    sprintf(ErrTxt, "console:  I cannot send logging mail to %s", LogAddress);
	    mailargvector[2] = LogAddress;
	    if ((lfp = qopen(_SITE_QUEUEMAIL, (char **)mailargvector, "w")) == NULL) {
		ReportInternalError(self, ErrTxt);
	    } else {
		fprintf(lfp, "From: console\nTo:%s\nSubject:Console Error Report\n\n%s\n", LogAddress, ErrText);
		if (fclose(lfp) != 0) {
		    ReportInternalError(self, ErrTxt);
		}
	    }
	}
    }
    if (Priority > HighestPriority) {
	return(0);
    }
    if (Priority == 0) {
	if (Application) {
	    snprintf(PrimaryErrorBuffer, ERRBUFSIZE, "ERROR: <%s>%s%s", Application, ErrText, AddNewline ? "\n" : "");
	} else {
	    snprintf(PrimaryErrorBuffer, ERRBUFSIZE, "ERROR: %s%s", ErrText, AddNewline ? "\n" : "");
	}
    } else {
	if (Application) {
	    snprintf(PrimaryErrorBuffer, ERRBUFSIZE, "<%s>%s%s", Application, ErrText, AddNewline ? "\n" : "");
	} else {
	    snprintf(PrimaryErrorBuffer, ERRBUFSIZE, "%s%s", ErrText, AddNewline ? "\n" : "");
	}
    }
    nerr = Numbers[ERRORS].Value + 1;
    if (Numbers[ERRORS].Value > 1000 || Numbers[ERRORS].Value < 0) {
	nerr = 1;
    }
    /*
      arrgh(("Calling NewValue\n"));
      arrgh(("Numbers[ERRORS].RawText = %s\n", Numbers[ERRORS].RawText));
      */
    /*    AddStringToLog(Numbers[ERRORS].RawText, RegionLogs[ERRORREGIONLOG]); */
    NewValue(self, &Numbers[ERRORS], nerr, NULL, TRUE);
    if (IntrnlVars[0].InUse && !IntrnlVars[0].Value) {
	IntrnlVars[0].Value = TRUE;
	(self)->WantUpdate( self);
        if (IntrnlVars[0].turnon) {
            sprintf(ErrTxt, "%s", IntrnlVars[0].turnoff);
	    menuProc = proctable::DefineProc("errmonf-toggle-var", (proctable_fptr)TogVar, ATK::LoadClass("consoleClass"), NULL, "dummy.");
            (self->userMenulist)->AddToML( ErrTxt, menuProc, 0, 0);
            (self->userMenulist)->DeleteFromML( IntrnlVars[0].turnon);
	}
    }
    (self)->PostMenus( self->stdMenulist);
    return 0;
}

/* ****************************************************************
 * Take an all lower case word and a mixed case word and return TRUE
 * if they match, case insensitively. Expectation is usually that it
 * won't match,so the goal is to return FALSE as soon as possible.
 ****************************************************************
 */

