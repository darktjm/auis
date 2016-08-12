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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atkams/messages/lib/RCS/messagesapp.C,v 1.6 1994/08/15 02:19:08 rr2b Stab74 $";
#endif


 

#include <andrewos.h>
#include <textview.H>
#include <sys/param.h>
#include <stdio.h>
#include <errprntf.h>

#include <im.H>
#include <frame.H>

#include <cui.h>
#include <fdphack.h>
#include <sendmessage.H>
#include <messwind.H>
#include <captions.H>
#include <folders.H>
#include <init.H>
#include <environ.H>
#include <ams.H>
#include <amsutil.H>
#include <messagesapp.H>

#include <msgsvers.h>


ATKdefineRegistry(messagesapp, application, NULL);
#ifndef NORCSID
#endif


messagesapp::messagesapp()
{
    this->MailOnly = FALSE;
    this->SendOnly = FALSE;
    this->AppendBugInfo = FALSE;
    this->ConsiderChanged = 0;
    this->UsingSnap = 0;
    this->RetainedArgc = 0;
    this->RetainedArg[0] = NULL;
    this->AddHeaderList[0] = NULL;
    this->MailSourceFile[0] = '\0';
    this->SeparateWindows = FALSE;
    (this)->SetMajorVersion( MESSAGES_MAJOR_VERSION);
    (this)->SetMinorVersion( MESSAGES_MINOR_VERSION);
    THROWONFAILURE((TRUE));
}

boolean messagesapp::Start()
{
    class folders *fold = NULL;
    class sendmessage *sm = NULL;
    class view *v;
    int i;
    class ams *tams=NULL;
    char VerString[25], DeliveryVersion[25];
    boolean Die = FALSE;

    (this)->application::Start();
    sprintf(VerString, "Version %d.%d-%c",
             MESSAGES_MAJOR_VERSION,MESSAGES_MINOR_VERSION , this->UsingSnap ? 'S' : 'N');
    sprintf(DeliveryVersion, "Messages %d.%d-%c",
	    MESSAGES_MAJOR_VERSION,MESSAGES_MINOR_VERSION , this->UsingSnap ? 'S' : 'N');
    tams=ams::MakeAMS();
    if(tams) {
	tams->RemoveErrorDialogWindow();
    }
    if (this->SendOnly) {
	sm = new sendmessage;
	sm->myframe = ams::InstallInNewWindow(sm, "sendmessage", VerString, environ::GetProfileInt("sendmessage.width", -1), environ::GetProfileInt("sendmessage.height", -1), sm);
	if (!sm->myframe) Die = TRUE;
	v = (class view *) sm;
    } else {
	if (this->SeparateWindows) {
	    fold = new folders;
	    v = (class view *) (fold)->GetApplicationLayer();
	    fold->myframe = ams::InstallInNewWindow(v, "messages-folders", VerString, environ::GetProfileInt("folders.width", 600), environ::GetProfileInt("folders.height", 120), fold);
	} else {
	    class messwind *mess = new messwind;
	    fold = mess->foldersp;
	    v = (class view *) mess;
	    fold->myframe = ams::InstallInNewWindow(mess, "messages", VerString, environ::GetProfileInt("messages.width", -1), environ::GetProfileInt("messages.height", -1), fold);
	}
	if (!fold->myframe) Die = TRUE;
    }
    if (Die) {
	fprintf(stderr, "Could not create new im object--sorry!");
	exit(-1);
    }
    ams::SetCUIRock((long)v);
    (this)->Fork();
    message::DisplayString(NULL, 10, "Processing arguments...");
    im::ForceUpdate();
    ams::WaitCursor(TRUE);
    (ams::GetAMS())->CUI_SetClientVersion( DeliveryVersion);

    if (this->SendOnly) {
	char TmpToHeader[1000];

	TmpToHeader[0] = '\0';
	for (i=0; i<this->RetainedArgc; ++i) {
	    char *s = amsutil::StripWhiteEnds(this->RetainedArg[i]);
	    strcat(TmpToHeader, s);
	    if (*(s+strlen(s)-1) != ',') {
		strcat(TmpToHeader, ", ");
	    } else {
		strcat(TmpToHeader, " ");
	    }
	}
	if (this->MailSourceFile[0]) {
	    (sm)->ReadFromFile( this->MailSourceFile, FALSE);
	} else {
	    (sm)->Reset();
	}
	for(i=0; this->AddHeaderList[i]; ++i) {
	    (sm)->AddHeaderLine( this->AddHeaderList[i]);
	    free(this->AddHeaderList[i]);
	}
	if (TmpToHeader[0]) {
	    TmpToHeader[strlen(TmpToHeader) - 2] = '\0';
	    (sm)->AddToToHeader( TmpToHeader);
	}
	if (this->ConsiderChanged) {
	    --sm->HeadModified;
	}
	(sm)->ResetSendingDot();
	if (this->AppendBugInfo) {
	    (sm)->AppendBugInfoToBody( FALSE);
	}
    } else {
	(fold)->UpdateMsgs( this->MailOnly, (this->RetainedArgc > 0) ? this->RetainedArg : NULL, FALSE);
	if (this->MailOnly && amsutil::GetOptBit(EXP_SHOWCLASSES)) {
	    (fold)->Reconfigure( LIST_MAIL_FOLDERS);
	}
    }
    ams::WaitCursor(FALSE);
    return(TRUE);
}
 
