/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

#include <andrewos.h> /* sys/types.h sys/time.h */
ATK_IMPL("typescript.H")

#include <text.H>
#include <mark.H>
#include <keystate.H>
#include <frame.H>

#include <menulist.H>
#include <proctable.H>
#include <keymap.H>
#include <im.H>
#include <message.H>
#include <msghandler.H>
#include <typetext.H>
#include <simpletext.H>
#include <dataobject.H>
#include <fontdesc.H>
#include <environment.H>
#include <style.H>
#include <stylesheet.H>
#include <environ.H>
#include <print.H>
#include <filetype.H>
#include <typescript.H>

#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef hpux
#include <sys/bsdtty.h>
#include <sys/ptyio.h>
#endif /* hpux */

#ifdef M_UNIX
#include <sys/termio.h>
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#include <termios.h>

#include <signal.h>

#if defined(POSIX_ENV)  && (!defined(sun) || defined(SOLARIS))
#undef USE_TERMIOS
#define USE_TERMIOS 1
#endif

#ifdef SOLARIS
#include <sgtty.h>
#endif

#ifdef USE_TERMIOS
#ifndef SOLARIS
#define tcgetattr tcgetattr_hohum
#define tcsetattr tcsetattr_hohum
#endif
#include <termios.h>
#ifndef SOLARIS
#undef tcgetattr
#undef tcsetattr
#endif
#if !defined(linux) && !defined(SOLARIS)
extern "C" {
extern int tcgetattr(int fildes, struct termios *termios_p);
extern int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p);
}
#endif
#else
#if SY_AIX221
#include <sys/devinfo.h>
#include <sys/pty.h>
#include <sys/termio.h>
#include <sys/tty.h>
#else /* #if SY_AIX221 */
#include <sgtty.h>
#endif /* #if SY_AIX221 */
#endif /* defined(POSIX_ENV) && !defined(sun) */

#if SY_AIX12
static char io_buffer[4096];
#endif /* SY_AIX12 */

/*
#if SY_AIX31 || SY_AIX31
#define UTMP_MUNGE 1
#endif
*/

#ifdef UTMP_MUNGE
#include <utmp.h>
#endif

#include <util.h>

#define PRINTER_SETUP_DIALOG_ENV 1 /* used to be in contrib, but now we should always use it. */

/* #define DONTCUTSTYLES */
#define MyEnvinfo(text,pos) ((class environment *)(text->rootEnvironment)->GetInnerMost(pos))
#define TEXT(A) ((class text*)(A->dataobject))
#define TEXTOBJ(A) A->dataobject
#define ISPIPESCRIPT(self) (self->pipescript)

static const char *DefaultArgList[] = { 0, 0 };
/* #define SENDRAW 1 */

static const char **myarglist = NULL;
static boolean Pipescript = FALSE;
static class style *staticBoldStyle = NULL;
static class keymap *ssmap;
static class menulist *typescriptMenus;
static FILE *df = NULL;
static FILE *odf = NULL;


	




/* The following are initialized from the proctable by our InitializeClass
 * routine. There may well be a better way of doing this. The routines
 * should certainly be void instead of int, but the proctable package
 * is screwed. */
static proctable_fptr textview_EndOfTextCmd;
static proctable_fptr textview_SelfInsertCmd;
static proctable_fptr textview_DigitCmd;
static proctable_fptr textview_BeginningOfLineCmd;
static proctable_fptr textview_EndOfLineCmd;
static proctable_fptr textview_YankCmd;
static proctable_fptr textview_BackwardsRotatePasteCmd;
static proctable_fptr textview_RotatePasteCmd;
static proctable_fptr textview_DeleteCmd;
static proctable_fptr textview_RuboutCmd;
static proctable_fptr textview_LineToTop;
static proctable_fptr typescript_CopyRegionCmd; 
static proctable_fptr typescript_ZapRegionCmd; 
#ifdef PRINTER_SETUP_DIALOG_ENV
static proctable_fptr typescript_poptPostWindow = NULL;
#endif

static long maxSize;
static long extraRoom;
static char *cmd;
static int CmdSize;
static boolean FileMenu = FALSE;
#define	SetCmdSize(A) if(A>CmdSize) cmd = (char*)realloc(cmd,(CmdSize = 64 + A))


ATKdefineRegistry(typescript, textview, typescript::InitializeClass);

static void typescriptAddMenu(char  *nbuf, struct proctable_Entry  *proc);
static void typescript_PreviewCmd(class typescript  *self);
static void typescript_PrintCmd(class typescript  *self);
static void doprint(class typescript  *self, int  porp);
static void typescript_SaveAs(class typescript  *self);
static void typescript_SetPrinterCmd(class typescript  *self);
static void typescript_PrintOptionsCmd(class typescript  *self);
static void typescriptAddSearchMenu();
static void typescriptAddFileMenu();
static void AnounceDeath(class typescript  *self);
static class environment * GetCommandEnv(class typescript  *self, long  pos, long  *start , long  *end);
static void  typescript_RuboutCmd(class typescript  *self);
static void MaintainLastEnv(class typescript  *td );
void SaveCommand(class typescript  *td);
static void TypescriptLeftCommand(class typescript  *tsv );
static void  TypescriptEndOfLineCommand(class typescript  *tsv );
static void TypescriptEOTCommand(class typescript    *tv );
static void TypescriptINTCommand(class typescript  *tv );
static void TypescriptSTOPCommand (class typescript  *tv );
static void TypescriptQUITCommand (class typescript  *tv );
static void SendSig (class typescript  *tv , int sig);
static void  TypescriptUnboundCommand(class typescript  *tv );
static void  smashReadOnlyBuf(class typescript  *tv );
static void TypescriptDoReturnCommand (class typescript  *tv,long  endpos);
static void TypescriptReturnCommand (class typescript  *tv );
static void  TypescriptReturnAndPositionCommand(class typescript  *self, long  data);
static void typescript_HandleMenus(class typescript  *self, long  data);
static void TypescriptZapCommand(class typescript  *tv);
static void GrabCommand(class typescript    *tv, class text  *fromText, long  start, long  end);
static void GrabLastCommand (class typescript  *tv);
static void GrabNextCommand (class typescript    *tv);
static void GrabCurrentCommand(class typescript  *tv );
static void ExecuteCurrentCommand(class typescript  *tv );
static void  SetTitle(class typescript  *self, const char  *titleLine);
static char *  ReadDirName(class typescript  *self, FILE  *f, char  *buf, int  *bufsiz);
static void ReadFromProcess(FILE  *f, class typescript  *td);
static void ClearTypescriptText(class typescript  *tv);
static void ClearTypescript(class typescript  *tv);
static void NoEchoCommand(class typescript  *tv);
static void ResetTTY(int  fd	    /* file descriptor for the tty */);
static void  MyCanOutHandler(FILE  *afile, class typescript  *ad );
static void  typescript_handlereadonly(class typescript  *self ,char  c);
static void PositionDot(class typescript  *self);
static void  typescript_YankCmd(class typescript  *self);
static void  Typescript_SelfInsertCmd(class typescript  *self, long  a);
static void  Typescript_DigitCmd(class typescript  *self, long  a);
static void typescript_BackwardsRotatePasteCmd(class typescript  *self);
static void typescript_RotatePasteCmd(class typescript  *self);
void  typescript_NoTextviewKey(class typescript  *self, long  key);
static void typescript_DragCwdCmd(class typescript  *self);
static void typescript_ResetTTY(class typescript  *self);
static void typescriptAddtypescriptMenus();
#ifdef TIOCGWINSZ
static void NullWinSizeProc();
#endif /* TIOCGWINSZ */
static int  WritePty(class typescript  *tv, char  *buf, int  len);
#ifdef UTMP_MUNGE
void  utmp_add(char  *ptyname, int  pid);
void utmp_delete(char  *ptyname);
#endif


static void
typescriptAddMenu(char  *nbuf, struct proctable_Entry  *proc)
{
    char *c, *bf, *cp, *retstr;

    retstr = NULL;
    bf = (char*) malloc(strlen(nbuf) + 1);
    for(c = bf,cp = nbuf; *cp != '\0'; cp++ ,c++) {
	if(*cp == ':') {
	    *c = '\0';
	    retstr = c + 1;
	}
	else 
	    *c = *cp;
    }
    *c = '\0';
    if(retstr && *retstr)
	(typescriptMenus)->AddToML( bf, proc, (long)retstr, 0);
    else free(bf);
}
    
static void
typescript_PreviewCmd(class typescript  *self)
{
    int pmode;
#ifdef PSPRINTING_ENV
    pmode = print_PREVIEWPOSTSCRIPT;
#else
    pmode = print_PREVIEWTROFF;
#endif
    doprint(self, pmode);
}

static void
typescript_PrintCmd(class typescript  *self)
{
    int pmode;
#ifdef PSPRINTING_ENV
    pmode = print_PRINTPOSTSCRIPT;
#else
    pmode = print_PRINTTROFF;
#endif
    doprint(self, pmode);
}

static void
doprint(class typescript  *self, int  porp)
{
    message::DisplayString(self, 0, "Processing request.");
    im::ForceUpdate();
    if(ATK::LoadClass("print") == NULL) {
	message::DisplayString(self, 0, "Print aborted: could not load class \"print\".");
        return;
    }
    print::ProcessView((class view *)self, porp, 1, "pipescript", "");
    if (porp)
	message::DisplayString(self, 0, "Print request submitted; watch console for results.");
    else
	message::DisplayString(self, 0, "Preview window should appear soon.");
}

