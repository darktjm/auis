/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("diredview.H")
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <im.H>
#include <view.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <proctable.H>
#include <environment.H>
#include <bind.H>
#include <cursor.H>
#include <message.H>
#include <observable.H>
#include <filetype.H>
#include <search.H>
#include <frame.H>
#include <buffer.H>

#include <dired.H>
#include <diredview.H>

#define Dired(self) \
    ((class dired *) (self)->GetDataObject())

#define WAITON()    im::SetProcessCursor(waitCursor)
#define WAITOFF()   im::SetProcessCursor(NULL)

static class keymap *diredKeymap;
static class menulist *diredMenus;

static class cursor *waitCursor;

#define mmask_None              0
#define mmask_ItemMarked        (1 << 0)
#define mmask_DotFilesOn        (1 << 1)
#define mmask_DotFilesOff       (1 << 2)
#define mmask_LongModeOn        (1 << 3)
#define mmask_LongModeOff       (1 << 4)


ATKdefineRegistry(diredview, textview, diredview::InitializeClass);
static void SetMenuMask(class diredview  *self);
static const char *GetFullName(class diredview  *self, const char  *filename);
static void ptproc_Refresh(class diredview  *self, long  rock);
static int ZoomProc(char  *filename , char  **foundp);
static int VisitFile(class diredview  *self, const char  *fname);
static void ptproc_Zoom(class diredview  *self, long  rock);
static int DeleteProc(char  *filename, class diredview  *self);
static void ptproc_Delete(class diredview  *self, long  rock);
static int RenameProc(char  *filename, class diredview  *self);
static void ptproc_Rename(class diredview  *self, long  rock);
static void ptproc_DownLine(class diredview  *self, long  rock);
static void ptproc_UpLine(class diredview  *self, long  rock);
static void ptproc_ToggleSelect(class diredview  *self, long  rock);
static void ptproc_RegexpSelect(class diredview  *self, long  rock);
static void ptproc_ModeChange(class diredview  *self, long  change);


static void SetMenuMask(class diredview  *self)
{
    class dired *dired = Dired(self);
    int mmask = 0;

    if ((dired)->AnythingMarked())
        mmask |= mmask_ItemMarked;

    if ((dired)->GetLongMode())
        mmask |= mmask_LongModeOn;
    else
        mmask |= mmask_LongModeOff;

    if ((dired)->GetDotFiles())
        mmask |= mmask_DotFilesOn;
    else
        mmask |= mmask_DotFilesOff;

    if ((self->menulist)->SetMask( mmask))
        (self)->textview::PostMenus( self->menulist);
}

/*
 * GetFullName simplifies a filename relative to the
 * currently viewed directory.
 */

static const char *GetFullName(class diredview  *self, const char  *filename)
{
    static char fullName[256];
    char buf[256];
    sprintf(buf, "%s/%s", (Dired(self))->GetDir(), filename);
    filetype::CanonicalizeFilename(fullName, buf, sizeof (fullName));
    return fullName;
}

/*
 * The `ptproc_' routines are the keyboard / menu handlers.
 */

static void ptproc_Refresh(class diredview  *self, long  rock)
{
    char *dir = (Dired(self))->GetDir();
    WAITON();
    if ((Dired(self))->SetDir( dir) < 0) {
        char buf[256];
        if (dir == NULL)
            sprintf(buf, "No directory specified.\n");
        else
            sprintf(buf, "Could not read: %s (%s)\n",
              dir, strerror(errno));
        message::DisplayString(self, 0, buf);
    }
    WAITOFF();
}

static int ZoomProc(char  *filename , char  **foundp)
{
    if (*foundp == NULL) {
        *foundp = filename;
        return TRUE;    /* Keep searching in case others marked */
    } else
        return FALSE;   /* Oops, more than one marked */
}

/* Perform the excrutiating activities necessary to visit file */

static int VisitFile(class diredview  *self, const char  *fname)
{
    class buffer *buffer;
    char realName[1000], buf[1000];
    boolean fileIsDir, fileExists;
    struct stat statBuf;
    class frame *newFrame;
    class im *window;

    filetype::CanonicalizeFilename(realName, fname, sizeof (realName) - 1);
    fname = realName;

    fileExists = fileIsDir = FALSE;
    if (stat(fname, &statBuf) >= 0) {
	fileExists = TRUE;
	if ((statBuf.st_mode & S_IFMT) == S_IFDIR)
	    fileIsDir = TRUE;
    }

    if (!fileExists)
      {
	sprintf (buf, "No such file %s.", fname);
	message::DisplayString (self, 0, buf);
	return -1;
      }
    buffer = buffer::GetBufferOnFile(fname, 0);
    if (buffer == NULL)
      {
	sprintf (buf, "Could not access file %s.\n", fname);
	message::DisplayString (self, 0, buf);
	return -1;
      }

    if((newFrame = new frame) == NULL) {
	fprintf(stderr,"dired: Could not allocate enough memory to visit file.\n");
	return(-1);
    }

    /* Note that frame menulist procs are bound in this procedure */
    (newFrame)->SetCommandEnable( TRUE);

    if((window = im::Create(NULL)) == NULL) {
	fprintf(stderr,"dired: Could not create new window to view file.\n");
	if(newFrame) (newFrame)->Destroy();
	return(-1);
    }
    (window)->SetView( newFrame);

    (newFrame)->PostDefaultHandler( "message",
			     (newFrame)->WantHandler( "message"));
    (newFrame)->SetBuffer( buffer, TRUE);
    return 0;
}

