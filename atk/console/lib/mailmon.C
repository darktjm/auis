/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* 
 ***************************************************************
 * Mail system monitoring routine for Instrument Console
 * Also now monitors random directories and print requests
  *****************************************************************
 */

#include <andrewos.h> /* sys/types.h */
#include <andyenv.h>

#include <consoleClass.H>
#include <environment.H>
#include <environ.H>
#include <console.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <sitevars.h>

extern char MyHomeDir[];

static int NewMsgs(FILE  *mf);

NO_DLL_EXPORT boolean UseNonAndrewMail = TRUE;

#ifdef ANDREW_PRINTING_ENV
NO_DLL_EXPORT boolean UseNonAndrewPrint = FALSE;
#else /* ANDREW_PRINTING_ENV */
NO_DLL_EXPORT boolean UseNonAndrewPrint = TRUE;
#endif /* ANDREW_PRINTING_ENV */

NO_DLL_EXPORT boolean CheckMyMail = FALSE;

NO_DLL_EXPORT char OutgoingDir[MAXPATHLEN];
NO_DLL_EXPORT int OutgoingAge = 3600; /* seconds until we notice delayed outgoing mail */

void SetMailEnv()
{
    char *buff;
    int needsFreed = FALSE;

    mydbg(("entering: SetMailEnv\n"));
    buff = getenv("MAIL");
    if (!buff || !*buff){
	buff = (char *)malloc(MAXPATHLEN);
	needsFreed = TRUE;
	if (UseNonAndrewMail){
#ifdef hpux
	    sprintf(buff, "%s/%s", _SITE_NON_ANDREW_MAIL, getenv("LOGNAME"));
#else /* hpux */
	    sprintf(buff, "%s/%s", _SITE_NON_ANDREW_MAIL, getenv("USER"));
#endif /* hpux */
	}
	else{
	    sprintf(buff, "%s/%s", MyHomeDir, _SITE_MAILBOX);
	}
    }
    envmail = strdup(buff);
    if (needsFreed) free(buff);
    CheckMyMail = environ::GetProfileSwitch("console.mailfrom", FALSE);
}


void InitMail(class consoleClass  *self)
{
    const char *s;

    mydbg(("entering: InitMail\n"));
    LastMailMod = 0;
    Numbers[MAIL].RawText = (char *)malloc(40); /* Size of biggest "you have x msgs" string */
    if ((s = environ::GetConfiguration("AMS_NonAMSDelivery")) != NULL){
	if (s[0] == 'y' || s[0] == 'Y' || s[0] == 't' || s[0] == 'T' || s[0] == '1'){
	    UseNonAndrewMail = TRUE;
	}
	else  if (s[0] == 'n' || s[0] == 'N' || s[0] == 'f' || s[0] == 'F' || s[0] == '0') {
	    UseNonAndrewMail = FALSE;
	}
    }
    SetMailEnv();
    if (UseNonAndrewMail) {
        if (envmail[strlen(envmail) - 1] == '/') {
            envmail = NULL;
            ReportInternalError(self, "console: Can't check for Non-Andrew mail when USER is undefined");
        }
        OutgoingDir[0] = '\0';
    }
    else {
        strcpy(OutgoingDir, envmail);
        char *t = strrchr(OutgoingDir, '/');
        if (t) {
            strcpy(++t, ".Outgoing");
	}
	else {
            OutgoingDir[0] = '\0';
        }
    }
}

NO_DLL_EXPORT int LastDirMod[LASTDIRECTORY-DIRECTORY1+2];

void InitDirectories()
{
    int i;

    mydbg(("entering: InitDirectories\n"));
    for (i=0; i<LASTDIRECTORY-DIRECTORY1+2; ++i) {
	LastDirMod[i] = 0;
    }
}

NO_DLL_EXPORT char *PrintDirName;
NO_DLL_EXPORT int PrintDirModTime;

void SetPrintEnv()
{
    char *buff = 0;
    const char *pdir = NULL;
    int needsFreed = FALSE;

    mydbg(("entering: SetPrintEnv\n"));

#ifndef hpux /* **JG Temporary hack: the semantics for PRINTDIR
    on system V does not include the printer name LPDEST */
    pdir = getenv ("PRINTDIR");
#endif /* hpux see also matching `}s' below */

    if (!pdir || !*pdir) {
	pdir = environ::GetProfile("print.printdir");
	if (!pdir || !*pdir) {
	    if (UseNonAndrewPrint) {
#ifdef hpux 
                char *printer_name;
                char *printer_directory;

		printer_name = environ::GetProfile("print.printer");
		if (!printer_name) printer_name = environ::GetProfile("print.spooldir");
		if (NULL == printer_name)
		    printer_name = getenv ("LPDEST");
                if (NULL == printer_name)
                    printer_name = getenv ("PRINTER");
                if (NULL == printer_name)
                    printer_name = "lp";
                printer_directory = environ::GetProfile("print.printdir");
		if (NULL == printer_directory)
		printer_directory = getenv ("PRINTDIR");
               if (printer_directory == NULL)
                    printer_directory = _SITE_NON_ANDREW_PRINTDIR;
		needsFreed = TRUE;
                buff = (char *)malloc (strlen (printer_directory) + strlen
			(printer_name) + 3);
                sprintf (buff, "%s/%s", printer_directory, printer_name);
#else /* hpux  */
		needsFreed = TRUE;
		buff = strdup(_SITE_NON_ANDREW_PRINTDIR);
#endif /* hpux  */
	    }
	    else{
		needsFreed = TRUE;
		buff = (char *)malloc(MAXPATHLEN);
		sprintf (buff, "%s/%s", MyHomeDir, _SITE_PRINTDIR);
	    }
	    pdir = buff;
	}
    }
    PrintDirName = strdup(pdir);
    if (needsFreed) free(buff);
}

