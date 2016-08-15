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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/extensions/RCS/tags.C,v 3.5 1996/09/03 19:12:18 robr Exp $";
#endif


 

/* * tags.c -- A tags package for the ATK editor
   *
   * Uses the output from the 'ctags' command to find
   * function definitions.
   * Currently cannot set default for tag file name.
   * Does not deal with multiple windows.
   * Now does recursive edit -- give it an argument (^U) to AVOID recursive edit
*/

ATK_IMPL("tags.H")

#include <textview.H>
#include <message.H>
#include <text.H>
#include <search.H>
#include <proctable.H>
#include <buffer.H>
#include <frame.H>
#include <dataobject.H>
#include <im.H>
#include <environ.H>
#include <filetype.H>
#include <completion.H>
#include <view.H>
#include <ctype.h>
#include <sys/param.h>
#include <tags.H>

struct SearchPattern {
    short size, used;
    unsigned char *body;
};


#define TBUFNAM "Tags-Buffer"
static const char *TagsFile = NULL;


struct finderInfo {
    class frame *myFrame, *otherFrame, *bestFrame;
    class buffer *myBuffer;
};


ATKdefineRegistry(tags, ATK, tags::InitializeClass);
#ifndef NORCSID
#endif
static boolean ViewEqual(class frame  *frame, class view  *view);
static class frame *FindByView(class view  *view);
static void checkFileName();
static class buffer *tags_OpenBuffer(const char  *filename);
static class buffer *tags_OpenTagsBuffer(const char  *name);
void tags_finished(int  pid, class view  *view, long  *status);
void tags_RebuildTagsFile(class view  *view, long  key);
long nextField(class text  *doc, long  pos);
int firstField(class text  *doc,long  pos);
char *getFunction(class text  *doc,long  pos);
class view *TagsCreateWindow(class buffer  *buffer );
void tags_FindTag (class view  *view, char  *tag, int  RecursiveEdit);
void tags_GotoTagCmd(class view  *view,long  key);
void tags_FindTagCmd(class view  *view,long  key);
void tags_OpenCmd(class view  *view,long  key);
void tags_LoadTagFileCmd(class view  *view,long  key);


static boolean ViewEqual(class frame  *frame, class view  *view)
        {

#if 1
    return ((frame)->GetView() == view);
#else /* 1 */
    return (((class view *) frame)->GetIM() == (view)->GetIM())
#endif /* 1 */
}

static class frame *FindByView(class view  *view)
    {

    return frame::Enumerate((frame_effptr)ViewEqual, (long) view);
}


static void checkFileName()
{
    if (TagsFile == NULL)
        if ((TagsFile = environ::GetProfile("tagfilename")) == NULL)
            TagsFile = "tags";

}


static class buffer *tags_OpenBuffer(const char  *filename)

{
    class buffer *rbuffer;
    long version;

    rbuffer = buffer::GetBufferOnFile(filename,buffer_MustExist);
    if (rbuffer) {
        version = ((rbuffer)->GetData())->GetModified();
        (rbuffer)->SetCkpClock(0);
        (rbuffer)->SetCkpVersion(version);
        (rbuffer)->SetWriteVersion(version);
    }
    return rbuffer;
}

/* open a buffer on the tag file*/

static class buffer *tags_OpenTagsBuffer(const char  *name)

{
    class buffer *tagsbuffer;
    long version;
    
    checkFileName();

    if ((tagsbuffer = buffer::FindBufferByName(TBUFNAM)) == NULL) {
        tagsbuffer = buffer::GetBufferOnFile(TagsFile, TRUE);
        if (tagsbuffer==NULL) return NULL;
        (tagsbuffer)->SetName(TBUFNAM);
        (tagsbuffer)->SetScratch(TRUE);
        version = ((tagsbuffer)->GetData())->GetModified();
        (tagsbuffer)->SetCkpClock(0);
        (tagsbuffer)->SetCkpVersion(version);
        (tagsbuffer)->SetWriteVersion(version);
    }
    return tagsbuffer;
}



/*
 * Zombie handler for completion of the 'tags' command.
 */