static void ptproc_Zoom(class diredview  *self, long  rock)
{
    class dired *dired = Dired(self);
    struct stat stbuf;
    const char *fname;
    char buf[256], tmpfname[256];

    if (! (dired)->AnythingMarked()) {
        fname = (dired)->Locate( (self)->GetDotPosition());
        if (fname != NULL)
            (dired)->Mark( fname);
    }

    fname = NULL;
    if ((dired)->EnumerateMarked((dired_efptr)ZoomProc, (long)&fname) != NULL) {
        message::DisplayString(self, 0,
          "Cannot zoom into more than one file.\n");
        return;
    }

    if (fname == NULL)
        return;

    /*
     * When changing into "..", try to do it by just
     * stripping the last component.  That way, dired
     * exhibits much more reasonable behavior with
     * symbolic links.
     */

    if (strcmp(fname, "..") == 0) {
        char *s;
        strcpy(tmpfname, (dired)->GetDir());
        for (s = tmpfname; *s; s++) /* Go to end of path */
            ;
        fname = tmpfname;
        if (--s != tmpfname) {      /* Nothing if plain slash */
            if (*s == '/')          /* Zap trailing slash */
                *s-- = '\0';
            while (*s != '/') {     /* Remove trailing component */
                if (s > tmpfname)
                    *s-- = '\0';
                else {  /* Not referenced from root! */
                    fname = "..";
                    goto giveup;
                }
            }
            if (s > tmpfname)      /* Remove slash if not root */
                *s = '\0';
        }
    } else {
      giveup:
        fname = GetFullName(self, fname);
    }

    WAITON();

    if (stat(fname, &stbuf) < 0) {
        sprintf(buf, "Could not stat: %s (%s)\n", fname, strerror(errno));
        WAITOFF();
        message::DisplayString(self, 0, buf);
        return;
    }

    switch (stbuf.st_mode & S_IFMT) {
        case S_IFDIR:
/*
            sprintf(buf, "Reading directory: %s\n", fname);
            message_DisplayString(self, 0, buf);
            im_ForceUpdate();
            if (dired_SetDir(dired, fname) < 0)
                sprintf(buf, "Could not read: %s (%s)\n", fname, strerror(errno));
            else
                strcpy(buf, "Done.\n");
            message_DisplayString(self, 0, buf);
*/
            VisitFile (self, fname);
            break;
        case S_IFREG:
/*
            sprintf(buf, "Not implemented: Editing File: %s\n", fname);
            message_DisplayString(self, 0, buf);
            im_ForceUpdate();
*/
            VisitFile(self, fname);
            break;
        default:    /* S_IFLNK never occurs; other cases are devs. */
            sprintf(buf, "Cannot Read Special File: %s\n", fname);
            message::DisplayString(self, 0, buf);
            break;
    }

    WAITOFF();
}

static int DeleteProc(char  *filename, class diredview  *self)
{
    char buf[256], ques[256];

    sprintf(ques, "Delete %s [n]? ", filename);
    if (message::AskForString(self, 0, ques, NULL, buf, sizeof (buf)) < 0) {
        message::DisplayString(self, 0, "Cancelled.\n");
        return FALSE;
    }

    if (buf[0] == 'y' || buf[0] == 'Y') {
        sprintf(buf, "Deleting: %s\n", filename);
        message::DisplayString(self, 0, buf);
        im::ForceUpdate();
        WAITON();
        if (unlink(GetFullName(self, filename)) < 0) {
            sprintf(buf, "Cannot delete: %s (%s)\n", filename, strerror(errno));
            WAITOFF();
            message::DisplayString(self, 0, buf);
            return FALSE;
        }
        WAITOFF();
    }

    return TRUE;
}

static void ptproc_Delete(class diredview  *self, long  rock)
{
    class dired *dired = Dired(self);

    if (! (dired)->AnythingMarked()) {
        char *fname =
          (dired)->Locate( (self)->GetDotPosition());
        if (fname != NULL)
            (dired)->Mark( fname);
    }

    if ((dired)->EnumerateMarked((dired_efptr)DeleteProc, (long)self) == NULL)
        message::DisplayString(self, 0, "Done.\n");

    ptproc_Refresh(self, 0);
}