static void
typescript_SaveAs(class typescript  *self)
{
    char frs[256], mes[256];
    FILE *f;
    int failed = 0;

    if(message::AskForString(self, 0, "File Name: ", "", frs, 256) == -1) 
	return;
    filetype::CanonicalizeFilename(mes, frs, 256);
    if((f = fopen(mes, "w")) == NULL) 
	failed++;
    else {
	(TEXT(self))->Write( f, im::GetWriteID(), 0); 
	if(fclose(f) < 0) 
	    failed++;
    }
    if(failed) {
	sprintf(frs, "Can't write %s", mes);
	message::DisplayString(self, 0, frs);
	return;
    }
    sprintf(frs, "wrote %s", mes);
    message::DisplayString(self, 0, frs);
}

static void
typescript_PrintOptionsCmd(class typescript  *self)
{
    if (typescript_poptPostWindow)
	(*typescript_poptPostWindow) (self, 0);
}

static void
typescript_SetPrinterCmd(class typescript  *self)
{
    class msghandler *messageLine = (class msghandler *) (self)->WantHandler( "message");
    char answer[256], prompt[sizeof("Current printer is . Set printer to []: ") + 128];
    const char *currentPrinter, *defaultPrinter;

    if(messageLine == NULL)
        return;
    currentPrinter = environ::GetPrinter();
    defaultPrinter = environ::GetProfile("print.printer");
    if (!defaultPrinter) defaultPrinter = environ::GetProfile("print.spooldir");
    if(currentPrinter && defaultPrinter)
        sprintf(prompt, "Current printer is %.64s. Set printer to [%.64s]: ", currentPrinter, defaultPrinter);
    else if(defaultPrinter)
        sprintf(prompt, "Set printer to [%.64s]: ", defaultPrinter);
    else
        strcpy(prompt, "Set printer to: ");
    if((messageLine)->AskForString( 0, prompt, NULL, answer, sizeof(answer)) == -1)
        return;
    if(*answer != '\0') {
        environ::PutPrinter(answer);
        defaultPrinter = answer;
    }
    else 
	environ::DeletePrinter();
    if(defaultPrinter) {
        sprintf(prompt, "Printer set to %.64s.", defaultPrinter);
        (messageLine)->DisplayString( 0, prompt);
    }
    else
        (messageLine)->DisplayString( 0, "Printer not set.");
}

static void
typescriptAddSearchMenu()
{
    struct proctable_Entry *tempProc;

    if((tempProc = proctable::Lookup("textview-search")))
        (typescriptMenus)->AddToML( "Search/Spell~20,Forward~10", tempProc, 0, 0);

    if((tempProc = proctable::Lookup("textview-reverse-search")))
        (typescriptMenus)->AddToML( "Search/Spell~20,Backward~11", tempProc, 0, 0);
}


static void
typescriptAddFileMenu()
{
    struct proctable_Entry *tempProc;
    struct ATKregistryEntry  *classInfo = &typescript_ATKregistry_ ;

    tempProc=proctable::DefineProc("typescript-Save-As", (proctable_fptr)typescript_SaveAs,  classInfo, NULL, "Prompt for file name to save");
    (typescriptMenus)->AddToML( "File~30,Save As~10", tempProc, 0, 0);

#ifdef RCH_ENV
    tempProc = proctable::DefineProc("rchprint-print", NULL, classInfo, "rchprint", NULL);
    if (tempProc != NULL) (typescriptMenus)->AddToML( "File~30,Print...~20", tempProc, 0, 0);
    tempProc = proctable::DefineProc("rchprint-preview", NULL, classInfo, "rchprint", NULL);
    if (tempProc != NULL) (typescriptMenus)->AddToML( "File~30,Preview...~21", tempProc, 0, 0);
    tempProc=proctable::DefineProc("typescript-Print", (proctable_fptr)typescript_PrintCmd, classInfo, NULL, "Print typescript");
#else
    tempProc=proctable::DefineProc("typescript-Print", (proctable_fptr)typescript_PrintCmd, classInfo, NULL, "Print typescript");
    (typescriptMenus)->AddToML( "File~30,Print~21", tempProc, 0, 0);

    tempProc=proctable::DefineProc("typescript-Preview", (proctable_fptr)typescript_PreviewCmd, classInfo, NULL, "Preview typescript");
    (typescriptMenus)->AddToML( "File~30,Preview~22", tempProc, 0, 0);

    tempProc = proctable::DefineProc("typescript-SetPrinter", (proctable_fptr)typescript_SetPrinterCmd, classInfo, NULL, "Set Printer");
#ifdef PRINTER_SETUP_DIALOG_ENV
    tempProc = proctable::DefineProc("typescript-post-window", (proctable_fptr)typescript_PrintOptionsCmd, classInfo, NULL, "Set print options");
    (typescriptMenus)->AddToML( "File~30,Printer Setup~20", tempProc, 0, 0);
#else
    (typescriptMenus)->AddToML( "File~30,Set Printer~20", tempProc, 0, 0);
#endif /* PRINTER_SETUP_DIALOG_ENV */
#endif
}

static void
AnounceDeath(class typescript  *self)
{
    char buf[512];
    const char *shell = environ::Get("SHELL");

    if(shell == NULL) 
	shell = "/bin/sh";
    if(ISPIPESCRIPT(self)) {
	sprintf(buf, "EOF on pipe");
	message::DisplayString(self, 15, buf);
    }
    else if(strcmp(self->progname,shell) == 0) {
	sprintf(buf,"This process died. Start new typescript to continue work.");
	message::DisplayString(self, 100, buf);
    }
    else {
	sprintf(buf,"The %s process died.",self->progname);
	message::DisplayString(self, 100, buf);
    }    
}

static class environment *
GetCommandEnv(class typescript  *self, long  pos, long  *start , long  *end)
{
    class environment *te;

    te = MyEnvinfo(TEXT(self), pos);
    if((te->data.style != staticBoldStyle) && (pos > 0)) {
	/* cursor may be at end of environment */
	te = MyEnvinfo(TEXT(self), pos - 1);
    }
    if(te->data.style != staticBoldStyle)
	return NULL;
    /* here we are in the command's bold region */
    *start = (te)->Eval();
    *end =  *start + te->length;
    return te;
}

static void 
typescript_RuboutCmd(class typescript  *self)
{
    if(self->readOnlyLen != -1) {
#ifdef SENDRAW
	write(self->SubChannel, "\010" , 1);
#else
	if(self->readOnlyLen > 0) 
	    self->readOnlyLen--;
#endif
    }
    else if((self)->GetDotPosition() + (self)->GetDotLength() > (TEXT(self))->GetFence())
	textview_RuboutCmd((class textview *)self, 0);
}

class typescript *
typescript::Create(const char  **arglist, FILE  *diskf, boolean  filemenu)
{
	ATKinit;

    class typescript *self;
    class typetext *tt;
    if(arglist == NULL) Pipescript = TRUE;
    else if(*arglist) myarglist = arglist;
    df = diskf;
    FileMenu = filemenu;
    tt = new typetext;
    self = new typescript;
    (self)->SetDataObject( (class dataobject *)tt);
    return(self);
}

class typescript *
typescript::CreatePipescript(FILE  *indiskf, FILE  *outdiskf, boolean  filemenu)
{
	ATKinit;

    class typescript *self;

    self = typescript::Create(NULL, indiskf, filemenu);
    odf = outdiskf;
    return self;
}

static void
MaintainLastEnv(class typescript  *td )
{
    long len, spos;
    class text *mydoc;
    class environment *te;

    mydoc =  TEXT(td);
    spos = (td->cmdStart)->GetPos();
    len = (mydoc)->GetLength();
    if(spos < len) {
	te = MyEnvinfo(mydoc, spos);
	if(te->data.style != staticBoldStyle)
	    te = (mydoc->rootEnvironment)->InsertStyle( spos, staticBoldStyle, TRUE);
        (te)->SetStyle( FALSE, FALSE);
	(te)->SetLength( len - spos);
   }

}

void
SaveCommand(class typescript  *td)
{
    long len, spos;
    class text *mydoc;
    char *textBuf;
    long retLen;

    mydoc =  TEXT(td);
    spos = (td->cmdStart)->GetPos();
    len = (mydoc)->GetLength();
    if(spos < len) {
	while(spos < len) {
	    textBuf = (mydoc)->GetBuf( spos, len - spos, &retLen);
	    if(textBuf) {
		(td->cmdText)->InsertCharacters( (td->cmdText)->GetLength(), textBuf, retLen);
		spos += retLen;
	    }
	    else break;
	}
	(td->cmdText)->InsertCharacters( (td->cmdText)->GetLength(), "\n", 1);
    }
}

static void
TypescriptLeftCommand(class typescript  *tsv )
{
    long pos;
    long start,end;

    pos = (tsv)->GetDotPosition();
    if((tsv)->GetDotLength()== 0) {
	if((tsv->cmdStart)->GetPos() == pos) 
	    return;
	if(GetCommandEnv(tsv,pos,&start,&end) != NULL) {
	    if(start == pos) 
		return;
	    (tsv)->SetDotPosition( start);
	    (tsv)->WantUpdate( tsv);
	    return;
	}
    }
    textview_BeginningOfLineCmd((class textview *)tsv, 0);
}

static void 
TypescriptEndOfLineCommand(class typescript  *tsv )
{
    long pos;
    long start,end;

    pos = (tsv)->GetDotPosition();
    if((tsv)->GetDotLength() == 0 && GetCommandEnv(tsv, pos, &start, &end) != NULL) {
	if(end == pos) 
	    return;
	(tsv)->SetDotPosition( end);
	(tsv)->WantUpdate( tsv);
    }
    else 
	textview_EndOfLineCmd((class textview *)tsv, 0);
}

