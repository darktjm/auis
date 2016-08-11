/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/papers.C,v 1.6 1996/06/11 01:28:57 wjh Exp $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/papers.C,v $ */
/* $Author: wjh $ */

#ifndef lint
static char *rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/mit/neos/RCS/papers.C,v 1.6 1996/06/11 01:28:57 wjh Exp $";
#endif /* lint */
/*
 * papers.c
 *
 * This does most of the work for the EOS applications.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *  For full copyright information see:'mit-copyright.h'     *
 *************************************************************/

#include <andrewos.h>
#include <mit-copyright.h>

#include <bind.H>
#include <buffer.H>
#include <completion.H>
#include <cursor.H>
#include <dataobject.H>
#include <environ.H>
#include <environment.H>
#include <eos.h>
#include <fontdesc.H>
#include <frame.H>
#include <hesiod.h>
#include <im.H>
#include <menulist.H>
#include <message.H>
#include <newbuttonview.H>
#include <point.h>
#include <pushbutton.H>
#include <rect.h>
#include <scroll.H>
#include <stdio.h>
#include <style.H>
#include <text.H>
#include <textview.H>
#include <view.H>

/* sys/types.h in AIX PS2 defines "struct label",  causing a type name clash.
  Avoid this by temporarily redefining "label" to be something else. */
#define label gezornenplatz
#include <eosfx.H>
/* eosfx.ih includes sys/types.h */
#undef label

#include <eos.H>   /* eos.ih uses struct label */
#include <papers.H>   /* papers.eh uses struct label */

/* This is the sorting algorithm for the list of papers */
/* June 21 1990 -njw */

/* Version 1: Treesort */
/* The idea is, we build a sorted binary tree with the data
 * which is, I think O(n.log n) and then with the resultant tree,
 * go back through the list, rechaining it. (Using left-first traversal 
 * of the tree.)
 * On the second run thru, where the original list is reordered, 
 * the tree is freed from memory on the fly.
 */

typedef struct node * Tree;

struct node {
    Tree left;
    Tree right;
    Tree parent;
    Paperlist data;
    int count; /* Used for the traversal of the tree */
};

static void add_node(Paperlist  item, Tree  *tree, int  data);
static void sort(Paperlist  *papers, int  data);

/* #define DEBUG_SORT */

#ifdef DEBUG_SORT
#define DEBUG
#endif

#ifdef DEBUG
#include <stdio.h>
#define debug(x) printf x ; fflush(stdin);
#endif /* DEBUG */

#define	SortByAuthor	1
#define	SortByNumber	2
#define	SortByFilename	3
#define	SortByDesc	4
#define	SortByCreate	5
#define	SortNoMore	0
#define	SortPreference	"SortOrder"
#define	SortLimit	5
#define	SortKeysHandouts    0
#define	SortKeysGrading	    1
#define	SortKeysExchange    2
static char *papertypes[3][2] = {
    { "Handouts", "number,author" },
    { "Grading", "number,author,filename" },
    { "Exchange", "author,filename" }
};
static int comparison[3][SortLimit];


/* One bit of data that is global to papers.c and papersaux.c */
class menulist *papers_global_menus;
static class cursor *clockcursor;
static class style *deleted;
static class style *marked;




#ifndef _IBMR2

#endif _IBMR2




ATKdefineRegistry(papers, view, papers::InitializeClass);
#ifndef lint
#endif /* lint */
#ifdef DEBUG_SORT
#endif
#ifdef DEBUG
#endif /* DEBUG */
#ifndef _IBMR2
#endif _IBMR2
static char *UniqueBufferName(char  *name);
static char *RealName(char  *name);
void CancelOperation(class papers  *self, FX  **fxp);
boolean OpenCourse(class papers  *self, FX  **fxp, boolean  continueIfPoss /* If we should continue after a warning from server */);
void MarkAsTaken(class papers  *self, FX  *fxp, struct paperPositions  *index);
#ifdef OLD_PAPERS_DISPLAY
void papers_Display(class papers  *self, class observable  *triggerer, long  rock);
#else /* OLD_PAPERS_DISPLAY */
void papers_Display(class papers  *self, class observable  *triggerer, long  rock);
#endif /*OLD_PAPERS_DISPLAY */
static void Print(class papers  *self, long  rock);
void papers_Keep(class papers  *self, class observable  *triggerer, long  rock);
void papers_Edit(class papers  *self, class observable  *triggerer, long  rock);
void papers_Hide( class papers  *self, class observable  *triggerer, long  rock);
void papers_GradingToggleAndList(class papers  *self, class observable  *triggerer, long  rock);
void papers_ReadList(class papers  *self, long  rock);
void DeletePaper(class papers  *self, struct Paper  *paper);
void papers_RemovePaper(class papers  *self, long  rock);
static void MakeHandout(class papers  *self);
static void MakeExchange(class papers  *self);
void MakePaper(class papers  *self, class observable  *triggerer, long  rock);
int compare(Paperlist  p1 , Paperlist  p2, int  data);
void papers_SetAndList(class papers  *self, long  rock);
void SetSortOrder();
void ToggleMark(class papers  *self, struct paperPositions  *node);
void RemoveMarks(class papers  *self, long  rock);
void SimulateClick(class papers  *self, boolean  IsLeftClick);


static char *UniqueBufferName(char  *name)
/*
 Returns a string name of buffer which is guaranteed to not 
 already exist. If, in the unlikely case, in which a name
 cannot be found, NULL will be returned.
 */
{
    int suffix, namelen;
    static char *uname;

    namelen = strlen(name);

    if (buffer::FindBufferByName(name) == NULL)
	/* The name already is unique! */
	return name;

    /* Allocate room for unique name, plus a bit extra for suffix */
    uname = (char *) malloc(strlen(name) + 5);
    strcpy(uname, name);

    for (suffix = 1; suffix < 1000 ; suffix++) {
	sprintf(uname + namelen, "-%d", suffix);
	if (buffer::FindBufferByName(uname) == NULL)
	    return uname;
    }
    /* There are over 1,000 buffers with a similar name! (Erk!)
      So - give up!
      */
    return NULL;
}


static char *RealName(char  *name)
/*
  Takes a username and tries to find a password entry for it via Hesiod;
  If the resolve fails or the password entry cannot be parsed, then the
  original name is returned, else the name given in passwd is returned, with
	the parameter name following in parentheses;
  e.g. RealName("jsmith") == "jsmith" || "John Smith (jsmith)";
 */
{
    char **namelist, *realname, *tmp;
    static char finalname[128];
    char *index();
    int i;

    if ((namelist = hes_resolve(name, "passwd")) == NULL) {
	strcpy(finalname, name);
	strcat(finalname, "(no hesiod info)");
    } else {
	/* Extract name from password entry */
	realname = *namelist;
	for (i=0; i<4; i++)
	    if ((realname = index(++realname, ':')) == NULL) {
		/* Password entry is screwy - so give up and return original */
		strcpy(finalname, name);
		return finalname;
	    }
	/* Remove rest of password entry */
	if ((tmp = index(++realname,':')) != NULL)
	    *tmp = '\0';
	/* Make sure this is just the name, no unneccassry junk */
	if ((tmp = index(realname, ',')) != NULL)
	    *tmp = '\0';
	/* Just to be nice, add on the original name */
	strcpy(finalname, realname);
	strcat(finalname, " (");
	strcat(finalname, name);
	strcat(finalname, ")");
    }
    return finalname;
}


