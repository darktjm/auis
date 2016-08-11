/* 
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/nessmv.C,v 1.6 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


#include <andrewos.h>
ATK_IMPL("nessmv.H")

#include <menulist.H>
#include <proctable.H>
#include <bind.H>
#include <completion.H>
#include <buffer.H>
#include <im.H>
#include <frame.H>
#include <framemessage.H>
#include <message.H>
#include <text.H>
#include <cursor.H>

#include <ness.H>
#include <nessmv.H>

static class cursor *waitcursor=NULL;

static int waitcount=0;


ATKdefineRegistry(nessmv, nessview, nessmv::InitializeClass);

static void WaitOn();
static void WaitOff();
static boolean DoAppend(class nessmv  *self, class ness  *src, class ness  *dest);
static void UpdateFile(class nessmv  *self, class ness  *src, class buffer  *b, char  *filename);
static char *GetName(class ness  *n);
static long match(class ness  *n,struct helpRock  *h);
static void helpProc(char  *partial, struct helpRock  *myrock,
		message_workfptr  HelpWork, long  rock);
static enum message_CompletionCode mycomplete(char  *partial, 
		struct helpRock  *myrock, char  *buffer, int  bufferSize);
static long AskForScript(class nessmv  *self, char  *prompt, char  *buf, long  bufsiz);
static void ScriptAppend(class nessmv  *self, long  rock);
static void Append(class nessmv  *self, long  rock);
static void Visit(class nessmv  *self, long  rock);


static void WaitOn() {
    class im *last=im::GetLastUsed();
    waitcount++;

    if(waitcount==1) {

	if(last==NULL || waitcursor!=NULL) return;

	waitcursor=cursor::Create(last);

	if(waitcursor!=NULL) {
	    (waitcursor)->SetStandard( Cursor_Wait);
	    im::SetProcessCursor(waitcursor);
	}
	
    }
}

static void WaitOff() {
    waitcount--;
    if(waitcount<=0) {
	waitcount=0;
	if(waitcursor!=NULL) {
	    im::SetProcessCursor(NULL);
	    delete waitcursor;
	    waitcursor=NULL;
	}
    }
}


static boolean DoAppend(class nessmv  *self, class ness  *src, class ness  *dest) {
    long end;
    (dest)->SetAccessLevel( ness_codeUV);
    (dest)->SetNeedsDialogBox( FALSE);
    (dest)->SetReadOnly( FALSE);

    (src)->SetAccessLevel( ness_codeUV);
    (src)->SetNeedsDialogBox( FALSE);
    
    if(!src->compiled) {
	WaitOn();
	message::DisplayString(self, 0, "Testing compilability...");

	im::ForceUpdate();

	if((src)->Compile()) {
	    (src)->Expose();
	    (src)->printerrors(stderr);
	    message::DisplayString(self, 0, "");
	    message::DisplayString(self, 100, "Error recompiling!");
	    WaitOff();
	    return FALSE;
	}
	WaitOff();
    }

    end=(dest)->GetLength();

    if(end>0 && (dest)->GetChar( end-1)!='\n') (dest)->InsertCharacters( end, "\n", 1);

    (dest)->CopyTextExactly( end+1, src, 0, (src)->GetLength());

    end=(dest)->GetLength();

    if(end>0 && (dest)->GetChar( end-1)!='\n') (dest)->InsertCharacters( end, "\n", 1);
    
    WaitOn();
    message::DisplayString(self, 0, "Re-compiling.");

    im::ForceUpdate();

    if((dest)->Compile()) {
	(dest)->Expose();
	(dest)->printerrors(stderr);
	message::DisplayString(self, 0, "");
	message::DisplayString(self, 100, "Error recompiling! Some bindings may be unavailable.");
	WaitOff();
	return FALSE;
    }
    
    WaitOff();
    
    return TRUE;
}

static void UpdateFile(class nessmv  *self, class ness  *src, class buffer  *b, char  *filename) {
    class buffer *ob;
    WaitOn();
    message::DisplayString(self, 0, "Saving...");
    im::ForceUpdate();

    if((b)->WriteToFile( filename, buffer_ReliableWrite|buffer_MakeBackup)<0) {
	message::DisplayString(self, 0, "");
	message::DisplayString(self, 100, "Write failed!");
	WaitOff();
	return;
    }
    (b)->SetIsModified( FALSE);
    WaitOff();

    message::DisplayString(self, 0, "Done re-compiling and saving.");

    ob=buffer::FindBufferByData(src);

    if(ob!=NULL) {
	class frame *f=frame::GetFrameInWindowForBuffer(ob);
	if(f!=NULL) {
	    (f)->SetBuffer( b, TRUE);
	    (b)->SetIsModified( FALSE);
	    (ob)->Destroy();
	}
    }
}

struct helpRock {
    message_workfptr HelpWork;
    class text *text;
    long rock;
    char *partial;
    class ness *n;
};


static char *GetName(class ness  *n) {
    char *name=(char *)(n)->GetName(), *p;
    if(name) return name;
    name=(n)->GetFilename();
    if(name==NULL) return NULL;
    p=strrchr(name, '/');
    if(p) return p+1;
    else return name;
}
    
static long match(class ness  *n,struct helpRock  *h) {
    char buf[1024];
    int len;
    char *name=GetName(n);
    if(name==NULL) return 0;
    printf("blah:'%s'\n",name);
    if(!strncmp(name, h->partial, strlen(h->partial))) {
	memset(buf,0, sizeof(buf));
	strncpy(buf,name,sizeof(buf)-1);
	(*h->HelpWork)(h->rock, message_HelpListItem, buf, 0);
    }
    return 0;
}


static void helpProc(char  *partial, struct helpRock  *myrock,
		message_workfptr  HelpWork, long  rock) {
    class text *t=myrock->text;
    class ness *n=ness::GetList();
    if(!HelpWork) return;
    (*HelpWork)(rock,
		    message_HelpGenericItem,
		    "Ness Scripts\n",
		    NULL);
    
    myrock->HelpWork=HelpWork;
    myrock->rock=rock;
    myrock->partial=partial;
    while(n) {
	if(n!=myrock->n) match(n, myrock);
	n=n->next;
    }
}


static enum message_CompletionCode mycomplete(char  *partial, 
		struct helpRock  *myrock, char  *buffr, int  buffrSize) {
    struct result result;
    char textbuffr[1024];
    class ness *n=ness::GetList();
    *textbuffr = 0;
    result.partial = partial;
    result.partialLen = strlen(partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textbuffr;
    result.max = sizeof(textbuffr) - 1; /* Leave extra room for a NUL. */
    while(n) {
	if(n!=myrock->n && GetName(n)) completion::CompletionWork(GetName(n), &result);
	n=n->next;
    }
    strncpy(buffr, result.best, buffrSize);
    if (result.bestLen == buffrSize) /* Now make sure buffr ends in a NUL. */
	buffr[result.bestLen] = 0;
    return result.code;
}

