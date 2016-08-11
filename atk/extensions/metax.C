/*********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved      *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/metax.C,v 3.5 1995/11/13 18:41:59 robr Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("metax.H")

#include <stdio.h>

#include <metax.H>

#include <text.H>
#include <proctable.H>
#include <view.H>
#include <message.H>
#include <im.H>
#include <observable.H>
#include <buffer.H>
#include <frame.H>
#include <frameview.H>
#include <keystate.H>
#include <keymap.H>
#include <completion.H>
#include <environ.H>
#include <framemessage.H>
#include <style.H>
#include <fontdesc.H>

static class style *fixed=NULL,*boldulined=NULL,*heading=NULL,*columns=NULL;

struct helpRock {
    message_workfptr HelpWork;
    class text *text;
    long rock;
    char *partial;
};

static char spaces[]="                               ";
#define MAXFUNLEN sizeof(spaces)


ATKdefineRegistry(metax, ATK, metax::InitializeClass);
#ifndef NORCSID
#endif
static void LoadClass(char  *partial);
static long match(struct proctable_Entry  *pe,struct helpRock  *h);
static void helpProc(char  *partial,struct helpRock  *myrock,message_workfptr  HelpWork,long  rock);
static char *GetProcName(struct proctable_Entry  *pe);
static boolean myCompletionWork(struct proctable_Entry  *pe, struct result  *data);
static enum message_CompletionCode mycomplete(char  *partial, long  dummyData /* Just along for the ride... */, char  *buffer, int  bufferSize);
static long donothing(class view  *vw);
static proctable_fptr dofunc(char  *func);
static boolean getfunction(class view  *v,char  *buf,int  size,char  *prompt,char  *initial);
static boolean getarg(class view  *v,char  *arg,int size,char  *prompt,char  *initial,long  *result);


static void LoadClass(char  *partial)
{
#if 0
    char class_c[100];
    int i;
    for(i = 0; (i < strlen(partial)) && (partial[i] != '-'); i++) class_c[i] = partial[i];
    if(i <= 99) {
	class_c[i]=0;
	(void) ATK::LoadClass(class_c);
    }
#else
    proctable::Preload(partial);
#endif
}

static long match(struct proctable_Entry  *pe,struct helpRock  *h)
{
    char buf[1024];
    int len;
    char *name=proctable::GetName(pe);
    char *doc=proctable::GetDocumentation(pe);
    if(!name) name="";
    if(!doc) doc="";
    if(!strncmp(name, h->partial, strlen(h->partial))) {
	memset(buf,0, sizeof(buf));
	strncpy(buf,name,254);
	len=strlen(buf);
	if(len<MAXFUNLEN) strcat(buf,spaces+len);
	strcat(buf," ");
	strncat(buf,doc,737);
	strcat(buf,"\n");
	(*h->HelpWork)(h->rock,message_HelpGenericItem,buf,NULL);
    }
    return 0;
}


static void helpProc(char  *partial,struct helpRock  *myrock,message_workfptr  HelpWork,long  rock)
{
    class text *t=myrock->text;
    if(!HelpWork) return;
    (*HelpWork)(rock,
		    message_HelpGenericItem,
		    "Proctable Listing\nName\t\t\t\tDocumentation\n",
		    NULL);
    
    myrock->HelpWork=HelpWork;
    myrock->rock=rock;
    myrock->partial=partial;
    (void) LoadClass(partial);
    (void)proctable::Enumerate((proctable_efptr)match,(char *)myrock);
    if(!fixed) return;
    (t)->SetGlobalStyle(fixed);
    (t)->AddStyle(0,17,heading);
    (t)->AddStyle(18,4,boldulined);
    (t)->AddStyle(26,13,boldulined);
    (t)->AddStyle(39,(t)->GetLength()-39,columns);
}

static char *GetProcName(struct proctable_Entry  *pe)
{
    return proctable::GetName(pe)?proctable::GetName(pe):"";
}

static boolean myCompletionWork(struct proctable_Entry  *pe, struct result  *data)
{
    completion::CompletionWork(GetProcName(pe), data);
    return FALSE;
}

static enum message_CompletionCode mycomplete(char  *partial, long  dummyData /* Just along for the ride... */, char  *bufout, int  bufoutSize)
                {
    struct result result;
    char textBuffer[1024];
    *textBuffer = 0;
    result.partial = partial;
    result.partialLen = strlen(partial);
    result.bestLen = 0;
    result.code = message_Invalid;
    result.best = textBuffer;
    result.max = sizeof(textBuffer) - 1; /* Leave extra room for a NUL. */
    (void) LoadClass(partial);
    (void)proctable::Enumerate((proctable_efptr)myCompletionWork, (char *)&result);

    strncpy(bufout, result.best, bufoutSize);
    if (result.bestLen == bufoutSize) /* Now make sure bufout ends in a NUL. */
        bufout[result.bestLen] = 0;
    return result.code;
}