static int RenameProc(char  *filename, class diredview  *self)
{
    char buf[256], ques[256], ans[256];

    sprintf(ques, "Rename %s to [don't rename] : ", filename);

    if (message::AskForString(self, 0, ques, NULL, ans, sizeof (ans)) < 0) {
        message::DisplayString(self, 0, "Cancelled.\n");
        return FALSE;
    }

    if (ans[0] != '\0') {
        sprintf(buf, "Renaming: %s\n", filename);
        message::DisplayString(self, 0, buf);
        im::ForceUpdate();
        WAITON();
        /* Have to copy the full name since it's a static buffer */
        strcpy(buf, GetFullName(self, filename));
        if (rename(buf, GetFullName(self, ans)) < 0) {
            sprintf(buf, "Cannot rename: %s (%s)\n", filename, strerror(errno));
            WAITOFF();
            message::DisplayString(self, 0, buf);
            return FALSE;
        }
        WAITOFF();
    }

    return TRUE;
}

static void ptproc_Rename(class diredview  *self, long  rock)
{
    class dired *dired = Dired(self);

    if (! (dired)->AnythingMarked()) {
        char *fname =
          (dired)->Locate( (self)->GetDotPosition());
        if (fname != NULL)
            (dired)->Mark( fname);
    }

    if ((dired)->EnumerateMarked((dired_efptr)RenameProc,(long)self) == NULL)
        message::DisplayString(self, 0, "Done.\n");

    ptproc_Refresh(self, 0);
}

static void ptproc_DownLine(class diredview  *self, long  rock)
{
    /* (textview-next-line proc handles the arg count) */
    struct proctable_Entry *NextPE =
      proctable::Lookup("textview-next-line");
    if (NextPE != NULL && NextPE->proc != NULL)
        (*NextPE->proc)(self, 0);
}

static void ptproc_UpLine(class diredview  *self, long  rock)
{
    /* (textview-previous-line proc handles the arg count) */
    struct proctable_Entry *NextPE =
      proctable::Lookup("textview-previous-line");
    if (NextPE != NULL && NextPE->proc != NULL)
        (*NextPE->proc)(self, 0);
}

static void ptproc_ToggleSelect(class diredview  *self, long  rock)
{
    class dired *dired = Dired(self);
    long count = (self->GetIM())->Argument();
    struct proctable_Entry *NextPE =
      proctable::Lookup("textview-next-line");

    (self->GetIM())->ClearArg();

    if (count > 10000)
        count = 10000;

    while (count--) {
        long pos = (self)->GetDotPosition();
        char *fname = (dired)->Locate( pos);

        if (fname != NULL) {
            if ((dired)->IsMarked( fname))
                (dired)->Unmark( fname);
            else
                (dired)->Mark( fname);
        }

        if (NextPE != NULL && NextPE->proc != NULL)
            (*NextPE->proc)(self, 0);
    }
}

static void ptproc_RegexpSelect(class diredview  *self, long  rock)
{
    class dired *dired = Dired(self);
    char buf[256];
    const char *res;
    static class search pat;
    long pos;

    if (message::AskForString(self, 0,
      "Regular expression: ", ".*", buf, sizeof (buf)) < 0
       || buf[0] == '\0') {
        message::DisplayString(self, 0, "Cancelled.\n");
        return;
    }

    res = pat.CompilePattern(buf);
    if (res != NULL) {
        message::DisplayString(self, 0, res);
        return;
    }

    WAITON();

    pos = 0;
    while (1) {
        struct stat stbuf;
        pos = pat.MatchPattern(dired, pos);
        if (pos < 0 || (res = (dired)->Locate( pos)) == NULL)
            break;
        if (stat(GetFullName(self, res), &stbuf) >= 0 &&
          (stbuf.st_mode & S_IFMT) != S_IFDIR)
            (dired)->Mark( res);
        while (++pos < (dired)->GetLength())
            if ((dired)->GetChar( pos) == '\n')
                break;
    }

    WAITOFF();
    message::DisplayString(self, 0, "Done.\n");
}

static void ptproc_ModeChange(class diredview  *self, long  change)
{
    class dired *dired = Dired(self);
    switch (change) {
        case 0: (dired)->SetLongMode( TRUE); break;
        case 1: (dired)->SetLongMode( FALSE); break;
        case 2: (dired)->SetDotFiles( TRUE); break;
        case 3: (dired)->SetDotFiles( FALSE); break;
    }
    ptproc_Refresh(self, 0);
}

/*
 * Class procedures
 */