void CancelOperation(class papers  *self, FX  **fxp)
/* Tidy up after an fx operation */
{
    eosfx::Close(fxp);
    (self->textobj)->SetReadOnly( TRUE);
    (self)->WantInputFocus( self->textv);
    (self)->WantUpdate( self->textv);
    (self)->WantInputFocus( self);
    StopWaitCursor();
    im::ForceUpdate();
}

boolean OpenCourse(class papers  *self, FX  **fxp, boolean  continueIfPoss /* If we should continue after a warning from server */)
{
    char *errormsg;

    errormsg = eosfx::OpenCourse(self->course, fxp);
    /* (fxp == NULL && errormsg) or (fxp is valid for reads&writes) */
    /* or (fxp is valid for reads, invalid for writes && errormsg) */
    if (fxp == NULL || errormsg) {
	/* fxp is not valid for BOTH reads & writes */
	message::DisplayString(self, DIALOG, errormsg);
	if (fxp && continueIfPoss) {
	    /* fxp is valid for reads */
	    message::DisplayString(self, MESSAGE, "Continuing...");
	    return TRUE;
	} else {
	    /* fxp is invalid */
	    message::DisplayString(self, MESSAGE, "Cancelled.");
	    CancelOperation(self, fxp);
	    return FALSE;
	}
    } else
	/* fxp is valid for both reads & writes */
	return TRUE;
}


void MarkAsTaken(class papers  *self, FX  *fxp, struct paperPositions  *index)
/*
  When grading, if a paper is selected for EDIT or KEEP, we want to change
  the status of the paper to TAKEN.
   This routine is careful not to close the fxp.
 */
{
    Paper tmppaper;
    char *errormsg;

    eosfx::PaperCopy(&index->paper->p, &tmppaper);
    tmppaper.type = TAKEN;
    if (errormsg = eosfx::Move (fxp, &index->paper->p, &tmppaper)) {
	message::DisplayString(self, DIALOG, errormsg);
	return;
    }
    index->flags |= eos_PaperTaken;
    message::DisplayString(self, MESSAGE, "Paper has been marked as taken.");
    CancelOperation(self, NULL);
    im::ForceUpdate();
}

#ifdef OLD_PAPERS_DISPLAY
/* Macro to make lines smaller */
#define dispWindow (self->daddy->displaywindow)

void papers_Display(class papers  *self, class observable  *triggerer, long  rock)
/* create frame and window;
  For each paper marked {
      find out which paper they're talking about;
      make a unique buffer name similar to paper.filename;
      create buffer;
      pull through the paper into temporary file;
      read file into buffer;
      set buffer name;
      place buffer into frame;
          goose the fontsize to the one we want for display;
  }
  finish up;
*/
{
    FX *fxp;
    char filename[128];
    char bufname[128];
    char *errormsg;
    int i;
    boolean GotOne = FALSE, many;
    struct paperPositions *index;
    class textview *tv;
    long dsize, usize;
    enum style_FontSize basis;

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    if (!self->daddy) {
	message::DisplayString(self, DIALOG, "I've been orphaned! Cannot tell daddy what you want to display!");
	CancelOperation(self, NULL);
	return;
    }

    index = self->Positions;
    if (!OpenCourse(self, &fxp, TRUE)) {
	CancelOperation(self, &fxp);
	return;
    }

    many = environ::GetProfileSwitch ("DisplayMany", FALSE);

    /* Create the new window - a frame/buffer view */
    if (many || dispWindow == NULL || dispWindow->im == NULL || dispWindow->data == NULL || (!(dispWindow->data)->Visible())) {
	if (dispWindow == NULL) dispWindow = (struct readWindow *)malloc(sizeof (struct readWindow));
	dispWindow->im = im::Create(NULL);
	dispWindow->fr = new class frame;
	if (!dispWindow->im || !dispWindow->fr) {
	    message::DisplayString(self, DIALOG, "Could not create a new window!");
	    CancelOperation(self, &fxp);
	    return;
	}
	/* Make sure there is a message handler for the new frame. -njw 9/6/90 */
	(dispWindow->fr)->PostDefaultHandler( "message", (dispWindow->fr)->WantHandler( "message"));
	(dispWindow->im)->SetView( dispWindow->fr);
	/* We also want to give the frame something to chew on, in case
	    we cannot find it a file to look at -njw */
	dispWindow->data = buffer::FindBufferByName("scratch");
	if (!dispWindow->data)
	    dispWindow->data = buffer::Create("scratch","scratch", NULL, NULL);
	(dispWindow->data)->SetName( "Display");
	(dispWindow->data)->SetScratch( TRUE);
	(dispWindow->data)->SetCkpClock( 0);
	(dispWindow->fr)->SetBuffer( dispWindow->data, TRUE);
    } else (dispWindow->im)->ExposeWindow();
    message::DisplayString(dispWindow->fr, MESSAGE, "Please wait: retrieving the file(s)...");
    im::ForceUpdate();

    /* For each marked paper... */
    for (i = self->markcount; i>0; i--) {
	while (!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted) {
	    message::DisplayString(self, MESSAGE, "That paper has been deleted.");
	} else {
	    strcpy(bufname, UniqueBufferName(index->paper->p.filename));
	    if (bufname == NULL) {
		message::DisplayString(self, DIALOG, "What a lot of buffers you've got! Cannot create a new one...");
		CancelOperation(self, &fxp);
		return;
	    }
	    /* Receive the paper into a temporary file */
	    strcpy(filename, "/tmp/");
	    strcat(filename, index->paper->p.filename);
	    strcpy(filename, eosfx::LocalUnique(filename));
	    if (errormsg = eosfx::RetrieveFile(fxp, &index->paper->p, filename)) {
		message::DisplayString(dispWindow->fr, MESSAGE, "Error encountered");
		message::DisplayString(dispWindow->fr, DIALOG, errormsg);
		if (!GotOne && many) (dispWindow->im)->Destroy();
		CancelOperation(self, &fxp);
		return;
	    } else GotOne = TRUE;
	    dispWindow->data = buffer::GetBufferOnFile(filename, buffer_ForceNew);
	    unlink(filename);
	    (dispWindow->data)->SetFilename( NULL);
	    (dispWindow->data)->SetName( bufname);
	    (dispWindow->fr)->SetBuffer( dispWindow->data, TRUE);

	    /* tickle the font size if it's different from the default */
	    tv = (class textview *) (dispWindow->fr)->GetView();
	    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
		class style *ds = (tv)->GetDefaultStyle();
		usize = environ::GetProfileInt("DisplayFontsize", 20);
		(ds)->GetFontSize( &basis, &dsize);
		if (basis == style_ConstantFontSize && dsize != usize)
		    (ds)->SetFontSize( style_ConstantFontSize, usize);
	    }

	    (dispWindow->fr)->SetCommandEnable( TRUE);
	}
	ToggleMark(self, index);
	index = index->next;
    }
    message::DisplayString(dispWindow->fr, MESSAGE, "");
    CancelOperation(self, &fxp);
    return;
}