static void
TypescriptEOTCommand(class typescript    *tv )
{
    static struct timeval t = { 0, 0 };

    fd_set wfds;


    if(tv->SubChannel < 0) 
	return;
    
    FD_ZERO(&wfds);
    FD_SET(tv->SubChannel, &wfds);
    select(tv->SubChannel+1, 0, &wfds, 0, &t);
    if(!FD_ISSET(tv->SubChannel, &wfds)) {
	message::DisplayString(tv, 0, "Process not ready for input");
	return;
    }	
    if((tv)->GetDotPosition() >= (TEXT(tv))->GetLength()) {
#ifdef USE_TERMIOS
	/* non-SunOS POSIX pty is in cooked mode.  Give it an EOF char.
	 * XXX - this distinction needs to be more precise; POSIX systems
	 * differ fairly strongly in their pty behaviors, and System V
	 * Release 4 and 4.4BSD probably behave more like SunOS.
	 */
    {
	struct termios tios;
	tcgetattr(tv->SubChannel, &tios);
	WritePty(tv, (char *)&tios.c_cc[VEOF], 1);
    }
#else /* defined(POSIX_ENV) && !defined(sun) */
	WritePty(tv, "", 0);
#endif /* defined(POSIX_ENV) && !defined(sun) */
    }
    else
	textview_DeleteCmd(tv, 0);
}

static void
TypescriptINTCommand(class typescript  *tv )
{
    SendSig(tv, SIGINT);
}

static void
TypescriptSTOPCommand (class typescript  *tv )
{
#if SY_AIX221
/* %%%%%%  must changed if AIX supports the STOP signal */
    SendSig(tv, 0);
#else /* if SY_AIX221 */
#if SY_OS2
    SendSig(tv, 0);
#else /* if SY_OS2*/
    SendSig(tv, SIGTSTP);
#endif /* if SY_OS2*/
#endif /* if SY_AIX221 */
}

static void
TypescriptQUITCommand (class typescript  *tv )
{
    SendSig(tv, SIGQUIT);
}

static void
SendSig (class typescript  *tv , int sig) 
{
#ifdef USE_TERMIOS
/* The non-SunOS POSIX pty is in cooked mode, so query the
 * pty for signal chars, then write that signal
 * char to the pty.  The pty will deliver the
 * signal.
 * XXX - this distinction needs to be more precise; POSIX systems
 * differ fairly strongly in their pty behaviors, and System V
 * Release 4 and 4.4BSD probably behave more like SunOS.
 */
    struct termios tios;
    char intchar;

    /* Get interrupt chars each time in case the user mucks with them. */
    tcgetattr(tv->SubChannel, &tios);
    switch(sig) {
        case SIGINT:
            intchar = tios.c_cc[VINTR];
            break;
        case SIGQUIT:
            intchar = tios.c_cc[VQUIT];
	    break;
#if !SY_OS2
        case SIGTSTP:
            intchar = tios.c_cc[VSUSP];
	    break;
#endif
        default:
            return;
	}
    WritePty(tv, (char *)&intchar, 1);
    return;
#else /* defined(POSIX_ENV) && !defined(sun) */
    int pgrp = 0;
    if(tv->SubChannel < 0) 
	return;
#if SY_AIX221
    pgrp = tv->pgrpid;	/* get saved process group */
#else
    ioctl(tv->SubChannel, TIOCGPGRP, &pgrp);
#endif
    if(pgrp == 0)
	message::DisplayString(tv, 0, "Can't send signal to subprocess");
    else 
	killpg(pgrp, sig);
    return;
#endif /* defined(POSIX_ENV) && !defined(sun) */
}

static void 
TypescriptUnboundCommand(class typescript  *tv )
{
}

static void 
smashReadOnlyBuf(class typescript  *tv )
{   /* Clear out the buf when no longer needed since it probably contains a password */
    char *c;
    int i = READONLYMAX;

    for(c = tv->readOnlyBuf; i > 0; i-- )
	*c++ = 'X';
}

static void
TypescriptDoReturnCommand (class typescript  *tv,long  endpos)
{
    class text *d;
    int maxpos, vfp;
    fd_set wfds;
    int stpos, len;
    static struct timeval t = { 0, 0};

    if(tv->SubChannel < 0) {
	AnounceDeath(tv);
	return;
    }
    MaintainLastEnv(tv);
    SaveCommand(tv);
    tv->lastCmdPos = (tv->cmdText)->GetLength();

    FD_ZERO(&wfds);
    FD_SET(1 << tv->SubChannel, &wfds);
    d = TEXT(tv);
    maxpos = (d)->GetLength();
    select(tv->SubChannel+1, 0, &wfds, 0, &t);
    if(!FD_ISSET(tv->SubChannel, &wfds) || tv->OutputWait) {
	if(!tv->OutputWait) {
	    /* add an output handler */
	    im::AddCanOutHandler(tv->SCFile, (im_filefptr) MyCanOutHandler, (char*) tv, 6);
	    tv->OutputWait = 1;
	}
	/* force newline addition to end of input during typeahead,
	 as it is forced in the normal case */
	(d)->InsertCharacters( maxpos, "\n", 1);
	(tv->cmdStart)->SetPos( maxpos + 1);
	(tv->cmdStart)->SetLength( 0);
	if (endpos < 0)
	    (tv)->SetDotPosition(maxpos + 1);
	else
	    (tv)->SetDotPosition(endpos);
	(tv)->SetDotLength( 0);
	(tv)->FrameDot( maxpos + 1);
	return;
    }	

    /* compute end of stuff to send */
    if(maxpos > (vfp = (tv->cmdStart)->GetPos())) {
	stpos = vfp;
	len = maxpos - stpos;
	if(len < 0)
	    len = 0;
    }
    else {
	stpos = maxpos;
	len = 0;
    }
    SetCmdSize(len + 1);

#ifdef SENDRAW
    if(tv->readOnlyLen == 0)
	write(tv->SubChannel,"\n",1);
#else
    if(tv->readOnlyLen > 0) {
        if(len > 0) {
	    (d)->CopySubString( stpos, len , cmd, FALSE);
            WritePty(tv, cmd, len);
	}
        tv->readOnlyBuf[tv->readOnlyLen] = '\n';
        WritePty(tv, tv->readOnlyBuf, tv->readOnlyLen + 1);
        (d)->InsertCharacters ( stpos + len++, "\n", 1);
	smashReadOnlyBuf(tv);
    }
#endif
    else {
        (d)->InsertCharacters( stpos + len++, "\n", 1);
	(d)->CopySubString( stpos, len, cmd, FALSE);
	if(len < 100)
            WritePty(tv, cmd , len);
	else {
	    int tl;
	    char *cc;
	    for(cc = cmd,tl = len; tl >= 100; cc += 100, tl -= 100)
		WritePty(tv, cc , 100);
	    if(tl > 0)
		WritePty(tv, cc , tl);
	}
    }
    tv->readOnlyLen = -1;
    stpos += len;
    if(endpos < 0)
	(tv)->SetDotPosition( stpos);
    else
	(tv)->SetDotPosition( endpos);
    (tv)->SetDotLength( 0);
    tv->lastCmdPos = (tv->cmdText)->GetLength();
    (tv->cmdStart)->SetPos( stpos);
    (tv->cmdStart)->SetLength( 0);
    (d)->SetFence( stpos);
    (tv)->FrameDot( stpos);
    (d )->NotifyObservers( 0);
    if((d)->GetLength() > maxSize)
	((class typetext*) d)->AlwaysDeleteCharacters( 0, (d)->GetLength() - maxSize + extraRoom);
}

static void
TypescriptReturnCommand (class typescript  *tv )
{
    TypescriptDoReturnCommand(tv, -1);
}

static void 
TypescriptReturnAndPositionCommand(class typescript  *self, long  data)
{
    TypescriptDoReturnCommand(self, (TEXT(self))->GetFence());
    textview_LineToTop((class textview*)self, data); 
} 

static void
typescript_HandleMenus(class typescript  *self, long  data)
{
    char *s = (char*) data;

    while(*s) {
	if((*s == '\n') || (*s == '\r')) 
	    TypescriptReturnCommand(self);
	else
	    Typescript_SelfInsertCmd(self, *s);
	s++;
    }
}

static void
TypescriptZapCommand(class typescript  *tv)
{
    class text *d;
    int maxpos;
    int stpos;
#ifdef SENDRAW
    if(tv->readOnlyLen == 0) {
	write(tv->SubChannel,"\025",1);
	return;
    }
#else
    if(tv->readOnlyLen != -1) {
        tv->readOnlyLen = 0;
        return;
    }
#endif
    d = TEXT(tv);
    maxpos = (d)->GetLength();
    stpos = (tv->cmdStart)->GetPos();
    if(maxpos > stpos) {
	(d)->DeleteCharacters( stpos, maxpos - stpos);
	(tv)->SetDotPosition( stpos);
	(tv)->SetDotLength( 0);
	tv->lastCmdPos = (tv->cmdText)->GetLength();
    }
    (d )->NotifyObservers( 0); 
}