static struct bind_Description diredBindings[] = {
    {   "dired-zoom-in", "z", 0,
        "Directory Edit,Zoom In~10",
         0, mmask_ItemMarked, (proctable_fptr)ptproc_Zoom,
        "Edit selected file or switch to selected directory." },
    {   "dired-delete", "d", 0,
        "Directory Edit,Delete~20",
         0, mmask_ItemMarked, (proctable_fptr)ptproc_Delete,
         "Delete selected file(s)." },
    {   "dired-rename", "r", 0,
        "Directory Edit,Rename~30",
         0, mmask_ItemMarked, (proctable_fptr)ptproc_Rename,
         "Rename selected file(s)." },
    {   "dired-refresh", "\r", 0,
         "Directory Edit,Refresh~40",
         0, mmask_None, (proctable_fptr)ptproc_Refresh,
         "Reread current directory." },
    {    "dired-toggle-select", " ", 0,
         NULL, 0, 0, (proctable_fptr)ptproc_ToggleSelect,
         "Toggle file at cursor." },
    {    "dired-down-line", "n", 0,
         NULL, 0, 0, (proctable_fptr)ptproc_DownLine,
         "Move cursor down (equiv ^N)." },
    {    "dired-up-line", "p", 0,
         NULL, 0, 0, (proctable_fptr)ptproc_UpLine,
         "Move cursor up (equiv ^P)." },
    {    "dired-regexp-select", "=", 0,
         "Directory Edit,Regexp Select~60",
         0, mmask_None, (proctable_fptr)ptproc_RegexpSelect,
         "Select files by regular expression." },
    {    "dired-long-mode-on", "l", 0,
         "Directory Edit,Long Format~50",
         0, mmask_LongModeOff, (proctable_fptr)ptproc_ModeChange,
         "List files in long format." },
    {    "dired-long-mode-off", "s", 1,
         "Directory Edit,Short Format~50",
         1, mmask_LongModeOn, (proctable_fptr)ptproc_ModeChange,
         "List files in normal format (names only)." },
    {    "dired-dot-files-on", "i", 2,
         "Directory Edit,Show Invisible~55",
         2, mmask_DotFilesOff, (proctable_fptr)ptproc_ModeChange,
         "Display invisible files." },
    {    "dired-dot-files-off", "h", 3,
         "Directory Edit,Hide Invisible~55",
         3, mmask_DotFilesOn, (proctable_fptr)ptproc_ModeChange,
         "Do not display invisible files." },
    NULL
};

boolean diredview::InitializeClass()
{
    diredKeymap = new keymap;
    diredMenus = menulist::Create(NULL);

    bind::BindList(diredBindings, diredKeymap, diredMenus, &diredview_ATKregistry_ );

    waitCursor = cursor::Create(NULL);
    (waitCursor)->SetStandard( Cursor_Wait);

    return TRUE;
}

diredview::diredview()
{
	ATKinit;

    this->keystate = keystate::Create(this, diredKeymap);
    this->menulist = (diredMenus)->DuplicateML( this);
    (this->menulist)->SetMask( mmask_None);
    THROWONFAILURE( TRUE);
}

diredview::~diredview()
{
	ATKinit;

    if(this->menulist) delete this->menulist;
}

/*
 * Overrides
 */

void diredview::SetDataObject(class dataobject   *object)
{
    class dired *dired = (class dired *) object;
    if ((dired)->GetDir() == NULL)
        (dired)->SetDir( ".");
    (this)->textview::SetDataObject( object);
    (dired)->NotifyObservers( 0);
}

class view *diredview::Hit(enum view::MouseAction  action, long  x , long  y , long  numberOfClicks)
{
    int button;

    switch (action) {
        default:
            button = 0;
            break;
        case view::LeftDown:
        case view::LeftMovement:
            button = 1;     /* Left button; select */
            break;
        case view::RightDown:
        case view::RightMovement:
            button = 2;     /* Right button; deselect */
            break;
    }

    if (button) {
        class dired *dired = Dired(this);
        long pos = (this)->Locate( x, y, NULL);
        char *fname = (dired)->Locate( pos);

        if (fname) {
            if (button == 1)
                (dired)->Mark( fname);
            else
                (dired)->Unmark( fname);

            while (pos > 0 &&
              (dired)->GetChar( pos - 1) != '\n')
                pos--;
            (this)->SetDotPosition( pos);
            (this)->SetDotLength( 0);
        }
    }

    return (class view *) this;
}

void diredview::PostKeyState(class keystate  *keystate)
{
    (this->keystate)->AddBefore( keystate);
    (this)->textview::PostKeyState( this->keystate);
}

void diredview::PostMenus(class menulist  *menulist)
{
    (this->menulist)->ChainAfterML( menulist, 0);
    (this)->textview::PostMenus( this->menulist);
}

void diredview::ObservedChanged(class observable  *changed, long  value)
{
    SetMenuMask(this);
    (this)->textview::ObservedChanged( changed, value);
}