#else /* OLD_PAPERS_DISPLAY */

void papers_Display(class papers  *self, class observable  *triggerer, long  rock)
/* create frame and window;
  For each paper marked {
      find out which paper they're talking about;
      make a unique buffer name similar to paper.filename;
      create buffer;
      pull through the paper into temporary file;
      read file into buffer;
      set buffer name;
      place buffer into frame;
          goose the fontsize to the one we want for display;
  }
  finish up;
*/
{
    FX *fxp;
    char filename[128];
    char bufname[128];
    char *errormsg;
    int i;
    struct paperPositions *index;
    class textview *tv;
    long dsize, usize;
    enum style_FontSize basis;

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    if (!self->daddy) {
	message::DisplayString(self, DIALOG, "I've been orphaned! Cannot tell daddy what you want to display!");
	CancelOperation(self, NULL);
	return;
    }

    if (!OpenCourse(self, &fxp, TRUE)) {
	CancelOperation(self, &fxp);
	return;
    }

    message::DisplayString(self, MESSAGE, "Please wait: retrieving the file(s)...");

    im::ForceUpdate();

    index = self->Positions;

   /* For each marked paper... */
    for (i = self->markcount; i>0; i--) {
	while (!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted) {
	    message::DisplayString(self, MESSAGE, "That paper has been deleted.");
	} else {
	    strcpy(bufname, UniqueBufferName(index->paper->p.filename));
	    if (bufname == NULL) {
		message::DisplayString(self, DIALOG, "What a lot of buffers you've got! Cannot create a new one...");
		CancelOperation(self, &fxp);
		return;
	    }
	    /* Receive the paper into a temporary file */
	    strcpy(filename, "/tmp/");
	    strcat(filename, index->paper->p.filename);
	    strcpy(filename, eosfx::LocalUnique(filename));
	    if (errormsg = eosfx::RetrieveFile(fxp, &index->paper->p, filename)) {
		message::DisplayString(self, MESSAGE, "Error encountered");
		message::DisplayString(self, DIALOG, errormsg);
		CancelOperation(self, &fxp);
		return;
	    }
	    (self->daddy)->SetBuffer( filename, buffer_ForceNew);
	    unlink(filename);
	    (self->daddy)->SetFontDisplay();
#if 0
	    /* tickle the font size if it's different from the default */
	    tv = (class textview *) (self->daddy->frame)->GetView();
	    if (strcmp ((tv)->GetTypeName(), "textview") == 0) {
		class style *ds = (tv)->GetDefaultStyle();
		usize = environ::GetProfileInt("DisplayFontsize", 20);
		(ds)->GetFontSize( &basis, &dsize);
		if (basis == style_ConstantFontSize && dsize != usize)
		    (ds)->SetFontSize( style_ConstantFontSize, usize);
	    }
#endif
	}
	ToggleMark(self, index);
	index = index->next;
    }
    message::DisplayString(self, MESSAGE, "Done.");
    CancelOperation(self, &fxp);
    return;
}
#endif /*OLD_PAPERS_DISPLAY */

static void Print(class papers  *self, long  rock)
{
    FX *fxp;
    FILE *printer;
    char string[128];
    char printcommand[256], command[256];
    char *errormsg;
    int i;
    struct paperPositions *index;
    char *AndrewDir();

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    index = self->Positions;

    /* Create the print command to activate the pipe...
      Use ezprint, specifying file to come from the stdin
      */
    strcpy(printcommand, AndrewDir("/bin/ezprint"));
    strcat(printcommand, " -s -o ");

    StartWaitCursor();
    if (!OpenCourse(self, &fxp, TRUE)) return;

    /* For each marked paper... */
    for (i = self->markcount; i>0; i--) {
	while (!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted) {
	    message::DisplayString(self, MESSAGE, "That paper has been deleted.");
	} else {
	    /* Start a pipe to the print command, ready for a file to be
		given to it.
		*/
	    strcpy(command, printcommand);
	    strcat(command, index->paper->p.filename);
	    if ((printer = popen(command, "w")) == NULL) {
		message::DisplayString(self, DIALOG, "Could not run ezprint!");
		CancelOperation(self, &fxp);
		return;
	    }

	    /* Retrieve the file, placing it into the pipe which leads into
	     the printer.
	     */
	    if (errormsg = eosfx::Retrieve(fxp, &index->paper->p, printer)) {
		CancelOperation(self, &fxp);
		message::DisplayString(self, DIALOG, errormsg);
		pclose(printer);
		return;
	    }

	    pclose(printer);
	    strcpy(string, index->paper->p.filename);	
	    strcat(string, " has been sent to printer");
	    message::DisplayString(self, MESSAGE, string);
	}
	ToggleMark(self, index);
	im::ForceUpdate();
	index = index->next;
    }

    CancelOperation(self, &fxp);
}

void papers_Keep(class papers  *self, class observable  *triggerer, long  rock)
{
    FX *fxp;
    char string[128], filename[128];
    char *errormsg;
    int i;
    struct paperPositions *index;

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    index = self->Positions;

    if (self->thiswindow == papersGrading) {
	/* we need to inform the server the paper has been taken, i.e.
	 * we need write access */
	if (!OpenCourse(self, &fxp, FALSE))
	    return;
    } else
	if (!OpenCourse(self, &fxp, TRUE))
	    return;

    /* For each marked paper... */
    for (i = self->markcount; i>0; i--) {
	while (!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted) {
	    message::DisplayString(self, MESSAGE, "That paper has been deleted.");
	} else {
	    /* Retrieve the paper into a locally unique named file */ 
	    strcpy(filename, eosfx::LocalUnique(index->paper->p.filename));
	    if (errormsg = eosfx::RetrieveFile(fxp, &index->paper->p, filename)) {
		CancelOperation(self, &fxp);
		message::DisplayString(self, DIALOG, errormsg);
		return;
	    }

	    if (self->thiswindow == papersGrading) 
		MarkAsTaken(self, fxp, index);

	    strcpy(string, "File has been received as ");
	    strcat(string, filename);
	    message::DisplayString(self, MESSAGE, string);
	}
	im::ForceUpdate();
	ToggleMark(self, index);
	index = index->next;
    }

    CancelOperation(self, &fxp);
    return;
}