static char sbuf[1024]=".atkmacros";

static long AskForScript(class nessmv  *self, char  *prompt, char  *buf, long  bufsiz) {
    class ness *n=ness::GetList();
    struct helpRock myrock;
    class framemessage *fmsg=(class framemessage *)(self)->WantHandler("message");
    class buffer *b;
    if(!fmsg || !ATK::IsTypeByName((fmsg)->GetTypeName(),"framemessage")) return -1;

    b=(fmsg->frame)->GetHelpBuffer();
    if(b!=NULL) myrock.text=(class text *)(b)->GetData();
    else return -1;

    myrock.n=(class ness *)(self)->GetDataObject();

    /* does the default exist? */
    while(n) {
	char *sn=GetName(n);
	if(sn && strcmp(sn, sbuf)==0) break;
	n=n->next;
    }

    if(message::AskForStringCompleted(self, 0, prompt, n?sbuf:NULL, 
		buf, bufsiz, NULL, (message_completionfptr)mycomplete, 
		(message_helpfptr)helpProc, (long)&myrock, 
		message_MustMatch)) {
	message::DisplayString(self,0,"Cancelled.");
	return -1;
    }
    strcpy(sbuf, buf);
    return 0;
}

static void ScriptAppend(class nessmv  *self, long  rock) {
    char buf[1024];
    char *name=NULL;
    class ness *n=ness::GetList();
    
    class buffer *b;
    
    if(rock>255) name=(char *)rock;
    
    if(name==NULL) {
	if(AskForScript(self, "Append to script:", buf, sizeof(buf))<0) return;
	name=buf;
    }
    
    while(n) {
	char *sn=GetName(n);
	if(sn && strcmp(sn, name)==0) break;
	n=n->next;
    }
    
    if(n==NULL) {
	message::DisplayString(self, 0, 
		"Couldn't find the specified Ness script.");
	return;
    }
    
    if(!DoAppend(self, (class ness *)(self)->GetDataObject(), n)) return;
 
    if((n)->GetFilename()==NULL) {
	message::DisplayString(self, 0, "Ness has no associated file, changes have NOT been saved.");
	return;
    }
    
    b=buffer::FindBufferByData(n);

    /* If we create the buffer here it will NOT
      destroy it's data when it goes away. */
    if(b==NULL) b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);

    if(b==NULL) {
	message::DisplayString(self, 0, "Couldn't get a buffer on the Ness, changes have NOT been saved.");
	return;
    }

    UpdateFile(self, (class ness *)(self)->GetDataObject(), b, (n)->GetFilename());
}