void InitPrint(class consoleClass  *self)
    {
    const char *s;

    mydbg(("entering: InitPrint\n"));
    if (!DidInitPrinting) {
	DidInitPrinting = TRUE;
	PrintDirModTime = 0;
	if ((s = environ::GetConfiguration ("print.nonandrew")) != NULL){
	    if (s[0] == 'y' || s[0] == 'Y' || s[0] == 't' || s[0] == 'T' || s[0] == '1'){
		UseNonAndrewPrint = TRUE;
	    }
	    else if (s[0] == 'n' || s[0] == 'N' || s[0] == 'f' || s[0] == 'F' || s[0] == '0') {
	        UseNonAndrewPrint = FALSE;
	    }
	}
	SetPrintEnv();
    }
}


int AbortPrintChecking(class consoleClass  *self, const char  *text)
        {
    mydbg(("entering: AbortPrintChecking\n"));
    ReportInternalError(self, text);
    DoPrintChecking = FALSE;
    Numbers[PRINTQUEUE].Value = -1;
    Numbers[PRINTSENT].Value = -1;
    Numbers[PRINTERRORS].Value = -1;
    PrintDirModTime = 0;
    return(-1);
}

int NonAndrewCheckMail(class consoleClass  *self, char  *Name, int  *LastModTime , int  LastValue)
            {
    FILE *fp;
    struct stat filstatbuf;
    int CurrentValue;

    mydbg(("entering: NonAndrewCheckMail\n"));
    if (stat(Name, &filstatbuf) == -1) {
	if (errno != ENOENT) {
	    sprintf(ErrTxt, "console: Cannot stat mailfile %s, cannot check your mail now.", Name);
	    ReportInternalError(self, ErrTxt);
	    *LastModTime = 0;
	    return(-1);
	}
	else {
	    *LastModTime = 0;
	    return(0);
	}
    }
    if ((filstatbuf.st_mode & S_IFMT) == S_IFDIR) {
	sprintf(ErrTxt, "console:  %s is a directory!  You probably can't receive mail!", Name);
	ReportInternalError(self, ErrTxt);
	*LastModTime = 0;
	return(-1);
    }
    if (filstatbuf.st_mtime == *LastModTime) {
	return(LastValue);
    }
    if ((fp = fopen(Name, "r")) == NULL) {
	sprintf(ErrTxt, "console:  Cannot open file %s", Name);
	ReportInternalError(self, ErrTxt);
	*LastModTime = 0;
	return(-1);
    }

    *LastModTime = filstatbuf.st_mtime;
    CurrentValue = NewMsgs(fp);
    fclose(fp);
    mydbg(("Nr of messages: %d\n", CurrentValue));
    return(CurrentValue);
}

/* Mail checker -- see how many messages in a BSD mail file are new. */

static int NewMsgs(FILE  *mf)
    {
    int LastLineBlank, inhead = 0, newCount;
    boolean EOMChar = FALSE; /* Do I have to check for a specific charater */
    int EndOfMsg = '\003'; /* If so, default to ^C but accept alternates */
    const char *s;

    mydbg(("entering: NewMsgs\n"));
    
    s = environ::GetConfiguration("AMS_DemandSeparatingCharacter");
    if (s != NULL) {
	while (isspace(*s)) ++s;
	if (*s != '\0') {
	    switch(*s) {
	      case 'y': case 'Y': case '1': case 't': case 'T':
		EOMChar = TRUE;
		break;
	      case 'n': case 'N': case '0': case 'f': case 'F':
		EOMChar = FALSE;
		break;
	      default:
		fprintf(stderr, "Bad Boolean value '%s' for AMS_DemandSeparatingCharacter in configuration file; ignored\n", s);
		EOMChar = FALSE;
		break;
	    }

	    if (EOMChar){
		s = environ::GetConfiguration("AMS_SeparatingCharacter");
		if (s != NULL) {
		    while (isspace(*s)) ++s;
		    if (*s != '\0')
			EndOfMsg = atoi(s);
		}
	    }
	}
    }

    newCount = 0;
    LastLineBlank = TRUE;
    
    while (!feof(mf)) {
	char line[100];
	int c;
	char *cp = line;

	/* read a line into our buffer */
	while ((c = getc(mf)) != EOF && c != '\n') {
	    if (cp - line >= (int)sizeof(line) - 1) {
		/* skip over the part that won't fit */
		while ((c = getc(mf)) != EOF && c != '\n');
		break;
	    }
	    *cp++ = c;
	}
	*cp = 0;

	if (LastLineBlank) {
	    if (EOMChar ? (line[0] == EndOfMsg) :
		          (strncmp(line, "From ", 5) == 0)) {
		newCount++;
		inhead = 1;
	    }
	}

	if (*line == '\0') {
	    LastLineBlank = TRUE;
	    inhead = FALSE;
	} else
	    LastLineBlank = FALSE;

	if (inhead && ((cp = strchr(line, ':')) != NULL)) {
	    if (strncmp(line, "Status:", 7) == 0) {
		if (strchr(cp, 'O') != NULL)
		    newCount--;
		inhead = 0;
	    }
	}
    }

    return (newCount);
}