void papers_Edit(class papers  *self, class observable  *triggerer, long  rock)
/*
  Retrieve paper into local file and tell the parent eos to make a buffer for
  the new file 
 */
{
    FX *fxp;
    char string[128];
    char filename[128];
    char *errormsg;
    int i;
    struct paperPositions *index;
    int	default_edit_keep = TRUE;   /* Students will want to keep edits by default */

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    if (!self->daddy) {
	message::DisplayString(self, DIALOG, "I've been orphaned! Cannot tell daddy what you want to edit!");
	CancelOperation(self, NULL);
	return;
    }

    index = self->Positions;

    if (self->thiswindow == papersGrading) {
	/* we need to inform the server the paper has been taken, i.e.
	 * we need write access */
	if (!OpenCourse(self, &fxp, FALSE))
	    return;
    } else
	if (!OpenCourse(self, &fxp, TRUE))
	    return;

    /* For each marked paper... */
    for (i = self->markcount; i>0; i--) {
	while (!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted) {
	    message::DisplayString(self, MESSAGE, "That paper has been deleted.");
	} else {
	    /* Retrieve paper into locally unique named file */
	    /* Graders add uname to file name */
	    if (self->thiswindow == papersGrading) {
		strcpy(filename, index->paper->p.author);
		strcat(filename, "_");
		strcat(filename, index->paper->p.filename);
		strcpy(filename, eosfx::LocalUnique(filename));
	    } else {
		strcpy(filename, eosfx::LocalUnique(index->paper->p.filename));
	    }
	    if (errormsg = eosfx::RetrieveFile(fxp, &index->paper->p, filename)) {
		CancelOperation(self, &fxp);
		message::DisplayString(self, DIALOG, errormsg);
		return;
	    }

	    if (self->thiswindow == papersGrading) {
		/* graders will not want to keep by default */
		default_edit_keep = FALSE;
		MarkAsTaken(self, fxp, index);
	    }

	    /* If EditImpliesKeep is true, we keep the file. */
	    if (environ::GetProfileSwitch("EditImpliesKeep", default_edit_keep)) {
		strcpy(string, "Received file as ");
		strcat(string, filename);
		message::DisplayString(self, MESSAGE, string);
		/* Tell parent eos about the new file */
		(self->daddy)->SetBuffer( filename, buffer_ForceNew);
	    } else {
		/* Tell parent eos about the new file */
		(self->daddy)->SetBuffer( filename, buffer_ForceNew);
		unlink (filename); /* Must unlink AFTER read */
	    }
	    /* If we're grading, hang the struct paper * off the buffer. */
	    if (self->thiswindow == papersGrading) {
		Paper *ppaper = (Paper *)malloc(sizeof(Paper));
		eosfx::PaperCopyContents (&(index->paper->p), ppaper);
		(((self->daddy->frame)->GetBuffer())->GetData())->Put( self->daddy->paperatom, self->daddy->paperatom, (long)ppaper);
	    }
	}
	im::ForceUpdate();
	ToggleMark(self, index);
	index = index->next;
    }

    CancelOperation(self, &fxp);
    return;

}

void papers_Hide( class papers  *self, class observable  *triggerer, long  rock)
{
    if (!self) return;
    self->IDoNotExist = TRUE;
    /* "If I close my eyes, then you can't see me!" */
    ((self)->GetIM())->VanishWindow();
}

void papers_GradingToggleAndList(class papers  *self, class observable  *triggerer, long  rock)
/* This is normally a button-triggered procedure, but it can
  * also be used to perform the initial listing of the papers
  * i.e. p = papers_New();
  *      set p->toggle to be papers_OLD or papers_NEW
    *      papers_GradingToggleAndList(p, NULL, 0);
  */
{
    class pushbutton *button =  (class pushbutton *) (self->toggle)->GetDataObject();

    if (self->toggled == papers_NEW) {
	(self)->SetTitle( "Old Papers");
	(button)->SetText( "NEW DOCS");
	self->toggled = papers_OLD;
	(self)->ListGrade();
    } else if (self->toggled == papers_OLD) {
	(self)->SetTitle( "Papers to Grade");
	(button)->SetText( "OLD DOCS");
	self->toggled = papers_NEW;
	(self)->ListGrade();
    }
    return;
}

/* --------- Menu functions -------- */

static char *confirms[] = { "Confirm", "Cancel", NULL };

void papers_ReadList(class papers  *self, long  rock)
/* Called from menu option "Update List" */
{
    switch (self->thiswindow) {
	case papersGrading:
	    (self)->ListGrade();
	    break;
	case papersHandouts:
	    (self)->ListHandouts();
	    break;
	case papersExchange:
	    (self)->ListExchange();
	    break;
	default:
	    break;
    }
}

void DeletePaper(class papers  *self, struct Paper  *paper)
/* Used once in the entire code. Hmmph */
{
    FX *fxp;
    char *errormsg;

    if (!OpenCourse(self, &fxp, FALSE))
	return;
    if (errormsg = eosfx::Delete(fxp, paper)) {
 	CancelOperation(self, &fxp);
	message::DisplayString(self, DIALOG, errormsg);
	return;
    }

    CancelOperation(self, &fxp);
}

void papers_RemovePaper(class papers  *self, long  rock)
/* Invoked from the menu option. We want to delete every selected paper */
{
    FX *fxp;
    char *errormsg;
    int answer, i;
    struct paperPositions *index;

    if (self->markcount == 0) {
	message::DisplayString(self, MESSAGE, "There are no papers selected!");
	return;
    }

    message::MultipleChoiceQuestion(self, DIALOG, "Please confirm you wish to do this", 1, &answer, confirms, NULL);
    if (answer) {
	message::DisplayString(self, MESSAGE, "Cancelled");
	CancelOperation(self, NULL);
	return;
    }

    index = self->Positions;
    if (!OpenCourse(self, &fxp, FALSE)) return;

    for (i = self->markcount; i>0; i--) {
	while(!((index->flags) & eos_PaperMarked)) index = index->next;

	if (index->flags & eos_PaperDeleted)
	    message::DisplayString(self, MESSAGE, "That paper is already deleted");
	else {
	    if (errormsg = eosfx::Delete(fxp, &index->paper->p)) {
		CancelOperation(self, &fxp);
		message::DisplayString(self, DIALOG, errormsg);
		return;
	    }

	    ToggleMark(self, index);
	    index->flags |= eos_PaperDeleted;
	    index->environment = new class environment;
	    index->environment = (self->textobj->rootEnvironment)->InsertStyle( index->textpos, deleted, TRUE);
	    (index->environment)->SetLength( index->textlength);
	    (self->textobj)->RegionModified( index->textpos, index->textlength);
	}
	index = index->next;
    }

    if (self->markcount > 1)
	message::DisplayString(self, MESSAGE, "Selected papers have been deleted");
    else
	message::DisplayString(self, MESSAGE, "The paper has been deleted");

    CancelOperation(self, &fxp);
    return;
}