static void
GrabCommand(class typescript    *tv, class text  *fromText, long  start, long  end)
{
    long size, len, pos;

    size = end - start;
    /* now copy the bits out */
    SetCmdSize(size);
    (fromText)->CopySubString( start, size, cmd, FALSE);
    if((len = (tv)->GetDotLength()) > 0) {
	/* deletecharacters will check the fence for us */
	(TEXT(tv))->DeleteCharacters((tv)->GetDotPosition(),len);
    }
    pos = (TEXT(tv))->GetLength();
    (TEXT(tv))->InsertCharacters( pos, cmd, size);
    (tv)->SetDotPosition( pos);
    (tv)->SetDotLength( size);
    (tv)->FrameDot( pos);
    (TEXT(tv))->NotifyObservers( 0);
}

static void
GrabLastCommand (class typescript  *tv)
{
    long cmdEnd;

    if(tv->lastCmdPos == 0)
	return;
    cmdEnd = tv->lastCmdPos - 1;
    tv->lastCmdPos = (tv->cmdText)->GetBeginningOfLine( cmdEnd);
    GrabCommand(tv, tv->cmdText, tv->lastCmdPos, cmdEnd);
}

static void
GrabNextCommand (class typescript    *tv)
{
    long cmdEnd;

    if(tv->lastCmdPos >= (tv->cmdText)->GetLength())
	return;
    tv->lastCmdPos = (tv->cmdText)->GetEndOfLine( tv->lastCmdPos) + 1;
    cmdEnd = (tv->cmdText)->GetEndOfLine( tv->lastCmdPos);
    if(tv->lastCmdPos != cmdEnd)
	GrabCommand(tv, tv->cmdText, tv->lastCmdPos, cmdEnd);
}

static void
GrabCurrentCommand(class typescript  *tv )
{
    int i;
    class text *d;
    class environment *te;
    int dlen;
    long lineBegin, lineEnd;

    d = TEXT(tv);
    dlen = (d)->GetLength();
    if((i = (tv)->GetDotLength()) == 0) {
	lineEnd = (d)->GetEndOfLine( (tv)->GetDotPosition());
	lineBegin = (d)->GetBeginningOfLine( lineEnd);

	for(i = lineBegin; i < lineEnd; i++) {
	    te = MyEnvinfo(d, i);
	    if(te->data.style == staticBoldStyle) {
		long start, size;

		/* here we are in the command's bold region */
		start = (te)->Eval();
		size = te->length;
		GrabCommand(tv, d, start, start + size);
		return;
	    }
	}
    }
    else {
	/* i still has the length */
	lineBegin = (tv)->GetDotPosition();
	lineEnd = lineBegin + i;
    }
    /* now send from lineBegin to lineEnd inclusive */
    SetCmdSize(lineEnd - lineBegin);
    (d)->CopySubString( lineBegin, lineEnd - lineBegin , cmd, FALSE);
    (d)->InsertCharacters( dlen, cmd, i = lineEnd - lineBegin);
    (tv)->SetDotPosition( dlen);
    (tv)->SetDotLength( i);
    (tv)->FrameDot( dlen);
    (d)->NotifyObservers( 0);
    (tv )->PostMenus( tv->menulistp);
    /* MaintainLastEnv(tv); */
}

static void
ExecuteCurrentCommand(class typescript  *tv )
{
    if((tv)->GetDotPosition() < (TEXT(tv))->GetFence())
	GrabCurrentCommand(tv);
    TypescriptReturnCommand (tv);
}

static void 
SetTitle(class typescript  *self, const char  *titleLine)
{

#define WMTITLELEN 70 /* Can you say "Magic hack?" */

    char titleBuffer[1000];
    int len, maxLen = WMTITLELEN - 1;
    char *tb;

    *titleBuffer = '\0';

    tb = titleBuffer;
    if(self->title) {
	len = strlen(self->title);
	if(len < WMTITLELEN - 3) {
	    strcpy(titleBuffer, self->title);
	    strcat(titleBuffer, ": ");
	    maxLen -= (len + 2);
	    tb = &titleBuffer[len + 2];
	}
	else {
	    strcpy(titleBuffer, self->title);
	    titleLine = NULL;
	}
    }
		
    if(titleLine) {
	const char *home = environ::Get("HOME");

	if(home) {
	    int hlen = strlen(home);
	    if(strncmp(titleLine,home,hlen) == 0) {
		strcpy(tb,"~");
		--maxLen;
		titleLine += hlen;
	    }
	}		    
	len = strlen(titleLine);
	if(len > maxLen) {
	    const char *partialName;

	    maxLen -= sizeof("---") - 1;
	    partialName = strchr(titleLine + (len - maxLen), '/');
	    if(partialName == NULL)
		partialName = titleLine + (len - maxLen);
	    else
		++partialName; /* Skip slash... */
	    strcpy(tb, "---");
	    strcat(tb, partialName);
	}
	else
	    strcat(tb, titleLine);
    }
    if (self->frame)
	(self->frame)->SetTitle( titleBuffer);
    else
	((self)->GetIM())->SetTitle( titleBuffer); 
}

#define StartMagicChar 1 /* Ctrl A */
#define EndMagicChar 2  /* Ctrl B */

static char * 
ReadDirName(class typescript  *self, FILE  *f, char  *buf, int  *bufsiz)
{
    char *cp;
    int c;
    int i = *bufsiz;
    for(cp = buf; --i && FILE_HAS_IO(f) > 0; cp++) {
	if((c = getc(f)) == EOF) {
	    im::RemoveFileHandler(f);
	    self->SubChannel = -1;
	    AnounceDeath(self);
	    break;
	}
	if((*cp = c) == '\n') {
	    cp++;
	    break;
	}
	if(*cp == EndMagicChar) {
	    *cp = '\0';
	    if(im::ChangeDirectory(buf) == 0)
		::SetTitle(self,buf);
	    return(NULL);
	}
    }
    *bufsiz = i;
    return(cp);
}

static void
ReadFromProcess(FILE  *f, class typescript  *td)
{
    char buf[4000];
    char *bp = buf;
    long dotpos, vfp;
    int c = getc(f), i = 3999;
    int reframe = 0;
    char *input;
    int cpos;
    class text *d = TEXT(td);

    (td->cmdStart)->IncludeBeginning() = FALSE;
    dotpos = (td)->GetDotPosition();
    if(td->lastPosition == -1)
	td->lastPosition = dotpos;
     if(c != EOF) {
	 if(odf) 
	     putc(c, odf);
	if(((c == StartMagicChar) || (c == EndMagicChar)) && !td->pipescript) {
	    cpos = i;
	    if((input = ReadDirName(td, f, bp, &cpos))) {
		bp = input;
		i = cpos;
	    }
	}
	else if(c != '\015') 
	    *bp++ = c;
        while(--i && FILE_HAS_IO(f) > 0) 
	{
	    if((c = getc(f)) == EOF) 
		break;
	    if(odf) 
		putc(c, odf);
            *bp = c;
	    if(((*bp == StartMagicChar) || (*bp == EndMagicChar)) && !td->pipescript) {
		cpos = i;
		if((input = ReadDirName(td, f, bp, &cpos))) {
		    bp = input;
		    i = cpos;
		    if(i == 0) 
			break;
			 /* we could lose some output this way, but only if someone
			is cat-ing a binary or something else bogus.
			Even then , I doubt the reads will be this big. */
		}
	    }
	    else if(*bp != '\015') 
		bp++;
        }
	if(bp != buf) {
	    vfp = (d)->GetFence();
	    if(vfp <= dotpos) {
		dotpos += bp - buf;
		reframe++;
	    }
	    (d)->InsertCharacters ( vfp, buf, bp - buf);
	    (d)->SetFence( vfp = vfp + (bp - buf));
	    if((td->cmdStart)->GetPos() < vfp) {
		(td->cmdStart)->SetPos( vfp);
		(td->cmdStart)->SetLength( 0);
	    }
	    if(reframe) {
		(td)->SetDotPosition(dotpos); 
/*		typescript_FrameDot(td,dotpos); */	
	    }
	    (d)->NotifyObservers( 0);
	}
#ifdef USE_TERMIOS
	{
	    struct termios tios;

	    tcgetattr(td->SlaveChannel, &tios);
	    if(tios.c_lflag & ECHO)
		td->readOnlyLen = -1;	/* echo should be on */
	    else if(td->readOnlyLen == -1)
		td->readOnlyLen = 0;	/* no echo just started */
	}
#else /* defined(POSIX_ENV) && !defined(sun) */
#if (!vax | vax2) && (!defined(M_UNIX)) /* everything but vax_11 */ /* this prevents vax from inserting chars in doc ! */
        {
#if SY_AIX221
            struct termio ThisTerm;

            ioctl(fileno(f), TCGETA, &ThisTerm);
            if(ThisTerm.c_lflag & ECHO)
		td->readOnlyLen = -1;	/* echo should be on */
	    else if(td->readOnlyLen == -1)
                td->readOnlyLen = 0;	/* noecho mode just started... */
#else /* #if SY_AIX221 */
            struct sgttyb sgttyb;

            ioctl(fileno(f), TIOCGETP, &sgttyb);
            if(sgttyb.sg_flags & ECHO)
		td->readOnlyLen = -1;	/* echo should be on */
	    else if(td->readOnlyLen == -1)
                td->readOnlyLen = 0;	/* noecho mode just started... */
#endif /* #if SY_AIX221 */
	}
#endif /* (!vax | vax2)   */
#endif /* defined(POSIX_ENV) && !defined(sun) */
     }
    if(c == EOF) {
	if(odf) fclose(odf);
        im::RemoveFileHandler(f);
	td->SubChannel = -1;
	AnounceDeath(td);
    }
    (td->cmdStart)->IncludeBeginning() = TRUE;
}

