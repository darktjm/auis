/* Copyright Carnegie Mellon University 1992, 1993  All rights Reserved */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/nessm.C,v 1.7 1995/07/11 19:16:53 rr2b Stab74 $";
#endif


#include <andrewos.h>
ATK_IMPL("nessm.H")

#include <ctype.h>

#include <proctable.H>
#include <message.H>
#include <view.H>
#include <im.H>
#include <buffer.H>
#include <frame.H>
#include <environ.H>
#include <mark.H>
#include <stylesheet.H>
#include <style.H>

#include <ness.H>
#include <nessm.H>


static char keytmpl[]="DoKeys(currentinputfocus, \"%\")\n";
static char menutmpl[]="(currentinputfocus, %)\n";
static char answertmpl[]="QueueAnswer(\"%\")\n";
static char canceltmpl[]="QueueCancellation()\n";
static char menu[]="menu";
static char keys[]="keys";


ATKdefineRegistry(nessm, ATK, nessm::InitializeClass);

static char *mousehits(enum view_MouseAction  act);
static long InsertKey(class ness  *n, class mark  *m, long  pos, char  key);
static int InsertProcCall(class ness  *n, long  pos, struct proctable_Entry  *proc, long  rock);
static void DumpActions(struct action  *a);
static void RegionToPrintable(class ness  *n, long  pos , long  len);
static struct action *QueueAnswers(class ness  *n, struct action  *look, class mark  *m);
static void DoConv(class view  *self, long  rock);


static char *mousehits(enum view_MouseAction  act) {
    switch(act) {
	case view_LeftDown:
	    return "mouseleftdown";
	case view_LeftUp:
	    return "mouseleftup";
	case view_LeftMovement:
	    return "mouseleftmove";
	case view_RightDown:
	    return "mouserightdown";
	case view_RightUp:
	    return "mouserightup";
	case view_RightMovement:
	    return "mouserightmove";
	default:
	    return "ERROR";
    }
};

static long lastpos=(-1);
static enum im_EventType lasttype=im_NoEvent;
static long InsertKey(class ness  *n, class mark  *m, long  pos, char  key) {
    if(lasttype!=im_KeyboardEvent) {
	lastpos=pos=(m)->GetPos()-1;
	(n)->AlwaysInsertCharacters( pos, keytmpl, strlen(keytmpl));
	pos=(n)->Index( pos, '%', (n)->GetLength()-pos);
	(n)->AlwaysDeleteCharacters( pos, 1);
    }
    if(key=='\0') key=(char)0x80;
    (n)->AlwaysInsertCharacters(  pos, &key, 1);
    return pos+1;
}

static int InsertProcCall(class ness  *n, long  pos, struct proctable_Entry  *proc, long  rock) {
    char buf[32];
    char procbuf[256], *p;
    char *procname=proctable::GetName(proc);
    char *opstring;
    
    if(strlen(procname)>sizeof(procbuf)-1) {
	return -1;
    }
    strcpy(procbuf, procname);
		
    p=procbuf-1;
    while(p=strchr(p+1, '-')) *p='_';
    lastpos=pos;
    (n)->AlwaysInsertCharacters( pos, menutmpl, strlen(menutmpl));
    (n)->AlwaysInsertCharacters( pos, procbuf, strlen(procbuf));
    pos+=strlen(procbuf);
    pos=(n)->Index( pos, '%', (n)->GetLength()-pos);
    if(rock>255) {
	opstring = (char *)rock;
	(n)->AlwaysReplaceCharacters( pos, 1, opstring, strlen(opstring));
	(n)->AlwaysInsertCharacters( pos+strlen(opstring), "\"", 1);
	(n)->AlwaysInsertCharacters( pos, "\"", 1);
    } else {
	sprintf(buf, "\"%d\"", rock);
	(n)->AlwaysReplaceCharacters( pos, 1, buf, strlen(buf));
    }
    return 0;
}