static char *handoutchoices[] =
{
    "The contents of the current edit window",
    "A file",
    "Cancel",
    NULL
};


void papers::GradingListType(enum papers_Toggles  type)
{
    /* This routine is for external use - it sets the object to
	be a listing of the required type.
	*/
    if (type == papers_NEW)
	this->toggled = papers_OLD;
    else
	this->toggled = papers_NEW;
    papers_GradingToggleAndList(this, NULL, 0L);
}

static void MakeHandout(class papers  *self)
{
    Paper paper;
    char answer[128], name[33], strnum[6], *errormsg;
    long result;

    StopWaitCursor();
    if (self->daddy == NULL) {
	message::DisplayString(self, DIALOG, "Program bug - I am an orphan!");
	CancelOperation(self, NULL);
	return;
    }

    message::MultipleChoiceQuestion(self, DIALOG, "What do you want to use as the handout?", 0, &result, handoutchoices, NULL);
    if (result == 2) {
	message::DisplayString(self, MESSAGE, "Cancelled.");
	CancelOperation(self, NULL);
	return;
    }

    eosfx::PaperClear(&paper);
    paper.type = HANDOUT;

    if (result == -1) {
	message::DisplayString(self, DIALOG, "Failed to get a straight answer!");
	CancelOperation(self, NULL);
	return;
    }

    if (result == 0) {
	strcpy(answer, mktemp("/tmp/.eosXXXXXX"));
	((self->daddy->frame)->GetBuffer())->WriteToFile( answer, buffer_ReliableWrite);
	paper.filename = ((self->daddy->frame)->GetBuffer())->GetName();
    } else {
	/* Specify filename */
	if (completion::GetFilename(self, "What is the name of the file? ", NULL, answer, sizeof(answer), FALSE, TRUE) == -1) {
	    CancelOperation(self, NULL);
	    return;
	}
	paper.filename = eosfx::PathTail(answer);
	message::DisplayString(self, MESSAGE, "");
	im::ForceUpdate();
    }

    if (message::AskForString(self, DIALOG, "Please give a description of the handout", "", name, 32) == -1) {
	message::DisplayString(self, MESSAGE, "Cancelled.");
	CancelOperation(self, NULL);
	return;
    } else
	paper.desc = name;

    if (message::AskForString(self, DIALOG, "For which class meeting number is the handout?", "", strnum, 5) == -1) {
	message::DisplayString(self, MESSAGE, "Cancelled.");
	CancelOperation(self, NULL);
	return;
    } else
	paper.assignment = atoi(strnum);

    StartWaitCursor();
    paper.type = HANDOUT;
    message::DisplayString(self, MESSAGE, "Please wait: Sending handout...");
    im::ForceUpdate();
    errormsg = eosfx::SendFile(self->course, answer, &paper, FALSE);
    if (result == 0) unlink (answer);	/* del temp */
    if (errormsg != NULL) {
	message::DisplayString(self, MESSAGE, "Cancelled");
	message::DisplayString(self, DIALOG, errormsg);
    } else message::DisplayString(self, MESSAGE, "Done.");
    CancelOperation(self, NULL);
    return;
}

static void MakeExchange(class papers  *self)
{
    Paper paper;
    char answer[128], name[128], *errormsg;
    long result;

    StopWaitCursor();
    message::MultipleChoiceQuestion(self, DIALOG, "What do you want to use as the submission?", 0, &result, handoutchoices, NULL);

    eosfx::PaperClear(&paper);

    switch (result) {
	case -1:
	    message::DisplayString(self, DIALOG, "failed to get a straight answer...");
	    break;
	case 0:
	    /* Take current buffer */
	    if (self->daddy == NULL) {
		message::DisplayString(self, DIALOG, "Program bug - Do not know what the buffer is!");
		CancelOperation(self, NULL);
		return;
	    }

	    if (message::AskForString(self, DIALOG, "Please give a name for the paper", "", name, 32) == -1) {
		message::DisplayString(self, MESSAGE, "Cancelled.");
		CancelOperation(self, NULL);
		return;
	    }

	    message::DisplayString(self, MESSAGE, "Please wait: Sending paper...");
	    StartWaitCursor();
	    im::ForceUpdate();

	    strcpy(answer, mktemp("/tmp/.eosXXXXXX"));
	    ((self->daddy->frame)->GetBuffer())->WriteToFile( answer, buffer_ReliableWrite);
	    paper.type = EXCHANGE;
	    paper.filename = eosfx::PathTail(name);
	    errormsg = eosfx::SendFile(self->course, answer, &paper, FALSE);
	    unlink (answer);
	    if (errormsg != NULL) {
		message::DisplayString(self, MESSAGE, "Cancelled");
		message::DisplayString(self, DIALOG, errormsg);
	    } else message::DisplayString(self, MESSAGE, "Done.");
	    break;
	case 1:
	    /* Specify filename */
	    if (completion::GetFilename(self, "What is the name of the file? ", NULL, answer, sizeof(answer), FALSE, TRUE) == -1) return;
	    /* Since completion MUST match, we have the name of the file - can go straight ahead and throw it in! */
	    message::DisplayString(self, MESSAGE, "Please wait: Sending exchange paper...");
	    StartWaitCursor();
	    im::ForceUpdate();
	    paper.type = EXCHANGE;
	    errormsg = eosfx::SendFile(self->course, answer, &paper, FALSE);
	    if (errormsg != NULL) {
		message::DisplayString(self, MESSAGE, "Cancelled");
		message::DisplayString(self, DIALOG, errormsg);
	    } else message::DisplayString(self, MESSAGE, "Done.");
	    break;
	case 2:
	    /* Cancel */
	    break;
    }

    CancelOperation(self, NULL);
    return;
}


void MakePaper(class papers  *self, class observable  *triggerer, long  rock)
/* Button "SUBMIT" - could be either a handout or exchange paper */
{
    switch (self->thiswindow) {
	case papersHandouts:
	    MakeHandout(self);
	    break;
	case papersExchange:
	    MakeExchange(self);
	    break;
	default:
	    break;
    }

    return;
}


/* ----- Misc functions & exported methods ------ */