static void
ClearTypescriptText(class typescript  *tv)
{
    class text *d = TEXT(tv);
    int p;

    p = (d)->GetBeginningOfLine( (tv->cmdStart)->GetPos());
    if(p != 0) {
	((class typetext*)d)->AlwaysDeleteCharacters( 0, p);
	tv->FrameDot(0);
	(d)->NotifyObservers( 0);
    }
}

static void
ClearTypescript(class typescript  *tv)
{
    ClearTypescriptText(tv);
    if (tv->cmdText != NULL) {
        (tv->cmdText)->Clear();
    }
    tv->lastCmdPos = 0;
}

static void
NoEchoCommand(class typescript  *tv)
{
    if(tv->readOnlyLen == -1)
        tv->readOnlyLen = 0;
}

/*
 * Reset the tty to a known state (like 'stty sane').
 *
 * This is really vendor-dependent stuff.
 */
static void
ResetTTY(int  fd	    /* file descriptor for the tty */)
{
#ifdef USE_TERMIOS
	/* Reset pty with Posix termios ioctl's */
	/* The pty runs in cooked mode.  Set everything to a known state
	 * se we can send signals, etc.
	 */
	{
		struct termios tios;

		tcgetattr(fd, &tios);
		tios.c_iflag = ICRNL;
#ifdef ONLCR
		tios.c_oflag = ONLCR;
#else
		tios.c_oflag = 0;
#endif
		tios.c_cflag = B9600|CS8|HUPCL|CREAD;
		tios.c_lflag = ISIG|ICANON|ECHO;
		tios.c_cc[VINTR] = '\003';	/* ^C */
		tios.c_cc[VQUIT] = '\034';	/* ^\ */
		tios.c_cc[VERASE] = '\010';	/* ^h */
		tios.c_cc[VKILL] = '\025';	/* ^u */
		tios.c_cc[VEOF] = '\004';	/* ^d */
		tios.c_cc[VSTART] = '\021';	/* ^q */
		tios.c_cc[VSTOP] = '\023';	/* ^s */
		tios.c_cc[VSUSP] = '\032';	/* ^z */

		if(tcsetattr(fd, TCSANOW, &tios) < 0)
			perror("typescript: failed to set terminal characteristics");
	}
#else
#if SY_AIX221
	{
	    struct termio ThisTerm;
	    ioctl(fd, TCGETA, &ThisTerm);
	    ThisTerm.c_iflag &= ~ICRNL;
	    ThisTerm.c_oflag &=	~OPOST;	    /* turn all this stuff off */
	    ThisTerm.c_lflag |= ECHO;
	    ThisTerm.c_cflag |=	ISIG;	    /* enable signals */
	    ThisTerm.c_cflag &=	~ICANON;    /* turn off the editing */
	    ioctl(fd, TCSETAW, &ThisTerm);
	}
#else /* #if SY_AIX221 */
	{	/* kernel doesn't reset pty's */
	    struct sgttyb sgttyb;
	    ioctl(fd, TIOCGETP, &sgttyb);
	    sgttyb.sg_flags &= ~CRMOD;
#if (!vax | vax2) /* everything but vax_11 */
	    /*  causes echoing on the vax; needed on RT and sun 
		so the test in ReadFromProcess will work */
	    sgttyb.sg_flags |= ECHO;
#endif /* (!vax | vax2)  */
#ifdef sun
	    /* Running in raw mode introduces other problems.
	       It makes a zero-byte write not introduce an EOF.
	       Don't do it.  "tm" manages to live without it.... */
	    sgttyb.sg_flags &= ~(CBREAK|RAW);
#else
	    /* Run in raw mode to fix select problems. */
	    sgttyb.sg_flags |= RAW;
#endif
	    ioctl (fd, TIOCSETP, &sgttyb);
	}
#endif /* #if SY_AIX221 */
#endif /* defined(POSIX_ENV) && !defined(sun) */
}

typescript::typescript()
{
	ATKinit;

    int pid;
    const char **arglist = NULL;
    int ptyChannel;
    int masterChannel;
    char ptyname[64];

    class style *defaultStyle;
    char bodyFont[100];
    long fontSize= 10;
    long fontStyle= fontdesc_Fixed;
    const char *font;
    if ((defaultStyle= GetDefaultStyle()) == NULL) {
	defaultStyle= new style;
	defaultStyle->SetName("default");

	if ((font= environ::GetProfile("typescript.bodyfont")) == NULL || !fontdesc::ExplodeFontName(font, bodyFont, sizeof(bodyFont), &fontStyle, &fontSize)) {
	    strcpy(bodyFont, "AndyType");
	    fontSize= 10;
	    fontStyle= fontdesc_Fixed;
	}
	defaultStyle->SetFontFamily(bodyFont);
	defaultStyle->SetFontSize(style_ConstantFontSize, fontSize);
	defaultStyle->ClearNewFontFaces();
	defaultStyle->AddNewFontFace(fontStyle);
	defaultStyle->SetJustification(style_LeftJustified);
	defaultStyle->SetNewLeftMargin(style_LeftMargin,16384, style_Inches);
	defaultStyle->SetNewIndentation(style_LeftEdge,-16384, style_Inches);
	defaultStyle->AddTabsCharacters();
	SetDefaultStyle(defaultStyle);
    }

#if !(defined(POSIX_ENV) || defined(hpux))
/* Disassociate this process from its controling tty. Must be done after opening the childs pty because that will become the controlling tty if we don't have one. */
    {
	int fd;

#ifdef SIGTTOU
	(void)signal(SIGTTOU, SIG_IGN);    /* For handling signals when changing the window size */
#endif

	fd = open("/dev/tty", O_RDWR);
	if(fd >= 0) {
	    ioctl(fd, TIOCNOTTY, 0);
	    close(fd);
	} 
	else
            setpgrp(0, 0);
      }
#endif

    this->frame = NULL;
    this->pipescript = FALSE;
    
    if(Pipescript) {
	this->pipescript = TRUE;
	Pipescript = FALSE;
    }
    else if(myarglist) {
	arglist = myarglist;
	myarglist = NULL;
    }
    else {
	const char *shell = environ::Get("SHELL");

	if (shell)
	    DefaultArgList[0] = shell;
	else
	    DefaultArgList[0] = "/bin/sh";
	arglist = DefaultArgList;
    }
    this->ChopLines = TRUE;
    this->OutputWait = 0;
    (this)->SetBorder( 5, 5);
    this->readOnlyLen = -1;
    this->lastPosition = -1;
    this->pgrpid = 0;
    this->ptyname = NULL;

    if(this->pipescript) {
	this->SCFile = df;
	this->SubChannel = -1;
	typescriptAddSearchMenu ();
	typescriptAddFileMenu();
	this->keystatep = keystate::Create(this, ssmap);
        this->menulistp = (typescriptMenus)->DuplicateML( this);
        this->cmdText = NULL;
	maxSize = 100000;
	THROWONFAILURE( TRUE);
	return;
    }
    
    this->progname =  strdup(*arglist);
    if(!GetPtyandName(&masterChannel, &ptyChannel, ptyname, sizeof(ptyname))) {
	printf("Can't connect subchannel\n");
	THROWONFAILURE( FALSE);
	return;
    }

    this->SubChannel = masterChannel;
    this->SlaveChannel = ptyChannel;
    this->ptyname = strdup(ptyname);

#if !defined(POSIX_ENV) || defined(sun)
    /* On non-SunOS POSIX systems we do not put the pty into REMOTE mode.
     * Instead we leave the pty in cooked mode.
     * XXX - this distinction needs to be more precise; POSIX systems
     * differ fairly strongly in their pty behaviors, and System V
     * Release 4 and 4.4BSD probably behave more like SunOS.
     */
#if SY_AIX221
    {
	int mode;

	mode = REMOTE;
	ioctl(this->SubChannel, PTYSETM, &mode);

	/* In order to establish a controlling terminal on AIX (and SysV??)
	 * we need to close the server (slave) and let the child reopen it.
	 * since the child is the first to open it, it will become the
	 * controlling terminal.  The master cannot have it open for this
	 * to happen.  If someone else comes along and opens it between the
	  * close and the open below, we are S.O.L.
	 */
	signal(SIGHUP, SIG_IGN);    /* ignore the hangup when we close the pty */
	close(ptyChannel);	    /* we reopen it in the child */
    }
#else /* #if SY_AIX221 */
    {
	int ON = 1;
	/* Convert TIOCREMOTE to 4.3 version, then try 4.2 version if
	   that doesn't work.  This is a HACK, and relies upon knowledge
	   of the encoding of ioctl's.  BUT it compiles correctly
	   with the ioctl.h from either 4.2 or 4.3.  What can you do?
	   Note that the ON flag is passed by reference in 4.3, by value in 4.2 */
#if defined(__STDC__) && !defined(__HIGHC__) || defined(_IBMR2) || defined(hpux) || defined(NeXT) || defined(sys_pmax_ul4)
        if(ioctl(this->SubChannel, _IOW('t', TIOCREMOTE&0xff, int), &ON) == -1)
            ioctl(this->SubChannel, _IO('t', TIOCREMOTE&0xff), ON);
#else /* defined(__STDC__) && !defined(__HIGHC__) */
	if(ioctl (this->SubChannel, _IOW(t, TIOCREMOTE&0xff, int), &ON) == -1)
	    ioctl (this->SubChannel, _IO(t, TIOCREMOTE&0xff), ON);
#endif /* defined(__STDC__) && !defined(__HIGHC__) */
	ioctl(this->SubChannel, FIOCLEX, 0);
    }
#endif /* #if SY_AIX221 */
#endif /* defined(POSIX_ENV) && !defined(sun) */


    if((pid = osi_vfork ()) < 0) {
	printf("Fork failed\n");
	THROWONFAILURE( FALSE);
	return;
    }
    if(pid == 0) {
	int pid = getpid ();
#ifdef POSIX_ENV
        /* Become a session leader. */
        if(setsid() < 0)
            perror("setsid");
        /* Re-open the pty so that it becomes the controlling terminal.
         * NOTE:  GetPtyandName must open the pty with the O_NOCTTY flag,
         * otherwise the parent (typescript) process will have the controlling
         * terminal and we (the shell) won't be able to get it.
         */
        if((ptyChannel = open(ptyname, O_RDWR)) < 0) {
             fprintf(stderr, "Could not open %s.\n", ptyname);
             exit(1);
        }
#else
#ifdef hpux
	setpgrp2(0, pid);
#else /* hpux */
#if SY_AIX221
	setpgrp();
	/* reopen the terminal so it becomes the controlling terminal */
	if((ptyChannel = open(ptyname, 2)) < 0) {
	    fprintf(stderr, "Could not open %s.\n", ptyname);
	    exit(1);
	}
#else /* if !SY_AIX221 */
	setpgrp(0, pid);
#endif /* if SY_AIX221 */
#endif /* hpux */
#endif /* !POSIX_ENV */
        dup2(ptyChannel, 0);
        dup2(ptyChannel, 1);
        dup2(ptyChannel, 2);

	/* Don't leave any unnecessary open file descriptors for child. */
	{
	    int numfds = FDTABLESIZE();
	    int fd;

	    for(fd = 3; fd < numfds; fd++)
		close(fd);
	}

#ifdef POSIX_ENV
    /* Set current terminal process group. */
    if(tcsetpgrp(0, pid) < 0) perror("tcsetpgrp failed");
#else
#if !SY_AIX221
	ioctl(0, TIOCSPGRP, &pid);
#endif /* if SY_AIX221 */
#endif /* POSIX_ENV */
        environ::Put("TERM", "wm");
        environ::Delete("TERMCAP");
	ResetTTY(0);
	execvp(this->progname, (char **)arglist);
	{   
	    char buf[200];

	    sprintf(buf, "Couldn't exec %s\n", this->progname);
	    write(1, buf, strlen(buf));
	    _exit(1);
	}
    }
#ifndef POSIX_ENV
#if SY_AIX221
    this->pgrpid = pid;	/* save pid for sending quit and intr signals */
#else /* SY_AIX221 */
#ifdef hpux
   setpgrp2(0, pgrp);
#else /* hpux */
    setpgrp(0, pgrp);
#endif /* hpux */
#endif /* SY_AIX221 */
#endif /* !POSIX_ENV */

    this->SCFile = fdopen(this->SubChannel, "r");
#if SY_AIX12
    setvbuf(this->SCFile, io_buffer, _IOLBF, sizeof(io_buffer));
#endif

    if(df) {
	struct proctable_Entry *UserMenuProc;
	char nbuf[512];

	UserMenuProc = proctable::DefineProc("Read-User-Menus", (proctable_fptr)typescript_HandleMenus, &typescript_ATKregistry_ , NULL, "Handle user supplied menus"); 
	while(fgets(nbuf, sizeof nbuf, df)) {
	    int pos = strlen(nbuf) - 1;
	    if(pos > 0) {
		if(nbuf[pos-1] == '\\')
		    nbuf[pos-1] = '\0';
		else
		    nbuf[pos] = '\r';
		typescriptAddMenu(nbuf, UserMenuProc);
	    }
	}
	fclose(df);
	df = NULL;
    }   
    typescriptAddtypescriptMenus();
    typescriptAddSearchMenu ();
    if(FileMenu) {
	typescriptAddFileMenu();
	FileMenu = FALSE;
    }
    this->menulistp = (typescriptMenus)->DuplicateML( this);
    this->keystatep = keystate::Create(this, ssmap);
    this->title = NULL;
    this->cmdText = new text;
    this->lastCmdPos = 0;
#ifdef UTMP_MUNGE
    utmp_add(ptyname, pid);
#endif
    THROWONFAILURE( TRUE);
}