static void DumpActions(struct action  *a) {
    while(a) {
	printf("a:%x\n",a);
	switch(a->type) {
	    case im_KeyboardEvent:
		printf("key:'%c'\n",a->v.key);
		break;
	    case im_ProcEvent:
		printf("proc %s\n",proctable::GetName(a->v.proc.procTableEntry));
		printf("rock %d '%c'\n",a->v.proc.rock, a->v.proc.rock);
		break;
	    case im_MenuEvent:
		printf("menu proc %s\n",proctable::GetName(a->v.proc.procTableEntry));
		printf("rock %d '%c'\n",a->v.proc.rock, a->v.proc.rock);
		break;
	    case im_MouseEvent:
		printf("mouse event!!\n");
		break;
	    case im_AnswerEvent:
		printf("answer:%x\n",a->v.answer);
		printf("answer:'%s'\n",a->v.answer);
		break;
	    case im_SuspendEvent:
		printf("suspend\n");
		break;
	    case im_ResumeEvent:
		printf("resume\n");
		break;
	    default:
		printf("unhandled type %d\n",a->type);
	}
	a=a->next;
    }
}

#define CNTRL(c) ((c)-'@')

static void RegionToPrintable(class ness  *n, long  pos , long  len) {
    class mark *m=(n)->CreateMark( pos, len);
    long cpos=pos;
    int c;
    while(cpos<(m)->GetPos()+(m)->GetLength()) {
	switch(c=(n)->GetChar( cpos)) {
	    case '"':
	    case '\\':
		(n)->AlwaysInsertCharacters( cpos, "\\", 1);
		cpos+=2;
		break;
	    case '\033':
		(n)->AlwaysReplaceCharacters( cpos, 1, "\\e", 2);
		cpos+=2;
		break;
	    case '\n':
		(n)->AlwaysReplaceCharacters( cpos, 1, "\\n", 2);
		cpos+=2;
		break;
	    case '\r':
		(n)->AlwaysReplaceCharacters( cpos, 1, "\\r", 2);
		cpos+=2;
		break;
	    case '\t':
		(n)->AlwaysReplaceCharacters( cpos, 1, "\\t", 2);
		cpos+=2;
		break;
	    default: 
		if(c>=CNTRL('@') && c<=CNTRL('Z')) {
		    char buf[4];
		    buf[0]='\\';
		    buf[1]='^';
		    buf[2]=c+'@';
		    buf[3]='\0';
		    (n)->AlwaysReplaceCharacters( cpos, 1, buf, 3);
		    cpos+=2;
		} else {
		    cpos+=1;
		}
	}
    }
    (n)->RemoveMark( m);
    delete m;
}


static struct action *QueueAnswers(class ness  *n, struct action  *look, class mark  *m) {
    struct action *a=look->next;
    int susplevel=1;
    while(a && a->type!=im_SuspendEvent) a=a->next;
    if(a) a=a->next;
    while(a && susplevel>0) {
	switch(a->type) {
	    case im_SuspendEvent:
		susplevel++;
		break;
	    case im_ResumeEvent:
		if(susplevel>0) susplevel--;
		break;
	    case im_AnswerEvent:
		if(a->v.answer) {
		    long lastpos=(m)->GetPos()-1;
		    long pos=0;
		    (n)->AlwaysInsertCharacters( lastpos, answertmpl, strlen(answertmpl));
		    pos=(n)->Index( lastpos, '%', (n)->GetLength()-lastpos);
		    (n)->AlwaysReplaceCharacters( pos, 1, a->v.answer, strlen(a->v.answer));
		    RegionToPrintable(n, pos, strlen(a->v.answer));
		} else {
		    (n)->AlwaysInsertCharacters( (m)->GetPos()-1, canceltmpl, strlen(canceltmpl));
		}
		break;
	    default: ;
	}
	a=a->next;
    }
    return a;
}
 
static char *choices[]={
    "Cancel - Don't make any binding.",
    "Install as a menu option.",
    "Install as a key binding.",
    "Edit the script.",
    NULL
};