int
compare(Paperlist  p1 , Paperlist  p2, int  data)
{
    int i,j;
#ifdef DEBUG_SORT
debug(("Begin comparison:\n"));
#endif

    for (i = 0; comparison[data][i] != SortNoMore; i++) {
	switch (comparison[data][i]) {
	    case SortByNumber:
#ifdef DEBUG_SORT
debug(("compare using assignment number\n"));
#endif
		if (p2->p.assignment < p1->p.assignment)
		    return -1;
		else if (p2->p.assignment > p1->p.assignment)
		    return 1;
		break;
	    case SortByAuthor:
#ifdef DEBUG_SORT
debug(("compare using author\n"));
#endif
		j = strcmp(p2->p.author, p1->p.author);
		if (j != 0)
		    return j;
		break;
	    case SortByFilename:
#ifdef DEBUG_SORT
debug(("compare using filename\n"));
#endif
		j = strcmp(p2->p.filename, p1->p.filename);
		if (j != 0)
		    return j;
		break;
	    case SortByDesc: /* Why would anyone want to sort on this ? */
#ifdef DEBUG_SORT
debug(("compare using description\n"));
#endif
		j = strcmp(p2->p.desc, p1->p.desc);
		if (j != 0)
		    return j;
		break;
	    case SortByCreate: /* Creation time */
#ifdef DEBUG_SORT
debug(("compare using creation time\n"));
#endif
		if (p2->p.created.tv_sec < p1->p.created.tv_sec)
		    return -1;
		else if (p2->p.created.tv_sec > p1->p.created.tv_sec)
		    return 1;
		break;
		      
	}
    }
    return 0; /* If we haven't discriminated it yet, assume it is identical */
}


int papers::ListHandouts()
/* Opens the course. Gets the list of papers from FX
  * Steps thru list, placing text into self->textobj describing the
  * paper. Also sets global array paperstextpos[0..numofpapers]
  * where paperstextpos[i] = the text position of the start of the 
  * i'th paper description.
		    * numofpapers is set to be the total counted. This value is
		    * also returned.
		    * The textobj is set to be writable during list and then after 
		    * the procedure has finished, it sets text to be read-only
		    */
{
    int i;
    Paper criterion;
    Paperlist node;
    FX *fxp;
    char *errormsg;
    char string[256];

    message::DisplayString(this, MESSAGE, "Please wait: Reading the list...");
    (this->textobj)->SetReadOnly( FALSE);
    StartWaitCursor();
    im::ForceUpdate();

    (this->textobj)->ClearCompletely();

    /* Make sure the list is 'clean' before we get a new list */
    eosfx::DestroyPositions(&this->Positions);
    eosfx::ListDestroy(&this->list);
    this->markcount = 0;

    if (!OpenCourse(this, &fxp, TRUE)) {
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	return 0;
    }

    /* Set the criterion - totally clean apart from type.
      * i.e. We want anything at all - so long as it is a paper.
      */
    eosfx::PaperClear(&criterion);
    criterion.type = HANDOUT;
    criterion.assignment = this->assignment;
#if 0
    criterion.author = this->student;
#endif

    if (errormsg = eosfx::List(fxp, &criterion, &this->list)) {
	textput(errormsg);
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	this->list = NULL;
	CancelOperation(this, &fxp);
	return 0;
    }

    eosfx::Close(&fxp);

    /* Step through the list of papers, displaying each one. */
    i = 0;
    sort(&(this->list->Paperlist_res_u.list), SortKeysHandouts);
    for (node = this->list->Paperlist_res_u.list; node !=NULL; node = node->next) {
	i++;
	sprintf(string, "Class Meeting %d: '%s'\nDescription: %s\n\tAuthor: %s\n\n", node->p.assignment, node->p.filename,node->p.desc, RealName(node->p.author));
	eosfx::AddPaperText(&this->Positions, node, (this->textobj)->GetLength(), strlen(string));
	textput(string);
    }

    if (!i) {
	char tmpstring[50];
	if (this->assignment) sprintf (tmpstring, "class meeting number %d", this->assignment);
	else sprintf (tmpstring, "any class meeting number");
	sprintf (string, "No handouts of %s available at present.\n", tmpstring);
	textput(string);
	this->list = NULL;
	message::DisplayString(this, MESSAGE, "");
    } else {
	if (i != 1) sprintf(string, "There are %d handouts", i);
	else sprintf(string, "There is one handout available.");
	message::DisplayString(this, MESSAGE, string);
    }

    message::DisplayString(this, MESSAGE, "");
    CancelOperation(this, NULL);
    return i;
}

int papers::ListExchange()
/* Opens the course. Gets the list of papers from FX
  * Steps thru list, placing text into self->textobj describing the
  * paper. Also sets self->list to show the list of papers and attributes
  * The textobj is set to be writable during list and then after 
  * the procedure has finished, it sets text to be read-only
  * The return value is the number of papers counted
  */
{
    int i;
    Paper criterion;
    Paperlist node;
    FX *fxp;
    char *errormsg;
    char string[256];

    (this->textobj)->SetReadOnly( FALSE);
    message::DisplayString(this, MESSAGE, "Please wait: reading list");
    StartWaitCursor();
    im::ForceUpdate();

    (this->textobj)->ClearCompletely();

    /* Make sure the list is 'clean' before we get a new list */
    eosfx::DestroyPositions(&this->Positions);
    eosfx::ListDestroy(&this->list);
    this->markcount = 0;

    if (!OpenCourse(this, &fxp, TRUE)) {
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	return 0;
    }


    /* Set the criterion - totally clean apart from type.
      * i.e. We want anything at all - so long as it is a paper.
      */
    eosfx::PaperClear(&criterion);
    criterion.type = EXCHANGE;
    criterion.author = this->student;

    if(errormsg = eosfx::List(fxp, &criterion, &this->list)) {
	textput(errormsg);
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	this->list = NULL;
	CancelOperation(this, &fxp);
	return 0;
    }

    eosfx::Close(&fxp);

    /* Step through the list of papers, displaying each one. */
    i = 0;
    sort(&(this->list->Paperlist_res_u.list), SortKeysExchange);
    for (node = this->list->Paperlist_res_u.list; node !=NULL; node = node->next) {
	i++;
	sprintf(string, "'%s' by %s\n", node->p.filename, RealName(node->p.author));
	eosfx::AddPaperText(&this->Positions, node, (this->textobj)->GetLength(), strlen(string));
	textput(string);
    }

    if (!i) {
	sprintf (string, "No exchanges by %s available at present.\n", (this->student)? this->student : "anyone");
	textput(string);
	this->list = NULL;
	message::DisplayString(this, MESSAGE, "");
    } else {
	if (i != 1) sprintf(string, "There are %d papers", i);
	else sprintf(string, "There is one paper available.");
	message::DisplayString(this, MESSAGE, string);
    }

    CancelOperation(this, NULL);
    return i;
}