#define COBSIZE 100
/* Called when can send to pty.  Removes handler when no more data remains to be sent. */


static void 
MyCanOutHandler(FILE  *afile, class typescript  *ad )
{
    long start;
    class text *myd;
    long i, end;
    char *tp, tc;
    char buffer[COBSIZE];

    if(ad->SubChannel < 0) 
	return;
    if(ad->readOnlyLen > 0) {
	WritePty(ad, ad->readOnlyBuf, ad->readOnlyLen);
	smashReadOnlyBuf(ad);
	ad->readOnlyLen = -1;
    }
    myd = TEXT(ad);
    start = (myd)->GetFence();
    end = (myd)->GetLength();
    if(start >= end) {
	ad->OutputWait = 0;
	im::RemoveCanOutHandler(ad->SCFile);
	return;
    }
    tp = buffer;
    for(i = start; i < end;) {
	tc = *tp++ = (myd)->GetChar( i);
	i++;		/* text_GetChar may become macro, so don't do this inside */
	if(tc == '\n') 
	    break;
	if(i >= start + COBSIZE - 2) 
	    break;
    }
    (myd)->SetFence( i);
    WritePty(ad, buffer, tp-buffer);
}

typescript::~typescript()
{
	ATKinit;

  /* dataobject_Destroy(TEXTOBJ(ap)); */ /* the doc will destroy its own marks */
    if(this->title)
	free(this->title);
    if(this->cmdText)
	(this->cmdText)->Destroy();
    if(this->ptyname) {
#ifdef UTMP_MUNGE
        utmp_delete(this->ptyname);
#endif
	free(this->ptyname);
    }
}

void 
typescript::ObservedChanged(class observable  *ov, long  value )
{
    long fencepos;

    /* after output has arrived, make sure view's dot is >= fence */
    if(ov == (class observable*) TEXT(this)) {
	if(value == observable_OBJECTDESTROYED)
	    /* the typescript can't do much without its text */
	    (this)->Destroy();
	else {
	    MaintainLastEnv(this);
	    fencepos = (TEXT(this))->GetFence();
	    if((fencepos <= (this)->GetDotPosition()) && (this)->Visible( this->lastPosition)) {
		(this)->FrameDot( fencepos);
	    }
	    (this)->WantUpdate( this);
	}
    }
}

void 
typescript::PostMenus(class menulist  *menulist)
{
    /* Ignore the textviews menus,
      but take advantage of the fact that it knows when to
      post (and retract) the selection menus */
    if(this->hasInputFocus) {
	long mask;
	boolean readonly = (this)->GetDotPosition() < (TEXT(this))->GetFence();

	mask = (((this)->GetDotLength() > 0) ? typescript_SelectionMenus : typescript_NoSelectionMenus) |
	  (readonly ? 0 : typescript_AfterFenceMenus);

	if((this->menulistp)->SetMask( mask))
	    (this)->textview::PostMenus( this->menulistp);
    }
}

void 
typescript::ReceiveInputFocus()
{
    this->textview::ReceiveInputFocus();
    if (this->keystatep) {
	this->keystatep->next = NULL;
	if (this->textview::keystate)
	    this->textview::keystate->next = NULL; 
	this->keystatep->AddBefore(this->textview::keystate); 
	this->PostKeyState(this->keystatep);
    }
    if (this->menulistp)
	this->menulistp->SetMask(textview_NoMenus);
}

static void 
typescript_handlereadonly(class typescript  *self ,char  c)
{   /* This will put characters in the read-only buffer without displaying them.
      Deals with the no-echo mode for entering passwords and the like. */
#ifdef SENDRAW
    write(self->SubChannel,&c,1);
#else
    if(isprint(c)) {
	if(self->readOnlyLen < (READONLYMAX - 1)) { /* Make sure to leave space for ending carriage return. */
	    self->readOnlyBuf[self->readOnlyLen++] = c;
	}
    }
#endif
}

static void
PositionDot(class typescript  *self)
{    
    long dotpos,markpos;
    class text *d = TEXT(self);

    if((dotpos = (self)->GetDotPosition()) < (markpos = (d)->GetFence())){
	while(dotpos < markpos) {
	    if((d)->GetChar(dotpos) == '\n') {
		textview_EndOfTextCmd(self, 0);
		return;
	    }
	    dotpos++;
	}
	(self)->SetDotPosition(dotpos); 
    }
    if(dotpos == markpos) {
/*	modify environment style so environment doesn't get fragmented */
        class environment *te =  MyEnvinfo(TEXT(self), dotpos);
        if(te->data.style == staticBoldStyle)
            (te)->SetStyle( TRUE , FALSE);
    }
}

static void 
typescript_YankCmd(class typescript  *self)
{    
    PositionDot(self);
    textview_YankCmd(self, 0);
}

static void 
Typescript_SelfInsertCmd(class typescript  *self, long  a)
{
    PositionDot(self);
    if(self->readOnlyLen != -1)
	typescript_handlereadonly(self, a);
    else
   	textview_SelfInsertCmd(self, a);
}

static void 
Typescript_DigitCmd(class typescript  *self, long  a)
{
    PositionDot(self);
    if(self->readOnlyLen != -1)
	typescript_handlereadonly(self, a);
    else
	textview_DigitCmd(self, a);
}

static void
typescript_BackwardsRotatePasteCmd(class typescript  *self)
    {
    if((self)->GetDotPosition() < (TEXT(self))->GetFence())
	return;
    textview_BackwardsRotatePasteCmd(self, 0);
}