static void DoConv(class view  *self, long  rock) {
    struct action *look=im::GetMacro();
    class ness *n;
    char ch;
    char nbuf[256], finalname[512], reallyfinalname[512];
    char *p;
    long pos;
    char *tname;
    long tlen;
    class buffer *buffer;
    class mark *m, *mke;
    long mpos, mkpos;
    long result;
    class frame *f;
    class im *im=(self)->GetIM();
    
    if(look==NULL) {
	message::DisplayString(self, 0, "No keyboard macro defined!");
	return;
    }

    n=new ness;

    if(n==NULL) {
	message::DisplayString(self, 100, "Couldn't create a ness script!");
	return;
    }

    (n)->SetAccessLevel( ness_codeUV);
#ifdef ROB
    result=(n)->ReadNamedFile( "/afs/andrew/usr20/rr2b/work/ness/objects/mtmpl.n");
#else
    result=(n)->ReadNamedFile( environ::AndrewDir("/lib/ness/mtmpl.n"));
#endif
    if(result<0) {
	message::DisplayString(self, 100, "Couldn't read macro template!");
	(n)->Destroy();
	return;
    }
    
    (n)->SetFilename( NULL);
    (n)->SetNeedsDialogBox( FALSE);
    (n)->SetReadOnly( FALSE);

    
    pos=(n)->Index( 0, '%', (n)->GetLength());
    if(pos<0) {
	message::DisplayString(self, 100, "Internal ERROR: couldn't find type name insertion point.");
	(n)->Destroy();
	return;
    }

    if(im && (im)->GetInputFocus()) 
		self=(class view *)(im)->GetInputFocus();

    tname = (char *)(self)->GetTypeName();
    tlen=strlen(tname);

    /* replace the argument to extend with the right view type */
    (n)->AlwaysReplaceCharacters( pos, 1, tname, tlen);

    /* find the place to insert the type of binding being done */
    pos+=tlen;
    mkpos=(n)->Index( pos, '%', (n)->GetLength()-pos);

    /* find the place to put the binding to be made */
    pos=mkpos+1;
    mpos=(n)->Index( pos, '%', (n)->GetLength()-pos);

    /* find where to put the code for the binding. */
    pos=mpos+1;
    pos=(n)->Index( pos, '%', (n)->GetLength()-pos)+1;
    
    m=(n)->CreateMark( pos, 1);
    if(m==NULL) {
	message::DisplayString(self, 0, "Internal ERROR: couldn't allocate mark!");
	(n)->Destroy();
	return;
    }
    
    (m)->SetStyle( FALSE, FALSE);

    /* find where to put the end of the type of binding being made */
    pos=(n)->Index( pos, '%', (n)->GetLength()-pos);

    (n)->AlwaysDeleteCharacters( pos, 1);
    mke=(n)->CreateMark( pos, 0);
    if(mke==NULL) {
	message::DisplayString(self, 0, "Internal ERROR: couldn't allocate mark!");
	(n)->RemoveMark( m);
	delete m;
	(n)->Destroy();
	return;
    }
    
    (mke)->SetStyle( FALSE, FALSE);
    
    lasttype=im_NoEvent;
    while(look) {
	struct action *nlook=NULL;
	switch(look->type) {
	    case im_KeyboardEvent:
		nlook=QueueAnswers(n, look, m);
		pos=InsertKey(n, m, pos, look->v.key);
		look=nlook;
		break;
	    case im_MenuEvent:
		lasttype=look->type;
		nlook=QueueAnswers(n, look, m);
		pos=(m)->GetPos()-1;		
		if(InsertProcCall(n, pos, look->v.proc.procTableEntry, look->v.proc.rock)<0) {

		    message::DisplayString(self, 100, "ERROR: proctable entry name too long\n");
		    (n)->Destroy();
		    return;
		}
		look=nlook;
		break;
	    case im_ProcEvent:
		if(look->v.proc.keys->next || !isprint(look->v.proc.keys->v.key)) {

		    nlook=QueueAnswers(n, look, m);
		    pos=(m)->GetPos()-1;
		    lasttype=look->type;
		    if(InsertProcCall(n, pos, look->v.proc.procTableEntry, look->v.proc.rock)<0) {

			message::DisplayString(self, 100, "ERROR: proctable entry name too long\n");
			(n)->Destroy();
			return;
		    }
		} else {
		    nlook=QueueAnswers(n, look, m);
		    pos=InsertKey(n, m, pos, look->v.proc.keys->v.key);
		    lasttype=im_KeyboardEvent;
		}
		look=nlook;
		break;
	    case im_MouseEvent:
		nlook=QueueAnswers(n, look, m);
		lasttype=look->type;
		sprintf(nbuf, "DoHit(currentwindow, %s, %d, %d)\n",mousehits(look->v.mouse.action), look->v.mouse.x, look->v.mouse.y);
		lastpos=(m)->GetPos()-1;
		(n)->AlwaysInsertCharacters( lastpos, nbuf, strlen(nbuf));
		look=nlook;
		break;
	    default: look=look->next;
	}
    }

    (n)->DeleteCharacters( (m)->GetPos()-1, 2);
    (n)->RemoveMark( m);
    delete m;

    if(message::MultipleChoiceQuestion(self, 100, "Macro Script Generated", 0, &result, choices, NULL)<0 || result==0) {
	message::DisplayString(self, 0, "Cancelled!");
	(n)->Destroy();
	return;
    }

    buffer::GetUniqueBufferName("NessMacro", nbuf, sizeof(nbuf));

    buffer=buffer::Create(nbuf, NULL, NULL, n);


    if(buffer==NULL) {
	message::DisplayString(self, 100, "ERROR: couldn't get buffer for script.");
	(n)->RemoveMark( mke);
	delete mke;
	(buffer)->Destroy();
	return;
    }

    (buffer)->SetDestroyData( TRUE);
    
    switch(result) {
	case 1:

	    if(message::AskForString(self, 0, "Menu Name (eg. Card,Item):", NULL, nbuf, sizeof(nbuf))<0) {
		message::DisplayString(self, 0, "Cancelled!");
		(n)->RemoveMark( mke);
		delete mke;
		(buffer)->Destroy();
		return;
	    }

	    (n)->AlwaysReplaceCharacters( mpos, 1, nbuf, strlen(nbuf));
	   
	    (n)->AlwaysReplaceCharacters( mkpos, 1, menu, strlen(menu));

	    (n)->AlwaysInsertCharacters( (mke)->GetPos(), menu, strlen(menu));
	    (n)->RemoveMark( mke);
	    delete mke;

	    strcpy(finalname, "Menu:");
	    strcat(finalname, nbuf);
	    
	    buffer::GetUniqueBufferName(finalname, reallyfinalname, sizeof(reallyfinalname));
	    (buffer)->SetName( reallyfinalname);

	    if((n)->Compile()) (n)->printerrors( stderr);
	    return;
	case 2:

	    if(message::AskForString(self, 0, "Key Sequence (eg. \\^X\\^L):", NULL, nbuf, sizeof(nbuf))<0) {
		message::DisplayString(self, 0, "Cancelled!");
		(n)->RemoveMark( mke);
		delete mke;
		(buffer)->Destroy();
		return;
	    }
	    (n)->AlwaysReplaceCharacters( mpos, 1, nbuf, strlen(nbuf));
	    
	    (n)->AlwaysReplaceCharacters( mkpos, 1, keys, strlen(keys));
	    (n)->AlwaysInsertCharacters( (mke)->GetPos(), keys, strlen(keys));
	    (n)->RemoveMark( mke);
	    delete mke;
	    
	    strcpy(finalname, "Keybinding:");
	    strcat(finalname, nbuf);
	    
	    buffer::GetUniqueBufferName(finalname, reallyfinalname, sizeof(reallyfinalname));
	    (buffer)->SetName( reallyfinalname);
	    
	    if((n)->Compile()) (n)->printerrors( stderr);
	    return;
	case 3: {
	    (n)->AlwaysInsertCharacters( (mke)->GetPos(), "%", 1);
	    f=frame::GetFrameInWindowForBuffer(buffer);
	    if(f!=NULL) {

		class view *v=(f)->GetView();
		if(v!=NULL) (v)->WantInputFocus( v);
	    }
	    }
    }
    (n)->RemoveMark( mke);
    delete mke;
}
    
boolean nessm::InitializeClass() {
    proctable::DefineProc("nessm-make-macro", (proctable_fptr)DoConv, ATK::LoadClass("view"), "nessm", "Converts a keyboard macro to ness code.");
    return TRUE;
}

