/* Copyright 1994 Carnegie Mellon University All rights reserved.
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
ATK_IMPL("unknownv.H")
#include <menulist.H>
#include <bind.H>
#include <completion.H>
#include <observable.H>
#include <text.H>
#include <unknown.H>
#include <message.H>
#include <unknownv.H>
#include <util.h>

static class menulist *umenus=NULL;

ATKdefineRegistry(unknownv, textview, unknownv::InitializeClass);

static long SaveRaw(ATK   *aself, long  rock)
{
    class unknownv *self=(class unknownv *)aself;
    class unknown *dself=(class unknown *)(self)->GetDataObject();
    if(dself->odata==NULL) {
	message::DisplayString(self, 0, "The unavailable inset had no data apparently.");
    } else {
	long pos=0, len;
	char buf[1024];
	FILE *fp=NULL;
	int res;
	buf[0]='\0';
	res=completion::GetFilename(self, "File for raw inset data:", "/tmp/", buf, sizeof(buf), FALSE, FALSE);
	if(res!=0) {
	    message::DisplayString(self, 0, "Cancelled.");
	    return 0;
	}
	fp=fopen(buf, "w");
	if(fp==NULL) {
	    message::DisplayString(self, 100, "Couldn't open the file for writing.");
	    return 0;
	}

	    /* write out the data verbatim! */
	while(pos<(dself->odata)->GetLength()) {
	    char *buf=(dself->odata)->GetBuf( pos, (dself->odata)->GetLength()-pos, &len);
	    if(len==0 || (long)fwrite(buf, 1, len, fp)!=len) {
		message::DisplayString(self, 100, "There was an error writing out the data.");
		return 0;
	    }
	    pos+=len;
	}
	if(fclose(fp)!=0) {
	    message::DisplayString(self, 100, "There was an error writing out the data.");
	    return 0;
	}
	message::DisplayString(self, 0, "The raw inset data has been written.");
    }
    return 0;
}

static const char sepline[]="### Raw Inset Data, do not edit ###\n";
static long DisplayRaw(ATK   *aself, long  rock)
{
    class unknownv *self=(class unknownv *)aself;
    class unknown *dself=(class unknown *)(self)->GetDataObject();
    if(dself->odata) {
	(dself)->AlwaysInsertCharacters( (dself)->GetLength(), sepline, strlen(sepline));
	(dself)->AlwaysCopyText( (dself)->GetLength(), dself->odata, 0, (dself->odata)->GetLength());
	(dself)->NotifyObservers( observable_OBJECTCHANGED);
	message::DisplayString(self, 0, "Inserted raw data for unavailable inset.");
    } else {
	message::DisplayString(self, 0, "The unavailable inset had no data apparently.");
    }
    return 0;
}

static long Help(ATK *aself, long rock) {
    char buf[1024];
    sprintf(buf, "%s/help unknown", AndrewDir("/bin"));
    if(system(buf)==0) {
	message::DisplayString((view *)aself, 0, "The help window should appear soon.");
    } else {
	message::DisplayString((view *)aself, 100, "Unable to start the command 'help  unknown'.");
    }
    return 0;
}


static const char * const choices[]={
    "Show Help",
    "Proceed",
    "Cancel",
    NULL
};


static void ExecuteRecovery(class unknownv *self) {
    const char *path=self->WantInformation("filename");
    if(path==NULL) {
	message::DisplayString(self, 100, "Unable to begin conversion automatically.  Please see help unknown.");
	return;
    }
    char buf[1024];
    while(1) {
	int res;
	buf[0]='\0';
	res=completion::GetFilename(self, "Filename for recovered file:", "/tmp/", buf, sizeof(buf), FALSE, FALSE);
	if(res!=0) {
	    message::DisplayString(self, 0, "Cancelled.");
	    return;
	}
	if(strcmp(buf, path)==0) {
	    message::DisplayString(self, 100, "The filename for the recovered file must be different than the original filename.");
	    continue;
	} else break;
    }
    char cbuf[4096];
    sprintf(cbuf, "%s/recover<%s>%s", AndrewDir("/bin"), path, buf);
    if(system(cbuf)!=0) {
	message::DisplayString(self, 100, "The recovery program has failed, please see help unknown.");
	return;
    }
    sprintf(cbuf, "%s/ez %s", AndrewDir("/bin"), buf);
    char cbuf2[2048];
    while(1) {
	int res=message::AskForString(self, 100, "Command to test the recovered file:", cbuf, cbuf2, sizeof(cbuf2));
	if(res<0) {
	    message::DisplayString(self, 0, "Cancelled running test on recovered file.");
	    return;
	}
	if(system(cbuf2)!=0) {
	    message::DisplayString(self, 100, "The test command seems to have failed. To try again click continue and try a different command.  Otherwise simply cancel the command prompt after clicking continue.");
	} else {
	    message::DisplayString(self, 100, "The test command may have succeeded, if you didn't get a window see help unknown.");
	    return;
	}
    }
    
}

static long Recover(ATK *aself, long rock) {
    class unknownv *uv=(class unknownv *)aself;
    long choice=2;
    do {
	long res=message::MultipleChoiceQuestion(uv, 100, "Please read the help file before proceeding.", 2, &choice, choices, NULL);
	if(res<0) {
	    message::DisplayString(uv, 0, "Cancelled.");
	    return 0;
	}
	switch(choice) {
	    case 0:
		Help(aself, 0);
		break;
	    case 1:
		ExecuteRecovery(uv);
		return 0;
	    case 2:
		message::DisplayString(uv, 0, "Cancelled.");
		return 0;
	}
    } while(1);
}

static struct bind_Description  ubind[]={
    {"unknownv-save-raw", NULL, 0, "Unknown~30,Save Raw Data~30", 0, 0, SaveRaw, "Saves the raw data for an unavailable inset.", NULL},
    {"unknownv-display-raw", NULL, 0, "Unknown~30,Display Raw Data~31", 0, 0, DisplayRaw, "Displays the raw data for an unavailable inset.", NULL},
    {"unknownv-help", NULL, 0, "Unknown~30,Help~1", 0, 0, Help, "Displays the help file for the unknown inset.", NULL},
    {"unknownv-recover", NULL, 0, "Unknown~30,Attempt Recovery~20", 0, 0, Recover, "Attempts to recover the original form of a document with unknown insets in it.", NULL},
    {NULL, NULL, 0, NULL, 0, 0, NULL, NULL, NULL}
};

boolean unknownv::InitializeClass()
{
    umenus=new menulist;
    if(umenus==NULL) return FALSE;
    bind::BindList(ubind, NULL, umenus, &unknownv_ATKregistry_ );
    return TRUE;
}

unknownv::unknownv()
{
	ATKinit;

    this->menus=(umenus)->DuplicateML( this);
    if(this->menus) THROWONFAILURE( TRUE);
    else THROWONFAILURE( FALSE);
}

unknownv::~unknownv()
{
	ATKinit;

    if(this->menus) delete this->menus;
}

void unknownv::PostMenus(class menulist  *menus)
{
    (this->menus)->UnchainML( (long)this);
    if(menus) (this->menus)->ChainBeforeML( menus, (long)this);
    (this)->textview::PostMenus( this->menus);
}