static void
typescript_RotatePasteCmd(class typescript  *self)
{
    if((self)->GetDotPosition() < (TEXT(self))->GetFence())
	return;
    textview_RotatePasteCmd(self, 0);
}

/* What to do when the textview hasn't defined something... */
void 
typescript_NoTextviewKey(class typescript  *self, long  key)
{
    message::DisplayString(self, 0, "Could not execute command. Failure in looking up textview command.");
}

/*
 * Drag&drop support.
 * Drag current working directory (or at least what we think is
 * the current working directory) out onto another window.
 */
static void typescript_DragCwdCmd(class typescript  *self)
{
    class im *im = (self)->GetIM();
    char wd[4096];

    if (im != NULL) {
	getwd(wd);
	(im)->DropFile( wd, NULL);
    }
}

static void typescript_ResetTTY(class typescript  *self)
{
    ResetTTY(self->SlaveChannel);
}


static void
typescriptAddtypescriptMenus()
{
    struct proctable_Entry *tempProc;

    tempProc = proctable::Lookup("typescript-yank");
    (ssmap)->BindToKey( "\031", tempProc, 0);

    (typescriptMenus)->AddToML("Paste~11",tempProc,0, typescript_AfterFenceMenus | typescript_NoSelectionMenus);

    tempProc = proctable::Lookup("typescript-rotate-backward-paste");
    (ssmap)->BindToKey( "\033\031", tempProc, 0);

    tempProc = proctable::Lookup("typescript-rotate-paste");
    (ssmap)->BindToKey( "\033y", tempProc, 0);

    tempProc = proctable::Lookup("typescript-Grab-Last-Cmd");
    (ssmap)->BindToKey( "\033=", tempProc, 0);

    tempProc = proctable::Lookup("typescript-Grab-Current-Cmd");
    (ssmap)->BindToKey( "\033+", tempProc, 0);
    (typescriptMenus)->AddToML("Move~30",tempProc,0,0);

    tempProc = proctable::Lookup("typescript-Execute-Current-Cmd");
    (ssmap)->BindToKey( "\033\015", tempProc, 0);
    (typescriptMenus)->AddToML("Execute~31",tempProc,0,0);

    tempProc = proctable::Lookup("typescript-Grab-Next-Cmd");
    (ssmap)->BindToKey( "\033`", tempProc, 0);
}

boolean 
typescript::InitializeClass()
{
    struct proctable_Entry *tempProc, *si, *dig;
    struct ATKregistryEntry  *classInfo = &typescript_ATKregistry_ ;
    long i;
    char str[2];
    maxSize = environ::GetProfileInt("maxsize", 10000);
    extraRoom = maxSize / 10;

    CmdSize = 256;
    cmd = (char*) malloc(CmdSize);
    ssmap = new keymap;
    typescriptMenus = new menulist;

/* Initialize our pointers to textview command routines. */
    ATK::LoadClass("textview"); /* Make sure the textview is loaded first. */
    if((tempProc = proctable::Lookup("textview-line-to-top"))) {
        proctable::ForceLoaded(tempProc);
        textview_LineToTop = proctable::GetFunction(tempProc);
    }
    else
        textview_LineToTop = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-end-of-text"))) {
        textview_EndOfTextCmd = proctable::GetFunction(tempProc);
    }
    else
        textview_EndOfTextCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-self-insert")))
        textview_SelfInsertCmd = proctable::GetFunction(tempProc);
    else
        textview_SelfInsertCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-digit")))
        textview_DigitCmd = proctable::GetFunction(tempProc);
    else
        textview_DigitCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-beginning-of-line")))
        textview_BeginningOfLineCmd = proctable::GetFunction(tempProc);
    else
        textview_BeginningOfLineCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-end-of-line")))
        textview_EndOfLineCmd = proctable::GetFunction(tempProc);
    else
        textview_EndOfLineCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-yank")))
        textview_YankCmd = proctable::GetFunction(tempProc);
    else
        textview_YankCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-rotate-backward-paste")))
        textview_BackwardsRotatePasteCmd = proctable::GetFunction(tempProc);
    else
        textview_BackwardsRotatePasteCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-rotate-paste")))
        textview_RotatePasteCmd = proctable::GetFunction(tempProc);
    else
        textview_RotatePasteCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-delete-next-character")))
        textview_DeleteCmd = proctable::GetFunction(tempProc);
    else
        textview_DeleteCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-copy-region")))
        typescript_CopyRegionCmd = proctable::GetFunction(tempProc);
    else
        typescript_CopyRegionCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc = proctable::Lookup("textview-zap-region")))
        typescript_ZapRegionCmd = proctable::GetFunction(tempProc);
    else
        typescript_ZapRegionCmd = (proctable_fptr)typescript_NoTextviewKey;

    if((tempProc	= proctable::Lookup("textview-delete-previous-character")    ) )
        textview_RuboutCmd = proctable::GetFunction(tempProc);
    else
        textview_RuboutCmd = (proctable_fptr)typescript_NoTextviewKey;

    tempProc = proctable::DefineProc("typescript-interupt-command", (proctable_fptr) TypescriptINTCommand, classInfo, NULL, "Handle ^C");
    (ssmap)->BindToKey( "\003", tempProc, 0);
#if SY_AIX221
    (ssmap)->BindToKey( "\177", tempProc, 0);   /* bind DEL too! */