boolean messagesapp::ParseArgs(int  argc, char  **argv)  
{
    int HeadersToAdd = 0;
    (this)->application::ParseArgs( argc, argv);
    ++argv;
    while (argv[0]) {
	if (argv[0][0] == '-') {
	    switch (argv[0][1]) {
		case 'b':
		    this->AppendBugInfo = TRUE;
		    break;
		case 'f':
		    ++argv;
		    if (argv[0]) {
			strcpy(this->MailSourceFile, argv[0]);
		    } else {
			--argv;
		    }
		    break;
		case 'h':
		    ++argv;
		    if (argv[0]) {
			this->AddHeaderList[HeadersToAdd] = (char *)malloc(1+strlen(argv[0]));
			strcpy(this->AddHeaderList[HeadersToAdd++], argv[0]);
			if (HeadersToAdd >= MAXADDHEADS) {
			    fprintf(stderr, "Too many added headers (compiled-in maximum is %d)\n", MAXADDHEADS);
			    exit(-1);
			}
		    } else {
			--argv;
		    }
		    break;
		case 'm':
		    this->MailOnly = TRUE;
		    break;
		case 's':
		    this->SendOnly = TRUE;
		    break;
		case 'w':
		    this->SeparateWindows = TRUE;
		    break;
		case 'z':
		    this->ConsiderChanged = 1;
		    break;
		case 'N':
		    this->UsingSnap = 0;
		    ams::SetWantSnap(this->UsingSnap);
		    break;
		case 'S':
		    this->UsingSnap = 1;
		    break;

		default:
		    fprintf(stderr, "Illegal option: '%s'--ignored\n", argv[0]);
		    break;
	    }
	} else {
	    this->RetainedArg[this->RetainedArgc++] = argv[0];
	    if (this->RetainedArgc >= MAXARGS) {
		fprintf(stderr, "Too many arguments (compiled-in maximum is %d)\n", MAXARGS);
		exit(-1);
	    }
	}
	++argv;
    }
    this->RetainedArg[this->RetainedArgc] = NULL;
    this->AddHeaderList[HeadersToAdd] = NULL;
    im::SetProgramName((char *)(this->SendOnly ? "sendmessage" : "messages"));
    (this)->SetName( (char *)(this->SendOnly ? "sendmessage" : "messages"));
    return(TRUE);
}
