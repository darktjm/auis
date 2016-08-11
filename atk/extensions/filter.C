/*LIBS: -lutil
*/

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/filter.C,v 3.6 1994/11/30 20:42:06 rr2b Stab74 $";
#endif

/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

 

/* filter the textview selection region through a command
 */


#include <andrewos.h>
ATK_IMPL("filter.H")
#include <filter.H>

#include <im.H>
#include <proctable.H>
#include <textview.H>
#include <text.H>
#include <message.H>

#include <util.h>

#define IN 1
#define OUT 2
#define FORMAT 4

struct filterdata{
    FILE *infp,*outfp;
/*
    struct region source,sink;
*/
    class textview *v;
    class text *t;
    int pos,len;
    short method;
};


ATKdefineRegistry(filter, ATK, filter::InitializeClass);

static void commandFinished(int  pid,struct filterdata  *fd, WAIT_STATUS_TYPE *status);
static void filterf(class textview  *tv,char  *command,short  method);
static void filterRegion(class textview  *tv);
static void filterRegionFmt(class textview  *tv);
static void filterRegionThruCommand(class textview  *tv,char  *command);
static void filterRegionThruCmdFmt(class textview  *tv,char  *command);
static void sinkRegionThruCommand(class textview  *tv,char  *command);
static void sinkRegion(class textview  *tv);
static void sinkRegionThruCmdFmt(class textview  *tv,char  *command);
static void sinkRegionFmt(class textview  *tv);


static void commandFinished(int  pid,struct filterdata  *fd, WAIT_STATUS_TYPE  *status)
{
    char buf[40],*em;

    strcpy(buf,"Filtering...process ");
    em=statustostr(status,buf+20,20);

    if(em==NULL){ /* sucessful completion */
	if(fd->method&OUT){
	    int oldlen;
	    (fd->t)->DeleteCharacters(fd->pos,fd->len);
	    oldlen=(fd->t)->GetLength();
	    rewind(fd->outfp);
            if (fd->method & FORMAT)
                (fd->t)->InsertFile(fd->outfp,NULL,fd->pos);
            else
                (fd->t)->ReadSubString(fd->pos,fd->outfp,FALSE);
	    (fd->v)->SetDotLength((fd->t)->GetLength()-oldlen);
	}
	message::DisplayString(fd->v,0,"Filtering...done.");
    }else
	message::DisplayString(fd->v,1,buf);

    (fd->t)->NotifyObservers(0);

    fclose(fd->infp);
    fclose(fd->outfp);

    free((char *)fd);
}

static void filterf(class textview  *tv,char  *command,short  method)
{
    static int count=0,pid;
    char *argvbuf[100],**argv;
    char buf[100];
    struct filterdata *fd=(struct filterdata *)malloc(sizeof(struct filterdata));

/*    if(im_ArgProvided(textview_GetIM(tv))){
	struct buffer *b=buffer_CreateTemp();
	sprintf(buf,"To buffer: [%s] ",buffer_GetName(b));
	message_AskForString(tv,0,buf,NULL,buf,sizeof(buf));
	
    }*/

    if(fd==NULL){
	message::DisplayString(tv,1,"Malloc failed.");
	return;
    }

    fd->v=tv;
    fd->t=(class text *)tv->dataobject;
    fd->pos=(tv)->GetDotPosition();
    fd->len=(tv)->GetDotLength();
    fd->method=method;

    sprintf(buf,"/tmp/filter.%d.%d",getpid(),count++);

    if(method&IN)
	fd->infp=fopen(buf,"w+");
    else
	fd->infp=fopen("/dev/null","r");
    if(fd->infp==NULL){
	char mbuf[200];
	sprintf("Can't open %s.",buf);
	message::DisplayString(tv,1,mbuf);
	return;
    }
    if(method&IN)
	unlink(buf);

    if(method&OUT)
	fd->outfp=fopen(buf,"w+");
    else
	fd->outfp=fopen("/dev/null","w");
    if(fd->outfp==NULL){
	char mbuf[200];
	sprintf("Can't open %s.",buf);
	message::DisplayString(tv,1,mbuf);
	fclose(fd->infp);
	return;
    }
    if(method&OUT)
	unlink(buf);

    if(method&IN){
	if(method&FORMAT) {
	    class text *selected=new text;
	    class style *oldglobsty=fd->t->GetGlobalStyle();
	    if(selected) {
		selected->AlwaysCopyTextExactly(0, fd->t, fd->pos, fd->len);
		if(oldglobsty) {
		    class style *globsty=new style;
		    oldglobsty->Copy(globsty);
		    globsty->template_c=FALSE;
		    selected->styleSheet->Add(globsty);
		    selected->SetGlobalStyle(globsty);
		}
		selected->Write(fd->infp, im::GetWriteID(), 1);
		selected->Destroy();
	    } else (fd->t)->WriteSubString(fd->pos,fd->len,fd->infp,FALSE);
	} else (fd->t)->WriteSubString(fd->pos,fd->len,fd->infp,FALSE);
	fflush(fd->infp);
	rewind(fd->infp);
    }

    argv=strtoargv(command,argvbuf,100);

    switch(pid = osi_vfork()){
	case 0:
	    close(0);
	    close(1);
	    dup(fileno(fd->infp));
	    dup(fileno(fd->outfp));
#if 0
	    {
		char **p;
		write(2,"Execing ",8);
		for(p=argv;*p!=NULL;p++){
		    write(2,*p,strlen(*p));
		    write(2," ",1);
		}
		write(2,"\n",1);
	    }
#endif /* 0 */
	    execvp(*argv,argv);
	    exit(0);
	    break;
	case -1:
	    message::DisplayString(tv,1,"Fork failed.");
	    return;
	default:
	    im::AddZombieHandler(pid, (im_zombiefptr) commandFinished,(long)fd);
	    message::DisplayString(tv,0,"Filtering...");
    }
}