static char dbuf[1024]="~/.atkmacros";
 
static char *appendchoices[]={
    "Cancel",
    "Update the existing script. (discard file)",
    "Update the file.",
    NULL
};


static void Append(class nessmv  *self, long  rock) {
    char buf[1024];
    char *filename=NULL;
    class buffer *b, *ob;
    ATK  *data;
    class ness *dest, *src=(class ness *)(self)->GetDataObject();
    class ness *n=ness::GetList();
    long result=2;
    
    if(rock>255) filename=(char *)rock;
    if(filename==NULL) {
	if(completion::GetFilename(self, "Append to file:", dbuf, buf, sizeof(buf), FALSE, FALSE)<0) {
	    message::DisplayString(self, 0, "Cancelled.");
	    return;
	}
	filename=buf;
    }

    strcpy(dbuf, filename);

    while(n) {
	if((n)->GetFilename() && strcmp((n)->GetFilename(), dbuf)==0) break;
	n=n->next;
    }

    /* If the buffer already exists we leave it's data destroying flag intact. */
    b=buffer::FindBufferByFile(filename);

    if(n && !(b && (b)->GetData()==(class dataobject *)n)) {
	if(message::MultipleChoiceQuestion(self, 100, "Warning a Ness Script with that filename is currently active.", 0, &result, appendchoices, NULL)<0 || result==0) {
	    message::DisplayString(self, 0, "Cancelled!");
	    return;
	}
    }

    switch(result) {
	case 1: /* visiting existing script. */
	    b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);
	    break;
	case 2: /* visiting file. */

	    if(b==NULL) {
		b=buffer::GetBufferOnFile(filename, 0);
		/* If we created the buffer we don't want the data to go away when the buffer does. */
		if(b!=NULL) (b)->SetDestroyData( FALSE);
	    }

    }
    
    if(b==NULL) {
	message::DisplayString(self, 0, "Couldn't get a buffer on the specified file.");
	return;
    }
    
    if((b)->GetReadOnly()) {
	message::DisplayString(self, 0, "File is read only. Append cancelled.");
	(b)->Destroy();
	return;
    }
    
    data=(ATK  *)(b)->GetData();
    if(strcmp((data)->GetTypeName(), "ness")!=0) {
	message::DisplayString(self, 0, "The named file is not a ness file.");
	(b)->Destroy();
	return;
    }
    
    dest=(class ness *)data;

    if(!DoAppend(self, src, dest)) return;

    UpdateFile(self, src, b, filename);
}