static long donothing(class view  *vw)
{  
    message::DisplayString(vw, 0, "No such function.");
    im::ForceUpdate();
    return 0;
}

static proctable_fptr dofunc(char *func)
{
    struct proctable_Entry *proc = proctable::Lookup(func);
    if(proc) return proctable::GetFunction(proc);
    else return (proctable_fptr)donothing;
}

static boolean getfunction(class view  *v,char  *buf,int  size,char  *prompt,char  *initial)
{
    struct helpRock myrock;
    class framemessage *fmsg=(class framemessage *)(v)->WantHandler("message");
    if(!fmsg || !ATK::IsTypeByName((fmsg)->GetTypeName(),"framemessage")) return FALSE;

    if(fmsg) {
	class buffer *b=(fmsg->frame)->GetHelpBuffer();
	myrock.text=(class text *)(b)->GetData();
    } else myrock.text=NULL;
    
    if(message::AskForStringCompleted(v, 0, prompt, initial, buf, size, NULL, (message_completionfptr)mycomplete, (message_helpfptr)helpProc, (long)&myrock, message_MustMatch | (initial?0:message_NoInitialString))) {
	message::DisplayString(v,0,"Cancelled");
	return FALSE;
    }
    return TRUE;
}

static boolean getarg(class view  *v,char  *arg,int size,char  *prompt,char  *initial,long  *result)
{

    if(message::AskForString(v, 0,prompt, initial, arg, size) != 0) return FALSE;
    switch(*arg) {
	case '"':
	    *result=(long)arg+1;
	    break;
	case '#':
	    *result=(long)atol(arg+1);
	    break;
	case '\'':
	    *result=(long) *(arg+1);
	    break;
	default:
	    *result=(long)arg;
    }
    return TRUE;
}

static void metax1(class view  *tv,long  argument)
{
    char cbuf[500];
    struct proctable_Entry *proc;
    class im *im = (tv)->GetIM();
    if(!getfunction(tv,cbuf,sizeof(cbuf),"Function: ",NULL)) return;
    proc = proctable::Lookup(cbuf);
    if(proc) {
	switch((im->keystate)->DoProc( proc, argument, tv)) {
	case keystate_NoProc:
	    message::DisplayString(im, 0, "Could not load procedure");
	    break;
	case keystate_TypeMismatch:
	    message::DisplayString(im, 0, "Bad command");
	    break;
	}
    }
    else
	donothing((class view *)tv);
}

static void metax2(class view  *tv,long  argument)
{
    char cbuf[500], arg[500];
    struct proctable_Entry *proc;
    class im *im = (tv)->GetIM();
    if(!getfunction(tv,cbuf,sizeof(cbuf),"Function: ",NULL)) return;
    if(!getarg(tv,arg,sizeof(arg),"Argument: ",NULL,&argument))
	return;
    proc = proctable::Lookup(cbuf);
    if(proc) {
	switch((im->keystate)->DoProc( proc, argument, tv)) {
	case keystate_NoProc:
	    message::DisplayString(im, 0, "Could not load procedure");
	    break;
	case keystate_TypeMismatch:
	    message::DisplayString(im, 0, "Bad command");
	    break;
	}
    }
    else
	donothing((class view *)tv);
}

boolean metax::InitializeClass()
{
    struct ATKregistryEntry  *info = ATK::LoadClass("view");
    fixed=new style;
    if(!fixed) return FALSE;
    boldulined=new style;
    if(!boldulined) {
	delete fixed;
	return FALSE;
    }
    heading=new style;
    if(!heading) {
	delete fixed;
	delete boldulined;
	return FALSE;
    }
    columns=new style;
    if(!columns) {
	delete heading;
	delete fixed;
	delete boldulined;
	return FALSE;
    }
    
    (columns)->SetNewLeftMargin( style_LeftMargin, 550000, style_CM); 
    (columns)->SetNewIndentation( style_LeftMargin, -550000, style_CM);
    (heading)->SetFontSize(style_ConstantFontSize,20);
    (heading)->SetJustification(style_Centered);
    (fixed)->SetFontFamily( "AndyType");
    (fixed)->AddNewFontFace( fontdesc_Fixed);
    (fixed)->SetFontSize(style_ConstantFontSize,10);
    (fixed)->Copy(boldulined);
    (boldulined)->AddUnderline();
    (boldulined)->AddNewFontFace(fontdesc_Bold);
    proctable::DefineProc("metax", (proctable_fptr)metax1, info, NULL, "Executes a proctable function by name.");
    proctable::DefineProc("metax-with-arg", (proctable_fptr)metax2, info, NULL, "Executes a function by name, prompting for an argument.");
    return TRUE;
}