static void filterRegion(class textview  *tv)
{
    char cbuf[500];
    if(message::AskForString(tv,0,"Command: ",NULL,cbuf,sizeof(cbuf))!=0)
	return;
    filterf(tv,cbuf,IN+OUT);
}

static void filterRegionFmt(class textview  *tv)
{
    char cbuf[500];
    if (message::AskForString(tv,0,"Command: ",NULL,cbuf,sizeof(cbuf))!=0)
        return;
    filterf(tv,cbuf,IN+OUT+FORMAT);
}

static void filterRegionThruCommand(class textview  *tv,char  *command)
{
    char cbuf[500];
    strcpy(cbuf,command); /* since filter trashes its input string */
    filterf(tv,cbuf,IN+OUT);
}

static void filterRegionThruCmdFmt(class textview  *tv,char  *command)
{
    char cbuf[500];
    strcpy(cbuf,command); /* since filter trashes its input string */
    filterf(tv,cbuf,IN+OUT+FORMAT);
}

static void sinkRegionThruCommand(class textview  *tv,char  *command)
{
    char cbuf[500];
    strcpy(cbuf,command); /* since filter trashes its input string */
    filterf(tv,cbuf,IN);
}

static void sinkRegion(class textview  *tv)
{
    char cbuf[500];
    if(message::AskForString(tv,0,"Command: ",NULL,cbuf,sizeof(cbuf))!=0)
	return;
    filterf(tv,cbuf,IN);
}

static void sinkRegionThruCmdFmt(class textview  *tv,char  *command)
{
    char cbuf[500];
    strcpy(cbuf,command); /* since filter trashes its input string */
    filterf(tv,cbuf,IN+FORMAT);
}

static void sinkRegionFmt(class textview  *tv)
{
    char cbuf[500];
    if(message::AskForString(tv,0,"Command: ",NULL,cbuf,sizeof(cbuf))!=0)
	return;
    filterf(tv,cbuf,IN+FORMAT);
}

boolean filter::InitializeClass()
{
    struct ATKregistryEntry  *tvi=ATK::LoadClass("textview");

    if(tvi==NULL)
	return FALSE;

    proctable::DefineProc("filter-filter-region",
			  (proctable_fptr) filterRegion,tvi,NULL,"Prompts for a command and executes it with the selection region as standard input, replacing the region with the output of the command.");
    proctable::DefineProc("filter-sink-region",
			  (proctable_fptr) sinkRegion,tvi,NULL,"Prompts for a command and provides the selected region as standard input to it; the command's output is discarded.");
    proctable::DefineProc("filter-filter-region-thru-command",
			  (proctable_fptr) filterRegionThruCommand,tvi,NULL,"Executes a command with the selection region as standard input, replacing it with the output of the command.");
    proctable::DefineProc("filter-sink-region-thru-command",
			  (proctable_fptr) sinkRegionThruCommand,tvi,NULL,"Provides the selected region as standard input to the specified command; the command's output is discarded.");
    proctable::DefineProc("filter-filter-region-formatted",
			  (proctable_fptr) filterRegionFmt,tvi,NULL,"Prompts for a command and executes it with the selection region as standard input, replacing the region with the output of the command. ATK datastream is read and written.");
    proctable::DefineProc("filter-sink-region-formatted",
			  (proctable_fptr) sinkRegionFmt,tvi,NULL,"Prompts for a command and provides the selected region as standard input to it; the command's output is discarded.  ATK datastream is written.");
    proctable::DefineProc("filter-filter-region-thru-command-formatted",
			  (proctable_fptr) filterRegionThruCmdFmt,tvi,NULL,"Executes a command with the selection region as standard input, replacing it with the output of the command.  ATK datastream is read and written.");
    proctable::DefineProc("filter-sink-region-thru-command-formatted",
			  (proctable_fptr) sinkRegionThruCmdFmt,tvi,NULL,"Provides the selected region as standard input to the specified command; the command's output is discarded.  ATK datastream is written.");

    return TRUE;
}