int papers::ListGrade()
/* Opens the course. Gets the list of papers from FX
  * Steps thru list, placing text into self->textobj describing the
  * paper. Also sets global array paperstextpos[0..numofpapers]
  * where paperstextpos[i] = the text position of the start of the 
  * i'th paper description.
  * numofpapers is set to be the total counted. This value is
  * also returned.
  * The textobj is set to be writable during list and then after 
  * the procedure has finished, it sets text to be read-only
  */
{
    int i;
    Paper criterion;
    Paperlist node;
    FX *fxp;
    char *errormsg;
    char string[256], typestring[128];

    (this->textobj)->SetReadOnly( FALSE);
    message::DisplayString(this, MESSAGE, "Please wait: reading list");
    StartWaitCursor();
    (this->textobj)->ClearCompletely();
    im::ForceUpdate();

    /* Make sure the list is 'clean' before we get a new list */
    eosfx::DestroyPositions(&this->Positions);
    eosfx::ListDestroy(&this->list);
    this->markcount = 0;

    if (!OpenCourse(this, &fxp, TRUE)) {
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	return 0;
    }
    /* Set the criterion - 
     *  The defaults have been set to 0 in papers_InitializeClass
         * or to default values by papers_SetDefault.
         * default values of 0 are wild cards.
     */
    eosfx::PaperClear(&criterion);

    criterion.type = TYPE_WILDCARD;
    criterion.assignment = this->assignment;
    criterion.author = this->student;

    if (errormsg = eosfx::List(fxp, &criterion, &this->list)) {
	textput(string);
	message::DisplayString(this, MESSAGE, "Operation aborted due to errors");
	this->list = NULL;
	CancelOperation(this, &fxp);
	return 0;
    }

    eosfx::Close(&fxp);

    /* Step through the list of papers, displaying each one. */
    i = 0;
    sort(&(this->list->Paperlist_res_u.list), SortKeysGrading);
    for (node = this->list->Paperlist_res_u.list; node !=NULL; node = node->next) {
	/* Are we looking at old papers or new papers? */
	if (this->toggled == papers_NEW) {
	    if (node->p.type == TURNEDIN) {
		i++;
		sprintf(string, "%s by %s (Assignment %d)\n\n", node->p.filename, RealName(node->p.author), node->p.assignment);
		eosfx::AddPaperText(&this->Positions, node, (this->textobj)->GetLength(), strlen(string));
		textput(string);
	    }
	} else {
	    /* Old papers are of several different types. So tell user
	     which paper is which type...
	     */
	    switch (node->p.type) {
		case TAKEN:
		    sprintf(typestring, "Taken by %s", RealName(node->p.owner));
		    break;
		case GRADED:
		    sprintf(typestring, "Graded by %s", RealName(node->p.owner));
		    break;
		case PICKEDUP:
		    strcpy(typestring, "Picked Up");
		    break;
		default:
		    typestring[0] = '\0';
	    }
	    if (typestring[0] != '\0') {
		i++;
		sprintf(string, "%s:\n%s by %s (Assignment %d)\n\n", typestring, node->p.filename, RealName(node->p.author), node->p.assignment);
		eosfx::AddPaperText(&this->Positions, node, (this->textobj)->GetLength(), strlen(string));
		textput(string);
	    }
	}
    }

    if (!i) {
	if (this->assignment) sprintf (typestring, "assignment %d", this->assignment);
	else sprintf (typestring, "any assignment");
	sprintf (string, "No papers of %s by %s available at present.\n", typestring, (this->student)? this->student : "anyone");
	textput(string);
	message::DisplayString(this, MESSAGE, "");
	this->list = NULL;
    } else {
	if (i != 1) sprintf(string, "There are %d papers to grade", i);
	else sprintf(string, "There is one paper to grade.");
	message::DisplayString(this, MESSAGE, string);
    }

    CancelOperation(this, NULL);
    return i;
}

void papers::SetDefault()
{
    char uname[33], strnum[6];

    switch (this->thiswindow) {
	case papersGrading:
	    if (this->assignment) sprintf(uname, "%d", this->assignment);
	    else sprintf (uname, "*");
	    if (message::AskForString(this, MESSAGE, "List papers matching assignment number (* for all): ", uname, strnum, 5) == -1) return;
	    if (strnum[0] == '*') this->assignment = 0;
	    else this->assignment = atoi(strnum);
	    if (message::AskForString(this, MESSAGE, "List papers matching student user name (* for all): ", (this->student)? this->student : "*", uname, 32) == -1) return;
	    if (this->student != NULL) free (this->student);
	    if (uname[0] == '*') this->student = NULL;
	    else {
		this->student = malloc (strlen (uname) + 1);
		strcpy (this->student, uname);
	    }
	    break;
	case papersHandouts:
	    if (this->assignment) sprintf(uname, "%d", this->assignment);
	    else sprintf (uname, "*");
	    if (message::AskForString(this, MESSAGE, "List handouts matching class meeting number (* for all): ", uname, strnum, 5) == -1) return;
	    if (strnum[0] == '*') this->assignment = 0;
	    else this->assignment = atoi(strnum);
	    /* We don't currently care about author of handouts */
#if 0
	    if (message::AskForString(this, MESSAGE, "List papers matching author user name (* for all): ", (this->student)? this->student : "*", uname, 32) == -1) return;
	    if (this->student != NULL) free (this->student);
	    if (uname[0] == '*') this->student = NULL;
	    else {
		this->student = malloc (strlen (uname) + 1);
		strcpy (this->student, uname);
	    }
#endif
	    break;
	case papersExchange:
	    if (message::AskForString(this, MESSAGE, "List papers matching author user name (* for all): ", (this->student)? this->student : "*", uname, 32) == -1) return;
	    if (this->student != NULL) free (this->student);
	    if (uname[0] == '*') this->student = NULL;
	    else {
		this->student = malloc (strlen (uname) + 1);
		strcpy (this->student, uname);
	    }
	    break;
	default:
	    break;
    }
}

void papers_SetAndList(class papers  *self, long  rock)
{
    (self)->SetDefault();
    papers_ReadList(self, NULL);
}

static struct bind_Description paperBindings[]={
    {"papers-remove-paper",   NULL, 0, "Remove selected paper(s)~30",	   0,MENUS_instructor, papers_RemovePaper,  NULL},
    {"papers-set-default", NULL, 0, "List By ...~09", 0, MENUS_general, papers_SetAndList, "Set criteria for and list papers."},
    {"papers-list", NULL, 0, "Update list~10",
    0, MENUS_general, papers_ReadList, NULL},
    {"papers-print", NULL, 0, "Print selected paper(s)~15", 0, MENUS_general, Print, NULL},
    NULL
};


boolean papers::InitializeClass()
{
    papers_global_menus = new class menulist;
    bind::BindList(paperBindings, NULL, papers_global_menus, &papers_ATKregistry_ );

    deleted = new class style;
    marked  = new class style;
    (deleted)->SetFontSize( style_ConstantFontSize, 10);
    (deleted)->AddNewFontFace( fontdesc_Italic);
    (marked)->AddNewFontFace( fontdesc_Bold);
    clockcursor = cursor::Create(NULL);
    (clockcursor)->SetStandard( Cursor_Wait);

    return TRUE;
}