static void Visit(class nessmv  *self, long  rock) {
    class ness *n=ness::GetList();
    char buf[1024];
    class buffer *b;
    class frame *f;
    char *name=NULL;

    if(rock>255) name=(char *)name;
    
    if(name==NULL) {
	if(AskForScript(self, "Visit script:", buf, sizeof(buf))<0) return;
	else name=buf;
    }
    
    while(n) {
	char *sn=GetName(n);
	if(sn && strcmp(sn, name)==0) break;
	n=n->next;
    }
    
    if(n==NULL) {
	message::DisplayString(self, 0, "Couldn't find the specified Ness script.");
	return;
    }

    b=buffer::FindBufferByData(n);
    
    if(b==NULL) b=buffer::Create(GetName(n), (n)->GetFilename(), NULL, n);

    if(b==NULL) {
	message::DisplayString(self, 0, "Couldn't get a buffer on the selected Ness script.");
	return;
    }

    f=frame::GetFrameInWindowForBuffer(b);

    if(f==NULL) {
	message::DisplayString(self, 0, "Couldn't get a window on the selected Ness script.");
	return;
    }
    if(f!=NULL) {
	class view *v=(f)->GetView();
	if(v!=NULL) (v)->WantInputFocus( v);
    }
}
    
    
static struct bind_Description Bindings[]={
	{"nessmv-visit-script", NULL, 0, "Ness,Visit Script~50", 0, 0,  
			(proctable_fptr)Visit, 
			"Visit an existing Ness script." },
	{"nessmv-append-to-file", NULL, 0, "Ness,Append to File~40", 0, 0, 
			(proctable_fptr)Append, 
			"Append the current buffer to a Ness file." },
	{"nessmv-append-to-script", NULL, 0, "Ness,Append to Script~45", 0, 0,
			(proctable_fptr)ScriptAppend, 
			"Append the current buffer to a Ness script." },
    NULL
};

static class menulist *menus=NULL;

boolean nessmv::InitializeClass() {
    menus = new menulist;
    bind::BindList(Bindings, NULL , menus, &nessmv_ATKregistry_ );
    return TRUE;
}

nessmv::nessmv() {
	ATKinit;
 
    this->ml=(menus)->DuplicateML( this);
    
    if(this->ml==NULL) THROWONFAILURE( FALSE);

    THROWONFAILURE( TRUE);
}

nessmv::~nessmv() {
	ATKinit;

    if(this->ml) delete this->ml;
    this->ml=NULL;
}

void nessmv::PostMenus(class menulist  *ml) {
    (this->ml)->ClearChain();
    if(ml && ml!=this->ml) {
	(this->ml)->ChainAfterML( ml, 0);
    }
    (this)->nessview::PostMenus( this->ml);
}

/*
 *    $Log: nessmv.C,v $
 * Revision 1.6  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.5  1994/08/12  20:43:39  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.4  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.3  1993/07/23  00:23:42  rr2b
 * Split off a version of CopyText which will copy surrounding
 * styles as well as embedded styles.
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.5  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.4  1992/12/14  20:50:00  rr2b
 * disclaimerization
 *
 * Revision 1.3  1992/11/26  02:42:25  wjh
 * converted CorrectGetChar to GetUnsignedChar
 * moved ExtendShortSign to interp.h
 * remove xgetchar.h; use simpletext_GetUnsignedChar
 * nessrun timing messages go to stderr
 * replaced curNess with curComp
 * replaced genPush/genPop with struct compilation
 * created compile.c to handle compilation
 * moved scope routines to compile.c
 * converted from lex to tlex
 * convert to use lexan_ParseNumber
 * truncated logs to 1992 only
 * use bison and gentlex instead of yacc and lexdef/lex
 *
 * .
 *
 * Revision 1.2  92/06/05  18:39:48  gk5g
 * Fixed some small oversights.
 * .
 * 
 * Revision 1.1  1992/06/05  17:28:27  rr2b
 * Initial revision
 *
 */