void tags_finished(int  pid, class view  *view, long  *status)
{
    int exit_status;
    class buffer *buffer;

    /* Extract exit status */
    exit_status = (status == NULL || *status & 0xff) ? -1 : (*status >> 8) & 0xff;
    if (exit_status != 0)
	message::DisplayString(view,0,"Rebuild failed");
    else {
	message::DisplayString(view,0,"Tag file rebuilt.");
	if (buffer=buffer::FindBufferByName(TBUFNAM)) {
	    (buffer)->ReadFile(TagsFile);
	}
    }
}

void tags_RebuildTagsFile(class view  *view, long  key)

{
    int pid;
#ifndef linux
    static char command[256]="ctags -w *.c *.h",
                command2[256];
#else
    static char command[256]="ctags *.c *.h",
    command2[256];
#endif
    
    checkFileName();

    if (((view)->GetIM())->ArgProvided()) {
	if (message::AskForString(view,0,"Rebuild tags with command: ",command, command2, 255))
	    return;
	strcpy(command, command2);
    }

    message::DisplayString(view,0,"Rebuilding tag file; please wait...");
    im::ForceUpdate();

    /*    sprintf(command,"ctags -w -f %s *.c", TagsFile);*/ /* should be able to use -f file */

    if ((pid = osi_vfork()) == 0) { /* in child process */
        execlp("/bin/csh","csh","-f", "-c",command,">>& /dev/console",0);
	exit(0);
    }
    im::AddZombieHandler(pid, (im_zombiefptr)tags_finished, (long)view);
}


/* skips over to the next field separated by blanks & tabs */

long nextField(class text  *doc, long  pos)

{
    char c;

    c = (doc)->GetChar(pos);
    while ((c!='\t') && (c!=' '))
        c = (doc)->GetChar(++pos);
    while ((c=='\t') || (c==' '))
        c = (doc)->GetChar(++pos);
    return pos;
}


/* returns true if pos is in the first field
   fields are separated by blanks & tabs */

int firstField(class text  *doc,long  pos)

{
    char c = (doc)->GetChar(pos);
    if ((c == ' ') || (c == '\t') || (c == '\n'))
        return 0;
    while ((c!='\n') && (pos>0)) {
        c = (doc)->GetChar(--pos);
        if ((c == ' ') || (c == '\t'))
            return 0;
    }
    return 1;
}


/* gets the function name at the dot */

char *getFunction(class text  *doc,long  pos)
{
    char c;
    static char name[256];
    char *nptr=name;
    
    c=(doc)->GetChar(pos);
    while (isalnum(c) || c=='_') 
        c=(doc)->GetChar(--pos);
    c=(doc)->GetChar(++pos);
    while (isalnum(c) || c=='_') 
        *nptr++ = c = (doc)->GetChar(pos++);
    *--nptr='\0';
    return name;
}

class view *TagsCreateWindow(class buffer  *buffer )
{
    class frame *frame;
    class im *newIM;
    class view *view;
    newIM = im::Create(NULL);
    frame = frame::Create(buffer);
    (newIM)->SetView( frame);
    (frame)->PostDefaultHandler( "message", (frame)->WantHandler( "message"));

/* This is here because the frame_Create call can't set the input focus
 * because the frame didn't have a parent when it called view_WantInputFocus.
 * This is bogus but hard to fix...
 */
    view = (frame)->GetView();
    (view)->WantInputFocus( view);
    return view;
}

/* Most of the work is done here. */

void tags_FindTag (class view  *view, char  *tag, int  RecursiveEdit)