void
SetSortOrder()
{
    char tmpstring[256];
    char *string;
    char *tok;
    int i,j;

    for (j = 0; j < 3; j++) {
	sprintf(tmpstring, "%s%s", papertypes[j][0], SortPreference);
#ifdef DEBUG_SORT
printf("Reading preferences for %s\n", tmpstring); fflush(stdout);
#endif
	string = environ::GetProfile(tmpstring);
	if (!string || *string == '\0') {
	    strcpy(tmpstring, papertypes[j][1]);
	    string = tmpstring;
	}

	i = 0;
	tok = strtok(string, ", ");
	do {
#ifdef DEBUG_SORT
	    printf("Setting sort preference to %s\n", tok); fflush(stdout);
#endif
	    if (strcmp(tok, "author") == 0)
		comparison[j][i] = SortByAuthor;
	    else if (strcmp(tok, "filename") == 0)
		comparison[j][i] = SortByFilename;
	    else if (strcmp(tok, "number") == 0)
		comparison[j][i] = SortByNumber;
	    else if (strcmp(tok, "description") == 0)
		comparison[j][i] = SortByDesc;
	    else if (strcmp(tok, "creation") == 0)
		comparison[j][i] = SortByCreate;
	    i++;
	} while ((tok = strtok(NULL,", ")) != NULL && i < SortLimit);
	comparison[j][i] = SortNoMore;
    }

}

void papers::Update()
{
    (this)->EraseVisualRect();
    (this)->FullUpdate( view_FullRedraw, (this)->GetLogicalTop(), (this)->GetLogicalLeft(), (this)->GetLogicalWidth(), (this)->GetLogicalHeight());
}

/* The routines ToggleMark, SimulateClick and papers__Hit were modified from
  messages/lib/captions.c */
void ToggleMark(class papers  *self, struct paperPositions  *node)
/* Toggles whether the paper described by 'node' is marked or not.
  the text display is modified accordingly
  */
{
    if (node->flags & eos_PaperMarked) {
	node->flags &= ~eos_PaperMarked;
	(node->environment)->Delete();
	(self->textobj)->RegionModified( node->textpos, node->textlength);
	(self->markcount)--;
    } else {
	node->flags |= eos_PaperMarked;
	node->environment = new class environment;
	node->environment = (self->textobj->rootEnvironment)->InsertStyle( node->textpos, marked, TRUE);
	(node->environment)->SetLength( node->textlength);
	(self->textobj)->RegionModified( node->textpos, node->textlength);
	(self->markcount)++;
    }
    (self)->WantUpdate( self->textv);
}

void RemoveMarks(class papers  *self, long  rock)
/* Removes all marks. The text display is modified also */
{
    struct paperPositions *node;

    StartWaitCursor();
    for (node = self->Positions; node != NULL;node = node->next)
	if (node->flags & eos_PaperMarked) ToggleMark(self, node);
    StopWaitCursor();
}

void SimulateClick(class papers  *self, boolean  IsLeftClick)
/* this code is based on code in the captions object, used
  in the Andrew messages application.
  */
{
    long start;
    struct paperPositions *node;

    (self)->WantInputFocus( self->textv);
    start = (self->textv)->GetDotPosition();
    node = eosfx::LocatePaper(self->Positions, start, NULL);
    (self->textv)->SetDotPosition( node->textpos);
    (self->textv)->SetDotLength( 0);
    if (IsLeftClick) {
	RemoveMarks(self, 0L);
	if (!(node->flags & eos_PaperDeleted)) ToggleMark(self, node);
    } else
	if (!(node->flags & eos_PaperDeleted)) ToggleMark(self, node);
}

void papers::PostMenus(class menulist  *menulist)
/* We want our menus, but no others! */
{
    (this->menus)->SetMask( this->menuflags);
    (this)->view::PostMenus(this->menus); 
}

void papers::ReceiveInputFocus()
{
    /* We always want the textview to be the star attraction, so that
      it has its cursor visible (almost all of the time)
      */
    (this)->WantInputFocus( this->textv);
}



	static void
add_node(Paperlist item, Tree *tree, int data) {
    Tree n = *tree;
    Tree oldnode = NULL;
    int direction;
    Tree newnode;

    if (n != NULL) {
	while (n != NULL) {
	    oldnode = n;
	    direction = compare(n->data, item, data);
	    if (direction <= 0)
		n = n->left;
	    else
		n = n->right;
	}
	newnode = (Tree) malloc(sizeof(struct node));
	/* What if malloc fails? XXX */
	newnode->data = item;
	newnode->left = newnode->right = NULL;
	newnode->parent = oldnode;
	if (direction <= 0)
	    oldnode->left = newnode;
	else
	    oldnode->right = newnode;
	newnode->count = 0;
    } else {
	*tree = (Tree) malloc(sizeof(struct node));
	(*tree)->data = item;
	(*tree)->left = (*tree)->right = (*tree)->parent = NULL;
	(*tree)->count = 0;
    }
}

static void
sort(Paperlist  *papers, int  data)
{
    Paperlist item;
    Tree tree = NULL;
    Tree tmpnode = NULL;
    Tree nextnode = NULL;
    int direction = 0; /* 0=left, 1=right */
    Paperlist *newlist = papers; /* This is the resorted list */
    int firstitem = 1; /* Boolean flag to show when at top of list */

    if (!papers || !*papers)
	return;

    for (item = *papers; item != NULL; item = item->next)
	add_node(item, &tree, data);

    /* We now do a left-first tree traversal, reordering the initial linked list to follow the order of the tree. A node is only ever looked at going down, so whenever there is a move up, the tree node is free'd */
    tmpnode = tree;
    while (tmpnode != NULL) {
	tmpnode->count++;
	if (tmpnode->count == 2) {
	    /* This is where we reorder the real list */
	    if (!firstitem)
		(*newlist)->next = tmpnode->data;
	    else {
		*papers = tmpnode->data;
		firstitem = 0;
	    }
	    newlist = &(tmpnode->data);
	}
	if (tmpnode->count == 3) {
	    nextnode = tmpnode->parent;
	    free(tmpnode); /* We destroy the tree as we scan it */
	    tmpnode = nextnode;
	} else {
	    if (direction == 0 /* left */) {
		nextnode = tmpnode->left;
	    } else
		nextnode = tmpnode->right;
	    if (nextnode) {
		tmpnode = nextnode;
		direction = 0 /* always try to go left */; 
	    } else
		if (direction == 1 /* right */) {
		    nextnode = tmpnode->parent;
		    free(tmpnode); /* We destroy the tree as we scan it */
		    tmpnode = nextnode;
		} else
		    direction = 1 /* right */;
	}
    }
    /* Close the end of the list */
    (*newlist)->next = NULL;
}