#endif /* #if SY_AIX221 */

    tempProc = proctable::DefineProc("typescript-beginning-of-line", (proctable_fptr) TypescriptLeftCommand, classInfo, NULL, "Move to line beginning");
    (ssmap)->BindToKey( "\001", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-end-of-line", (proctable_fptr) TypescriptEndOfLineCommand, classInfo, NULL, "Move to line end");
    (ssmap)->BindToKey( "\005", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-EOT-command", (proctable_fptr) TypescriptEOTCommand, classInfo, NULL, "Handle ^D");
    (ssmap)->BindToKey( "\004", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-return-cmd", (proctable_fptr) TypescriptReturnCommand, classInfo, NULL, "Handle enter key");
    (ssmap)->BindToKey( "\015", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-return-and-position", (proctable_fptr) TypescriptReturnAndPositionCommand, classInfo, NULL, "Handle enter key and place command at top of the screen");
    (ssmap)->BindToKey( "\012", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-ZAP-command", (proctable_fptr) TypescriptZapCommand, classInfo, NULL, "handles ^U line clearing");
    (ssmap)->BindToKey( "\025", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-Stop-cmd", (proctable_fptr) TypescriptSTOPCommand, classInfo, NULL, "Handle ^Z Stop cmd");
    (ssmap)->BindToKey( "\032", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-unbound", (proctable_fptr) TypescriptUnboundCommand, classInfo, NULL, "Does nothing.");
    (ssmap)->BindToKey( "\033~", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-zap-region", (proctable_fptr)typescript_ZapRegionCmd, classInfo, NULL, "Cut region to kill-buffer."); 
    (ssmap)->BindToKey( "\027", tempProc, 0);
    (typescriptMenus)->AddToML( "Cut~11", tempProc, 0, typescript_AfterFenceMenus | typescript_SelectionMenus);

    tempProc = proctable::DefineProc("typescript-copy-region", (proctable_fptr)typescript_CopyRegionCmd, classInfo, NULL, "Copy region to kill-buffer.");
    (ssmap)->BindToKey( "\033w", tempProc, 0);
    (typescriptMenus)->AddToML( "Copy~12", tempProc, 0, typescript_SelectionMenus);

    tempProc = proctable::DefineProc("typescript-QUIT-command", (proctable_fptr) TypescriptQUITCommand, classInfo, NULL, "handles quit cmd");
    (ssmap)->BindToKey( "\034", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-No-Echo-Cmd", (proctable_fptr) NoEchoCommand, classInfo, NULL, "Turn Off Echoing");
    (ssmap)->BindToKey( "\030\014", tempProc, 0);

    tempProc = proctable::DefineProc("typescript-delete-previous-character", (proctable_fptr)typescript_RuboutCmd, classInfo, NULL, "Delete the previous character.");
    (ssmap)->BindToKey( "\010", tempProc, 0); 
    (ssmap)->BindToKey( "\177", tempProc, 0);
 
    si = proctable::DefineProc("Typescript-self-insert", (proctable_fptr)Typescript_SelfInsertCmd, classInfo, NULL, "Insert a character.");
    dig = proctable::DefineProc("Typescript-digit", (proctable_fptr)Typescript_DigitCmd, classInfo, NULL, "Insert a character.");
    str[0] = ' ';
    str[1] = '\0';
    for(i = 32; i < 127; i++)  {
	if(i < 48 || i > 57)
	    (ssmap)->BindToKey( str, si, i);
	else
	    (ssmap)->BindToKey( str, dig, i);
	str[0]++;
    }

    tempProc = proctable::DefineProc("typescript-Clear", (proctable_fptr) ClearTypescript, classInfo, NULL, "Clear Typescript");
    (typescriptMenus)->AddToML("Clear~51",tempProc,0,0);

    tempProc = proctable::DefineProc("typescript-Clear-Text", (proctable_fptr) ClearTypescriptText, classInfo, NULL, "Clear the text of the typescript");

    tempProc = proctable::DefineProc("typescript-drag-cwd", (proctable_fptr)typescript_DragCwdCmd, classInfo, NULL, "Drag out current working directory.");
    tempProc = proctable::DefineProc("typescript-reset-tty", (proctable_fptr)typescript_ResetTTY, classInfo, NULL, "Reset the tty (stty sane)");
    (ssmap)->BindToKey( "\033r", tempProc, 0);

#ifdef PRINTER_SETUP_DIALOG_ENV
    ATK::LoadClass("printopts");
    tempProc = proctable::Lookup("printopts-post-window");
    if (tempProc) typescript_poptPostWindow = proctable::GetFunction(tempProc);
#endif

/* We need to define these here so that fcomp can look them up before a tscript object is created. */
    tempProc = proctable::DefineProc("typescript-yank", (proctable_fptr)typescript_YankCmd, classInfo, NULL, "Yank text back from kill-buffer.");

    tempProc = proctable::DefineProc("typescript-rotate-backward-paste",  (proctable_fptr)typescript_BackwardsRotatePasteCmd, classInfo, NULL, "Rotate kill-buffer backwards.");

    tempProc = proctable::DefineProc("typescript-rotate-paste", (proctable_fptr)typescript_RotatePasteCmd, classInfo, NULL, "Rotate kill-buffer.");

    tempProc = proctable::DefineProc("typescript-Grab-Last-Cmd", (proctable_fptr) GrabLastCommand, classInfo, NULL, "Grab Last Command");

    tempProc = proctable::DefineProc("typescript-Grab-Current-Cmd", (proctable_fptr) GrabCurrentCommand, classInfo, NULL, "Grab Current Command");

    tempProc = proctable::DefineProc("typescript-Execute-Current-Cmd", (proctable_fptr) ExecuteCurrentCommand, classInfo, NULL, "Execute Current Command");

    tempProc = proctable::DefineProc("typescript-Grab-Next-Cmd", (proctable_fptr) GrabNextCommand, classInfo, NULL, "Grab Next Command");

    return TRUE;
}

void 
typescript::Update()
{
    this->lastPosition = -1;
    (this)->textview::Update();
}

class view *
typescript::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    class view *v;
    int i;
    char *p;

    v = (this)->textview::Hit( action, x, y, numberOfClicks);
    if (action == view_LeftFileDrop) {
	char **files;
	class im *im = (this)->GetIM();

	if (im == NULL) return v;
	files = (im)->GetDroppedFiles();
	if (files) {
	    if (files[0] != NULL)
		free(files[0]);	/* ignore host for now */
	    for (i = 1; files[i] != NULL; i++) {
		if (i != 0)
		    textview_SelfInsertCmd(this, ' ');
		for (p = files[i]; *p != '\0'; p++)
		    textview_SelfInsertCmd(this, *p);
		free(files[i]);
	    }
	    free(files);
	}
    }
    return v;
}

#ifdef TIOCGWINSZ
static void
NullWinSizeProc()
{
}
#endif /* TIOCGWINSZ */

void 
typescript::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
{
    this->lastPosition = -1;
    (this)->textview::FullUpdate( type, left, top, width, height);

#ifdef TIOCGWINSZ
    if (width > 0 && height > 0) {
	class style *tsStyle;
	enum style_FontSize dummy;
	char ffamily[1000];
	long tssize;
	class fontdesc *tsfont;
	struct FontSummary *fontSummary;
	long widthinchar, heightinchar;
	struct winsize winSize;

	im::SignalHandler(SIGWINCH, (im_signalfptr)NullWinSizeProc, NULL);

	tsStyle = (this)->GetDefaultStyle();
	(tsStyle)->GetFontSize( &dummy, &tssize);
	(tsStyle)->GetFontFamily( ffamily, 1000);
	(tsStyle)->AddTabsCharacters();
	tsfont = fontdesc::Create(ffamily, (tsStyle)->GetAddedFontFaces(), tssize);
	fontSummary = (tsfont)->FontSummary( (this)->GetDrawable());
	if(fontSummary) {
	    widthinchar = ((width - this->bx - this->bx) / fontSummary->maxSpacing) - 2;
	    heightinchar = ((height - this->by - this->by) / fontSummary->maxHeight);
	} else {
	    widthinchar = 80;
	    heightinchar = 0;
	}

	// See what the value currently is and change only if necessary to
	// avoid extra SIGWINCH'es to clients.  We redraw more frequently than they.
	ioctl (this->SubChannel, TIOCGWINSZ, &winSize);
	if(winSize.ws_col != widthinchar || winSize.ws_row != heightinchar) {
	    winSize.ws_col = widthinchar;
	    winSize.ws_row = heightinchar;
	    ioctl(this->SubChannel, TIOCSWINSZ, &winSize);
	}
    }
#endif /* TIOCGWINSZ */
}

void 
typescript::GetClickPosition(long  position, long  numberOfClicks, enum view_MouseAction  action, long  startLeft, long  startRight, long  *leftPos, long  *rightPos)
{
    if(numberOfClicks  %3) {
	(this)->textview::GetClickPosition( position, numberOfClicks, action, startLeft, startRight, leftPos, rightPos);
	return;
    }
    if(GetCommandEnv(this, position, leftPos, rightPos) == NULL)
	(this)->textview::GetClickPosition( position, numberOfClicks, action, startLeft, startRight, leftPos, rightPos);
}


static int 
WritePty(class typescript  *tv, char  *buf, int  len)
{
#ifdef USE_TERMIOS
/* Write to a non-SunOS POSIX pty.  Turn off echo before the write,
 * then reset echo to its previous state after the write.
 * XXX - this distinction needs to be more precise; POSIX systems
 * differ fairly strongly in their pty behaviors, and System V
 * Release 4 and 4.4BSD probably behave more like SunOS. */
    int ret;
    struct termios tios;

    /* Save flags. */
    tcgetattr(tv->SlaveChannel, &tios);
    tios.c_lflag &= ~ECHO;	/* turn off echo for a moment */
    tcsetattr(tv->SlaveChannel, TCSANOW, &tios);
    /* Write the data */
    ret = write(tv->SubChannel, buf, len);
    /* Put echo back */
    tios.c_lflag |= ECHO;	/* assume it was on!! */
    tcsetattr(tv->SlaveChannel, TCSANOW, &tios);
    return ret;
#else /* defined(POSIX_ENV) && !defined(sun) */
/* Non-POSIX or SunOS...just write to the pty. */
    return write(tv->SubChannel, buf, len);
#endif /* defined(POSIX_ENV) && !defined(sun) */
}

void 
typescript::SetDataObject(class dataobject  *obj)
{
    long stpos;
    class typetext *shtext = (class typetext*) obj;
    class text *SsText = (class text*) shtext;

    (this)->textview::SetDataObject( obj);
    if(!staticBoldStyle) {
	if((staticBoldStyle = (SsText->styleSheet)->Find( "bold")) == NULL) {
	    staticBoldStyle = new style;
	    (staticBoldStyle)->SetName( "bold");
	    (staticBoldStyle)->AddNewFontFace( fontdesc_Bold);
	    (SsText->styleSheet)->Add( staticBoldStyle);
	}
    }
    stpos = (SsText)->GetLength();
    this->cmdStart = (SsText)->CreateMark(stpos,0);
    (SsText)->SetFence( stpos);
    (this->cmdStart)->IncludeBeginning() = TRUE;
    if(!shtext->hashandler) {
	im::AddFileHandler(this->SCFile, (im_filefptr) ReadFromProcess, (char*) this, 6);
	shtext->hashandler = TRUE;
    }
    (this)->SetDotPosition(stpos);
    im::ForceUpdate();
}

void 
typescript::SetTitle(const char  *title)
{
    if(this->title != NULL)
	free(this->title);
    if(title || *title != '\0')
	this->title = strdup(title);
    else
	this->title = NULL;
}

const char *
typescript::GetTitle()
{
    return this->title;
}

/*
 * Set/Get frame.
 * Typescriptapp sets the initial frame.  When the title
 * changes (via the ^A(pwd)^B hack), typescript then has
 * a frame which can be given the new title.
 */
void 
typescript::SetFrame(class frame  *frame)
{
    this->frame = frame;
}

class frame *
typescript::GetFrame()
{
    return this->frame;
}

#ifdef UTMP_MUNGE
/*
 * The following establishes a utmp entry if /etc/utmp is writeable.
 * Since typescript should never be setuid root (implying ez and everything else is),
 * this will only work on relaxed systems where /etc/utmp is open.
 */
void
utmp_add(char  *ptyname, int  pid)
{
    struct utmp utmp;
    char *username;

    memset(&utmp, 0, sizeof(utmp));
    username = cuserid(NULL);
    if(username) {
        strncpy(utmp.ut_user, username, sizeof(utmp.ut_user));

	/* skip /dev/ */
        strncpy(utmp.ut_line, ptyname + 5, sizeof(utmp.ut_line));

	/* skip /dev/ */
        strncpy(utmp.ut_id, ptyname + 5, sizeof(utmp.ut_id));

        utmp.ut_type = USER_PROCESS;
        utmp.ut_pid = pid; /* pid of shell or command */
        utmp.ut_time = time(NULL);
        strncpy(utmp.ut_host, "typescript", sizeof(utmp.ut_host));
        setutent();
        pututline(&utmp); /* This may fail on secure systems. */
    }
}

void 
utmp_delete(char  *ptyname)
{
    struct utmp utmp;

    memset(&utmp, 0, sizeof(utmp));
    setutent();

    /* skip /dev/ */
    strncpy(utmp.ut_line, ptyname+5, sizeof(utmp.ut_line));

    /* skip /dev/ */
    strncpy(utmp.ut_id, ptyname+5, sizeof(utmp.ut_id));

    utmp.ut_type = DEAD_PROCESS;
    pututline(&utmp);	/* This may fail on secure systems. */
}
#endif