{
    char find[255];

    class text *doc; /* = (struct text *) view->dataobject;*/
    long position=-1, fuzzypos=-1, curpos, length;
    boolean foundTag=FALSE;
    int same;

    static struct SearchPattern *result = NULL;
    class frame *ourFrame;
    class view *ourView;
    class buffer *filebuffer, *curbuf, *TagsBuffer;
    static char c, filename[256], searchstring[256];
    char *fname=filename, *sstring=searchstring;
    int match_bol = 0, match_eol = 0;
    boolean justAnumber;

    /* open or create tags buffer */
    if ((TagsBuffer = tags_OpenTagsBuffer("Tags")) == NULL) {
        message::DisplayString(view,0,"Could not open tags file");
        return;
    }
    doc = (class text *) (TagsBuffer)->GetData();

    search::CompilePattern(tag, &result);
    while (!foundTag) {
	/* keep looking for the EXACT tag name, and only use a fuzzy match IF no exact one was found  -RSK*/
	position = search::MatchPattern(doc, position+1, result);
	if (position<0) {
	    /* we're all done looking, no exact match, so use fuzzy one */
	    foundTag= TRUE;
	    if (fuzzypos>=0)
		position= fuzzypos;
	    else {
		/* no matches found, not even a fuzzy one */
		sprintf(find,"Tag '%s' not found",tag);
		message::DisplayString(view, 0, find);
		return;
	    }
	} else if (firstField(doc,position)) {
	    /* the string we found IS a tag, remember this position for posterity */
	    if ((position==0 || (doc)->GetChar(position-1)=='\n') &&
		((c=(doc)->GetChar(position+search::GetMatchLength()))==' ' || c=='\t')) {
		/* we found that EXACT tag */
		foundTag= TRUE;
	    } else if (fuzzypos<0) {
		/* it's the first fuzzy match we've found; remember it just in case an EXACT one isn't found */
		fuzzypos= position;
	    }
	} /* else what we found is in the middle of something else, just keep searching */
    }

    /* position is pointing to either the exact tag, or the first fuzzy one if no exact match was found  -RSK*/
    position = nextField(doc, position);

    /* copy filename to string */
    c = (doc)->GetChar(position);
    while((c!='\t') && (c!=' ')) {
	*fname++ = c;
	c = (doc)->GetChar(++position);
    }
    *fname = '\0';

    /* copy search string */
    /* skip over '/^' */
    curpos= position= nextField(doc, position);
    while ((c=(doc)->GetChar(position))=='/' || c=='?')
	++position;
    if (position==curpos)
	justAnumber= TRUE; /* might be proven wrong later, but at least it's POSSIBLE */
    else
	justAnumber= FALSE; /* impossible; it's definitely a search string */

    while ((c!='/') && (c!='?')) {
	switch(c) {
	    case '\n':
		if (justAnumber && sstring>searchstring) {
		    /* quit grabbing stuff, we've got our typedef line number */
		    c= '/';
		    continue;
		}
		break;
	    case '^':
		*sstring++ = '\n';
		match_bol = 1;
		break;
	    case '$':
		*sstring++ = '\n';
		match_eol = 1;
		break;
	    case '*':
		*sstring++ = '\\';
		/* fall through */
	    default:
		*sstring++ = c;
		if (justAnumber && !isdigit(c))
		    justAnumber= FALSE;
	}
	c = (doc)->GetChar(++position);
    }
    *sstring = '\0';

    if ((filebuffer = tags_OpenBuffer(filename)) == NULL) {
	sprintf(find,"Bad tag - could not open source file %s",filename);
	message::DisplayString(view,0,find);
	return;
    }

    doc = (class text *) (filebuffer)->GetData();

    if (justAnumber && strlen(searchstring)>=1) {
	/* it's just a line number, the result of using "ctags -t" to find typedefs  -RSK*/
	long linenum;
	if (sscanf(searchstring, "%ld", &linenum)==1) {
	    position= (doc)->GetPosForLine( linenum);
	    length= (doc)->GetPosForLine( linenum+1) - position - 1;
	} else {
	    sprintf(find, "Invalid numerical data '%s' in tag file", searchstring);
	    message::DisplayString(view,0,find);
	    return;
	}
    } else {
	/* search for tag in source file */
	search::CompilePattern(searchstring, &result);
	position = search::MatchPattern(doc,0,result);
	if (position<0) {
	    message::DisplayString(view,0,"Tag not found in source file -- rebuild tag file!");
	    return;
	}
	length = search::GetMatchLength();
	if (match_bol) {
	    position++;
	    length--;
	}
	if (match_eol)
	    length--;
    }

    if (view == NULL) {
	/* being called by ezapp to open a window */
	view = TagsCreateWindow(filebuffer);
	if (view == NULL) {
	    tag = "Window could not be created.";
	    return;
	}
	*tag = '\0';	/* let caller know it succeeded */
    }

    /* set the filebuffer to be current buffer */ 

    ourFrame=FindByView(view);

    curbuf = (ourFrame)->GetBuffer();
    curpos = ((class textview *) view)->GetDotPosition();
    same = (filebuffer == curbuf);

    if (!same)
	(ourFrame)->SetBuffer( filebuffer, TRUE);

    ourView = (ourFrame)->GetView();

    ((class textview *) ourView)->SetDotPosition( position);
    ((class textview *) ourView)->SetDotLength( length);
    ((class textview *) ourView)->FrameDot( position);


    if (RecursiveEdit) {
	char bufferName[100];
	message::DisplayString(ourView, 0, "Recursive editing - ^C exits");
	if (!same && curbuf!=NULL)
	    strncpy(bufferName, (curbuf)->GetName(), sizeof(bufferName));
	else
	    *bufferName= '\0';
	im::KeyboardProcessor();
	if (same) {
	    ((class textview *) view)->SetDotPosition( curpos);
	    ((class textview *) view)->SetDotLength( 0);
	    ((class textview *) view)->FrameDot( curpos);
	} else if (*bufferName && (curbuf= buffer::FindBufferByName(bufferName))!=NULL)
	    (ourFrame)->SetBuffer(curbuf,TRUE);
	message::DisplayString(view,0,"");
    }
    else
	message::DisplayString(ourView, 0, searchstring);
    /*im_ForceUpdate();*/  /*isn't this bogus? */
}

void tags_GotoTagCmd(class view  *view,long  key)
{
    static char name[256];
    int RecursiveEdit= !((view)->GetIM())->ArgProvided() && environ::GetProfileSwitch("TagRecursiveEdit", TRUE);

    /* ask for the tag to search for */
    if (message::AskForString(view,0,"Goto tag: ",NULL,name,255))
	return;
    tags_FindTag(view,name,RecursiveEdit);
}

void tags_FindTagCmd(class view  *view,long  key)
{
    char *name;
    int RecursiveEdit= !((view)->GetIM())->ArgProvided() && environ::GetProfileSwitch("TagRecursiveEdit", TRUE);
    class text *doc=(class text *)view->dataobject;

    name = getFunction(doc,((class textview *)view)->GetDotPosition());
    tags_FindTag(view,name,RecursiveEdit);
}

void tags_OpenCmd(class view  *view,long  key)
{
    char *name;
    int RecursiveEdit;

    if (key != 0) {
	tags_FindTag(view,(char *)key, FALSE);
    }

}

void tags_LoadTagFileCmd(class view  *view,long  key)
{
    class buffer *buffer, *tbuf;
    static char name[MAXPATHLEN+1];

    checkFileName();
    filetype::CanonicalizeFilename(name,TagsFile,sizeof(name));
    if(completion::GetFilename(view, "Load tag file: ", name, name, sizeof(name), FALSE, FALSE) == -1)
        return;
    else
        TagsFile = name;
    
    tbuf = buffer::FindBufferByName(TBUFNAM);
    buffer = tags_OpenBuffer(TagsFile);

    if (buffer == NULL) {  /* couldn't open, save old buffer */
        message::DisplayString(view,0,"Could not open tag file!");
        return;
    }
    else if (buffer == tbuf) { /* same file as old buffer */
            (buffer)->ReadFile(TagsFile);
    }
    else {  /* new file, new buffer */
        if (tbuf) (tbuf)->Destroy();
        (buffer)->SetName(TBUFNAM);
    }

    message::DisplayString(view,0,"Done.");
}




boolean tags::InitializeClass()
    {
    struct ATKregistryEntry  *textviewType = ATK::LoadClass("textview");

    proctable::DefineProc("tags-goto-tag", (proctable_fptr) tags_GotoTagCmd, textviewType, NULL, "Goto a given tag.");
    proctable::DefineProc("tags-find-tag", (proctable_fptr) tags_FindTagCmd, textviewType, NULL, "Goto the tag whose name is at the dot.");
    proctable::DefineProc("tags-open", (proctable_fptr) tags_OpenCmd, textviewType, NULL, "Goto the tag whose name is passed on the command line.");
    proctable::DefineProc("tags-rebuild-tags", (proctable_fptr) tags_RebuildTagsFile, textviewType, NULL, "Regenerate the tag file with ctags");
    proctable::DefineProc("tags-load-tag-file", (proctable_fptr) tags_LoadTagFileCmd, textviewType, NULL, "Load a tag file from the current directory");
    return TRUE;
}
