/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("dired.H")
#if POSIX_ENV || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif
#include <attribs.h>

#include <sys/stat.h>
#include <stylesheet.H>
#include <style.H>
#include <environment.H>
#include <fontdesc.H>
#include <filetype.H>
#include <list.H>

#include <dired.H>

#define RootEnv(dired) \
    ((class environment *) (dired)->rootEnvironment)

/*
 * DirIntoList returns a list of the files in a directory.
 * The elements of the list are fileinfo structures.
 * The directory is referenced from the current directory.
 * If the directory cannot be loaded, returns NULL and
 * errno contains the error code from the opendir(2) call.
 */


ATKdefineRegistryNoInit(dired, text);
static int CompareFilenameProc(struct fileinfo  *f1 , struct fileinfo  *f2);
static void LongModeLine(char  *dname , char  *fname , char  *buf);
static class list *DirIntoList(char  *dname, boolean  longMode , boolean  dotFiles);
static int FreeProc(struct fileinfo  *fi, long  rock);
static void DestroyList(class list  *list);
static int InsTextProc(struct fileinfo  *fi, class dired  *dired);
static void ListIntoText(class dired  *self, class list  *list);
static void SetupStyles(class dired  *self);
static int FindNameProc(struct fileinfo  *fi, const char  *name);
static struct fileinfo *LookupFile(class dired  *self, const char  *name);
static int FindPosProc(struct fileinfo  *fi, long  pos);
static struct fileinfo *LookupPos(class dired  *self, long  pos);
static void WrapStyle(class dired  *self, struct fileinfo  *fi, class style  *style);
static int AnythingProc(struct fileinfo  *fi, long  rock);
static long EnumProc(struct fileinfo  *fi, struct emargs  *args);
static char *DoEnumerate(class dired  *self, dired_efptr  proc, long  rock, boolean  all);


static int CompareFilenameProc(struct fileinfo  *f1 , struct fileinfo  *f2)
{
    return strcmp(f1->fileName, f2->fileName);
}

static void LongModeLine(char  *dname , char  *fname , char  *buf)
{
    struct stat stbuf;
    strcpy(buf, dname);
    if(buf[strlen(buf) - 1] != '/')
	strcat(buf, "/");
    strcat(buf, fname);
    if (stat(buf, &stbuf) < 0) {
        strcpy(buf, "? ");
        strcat(buf, fname);
        return;
    }
}

#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static class list *DirIntoList(char  *dname, boolean  longMode , boolean  dotFiles)
{
    class list *listp;
    DIR *dirp;
    struct dirent *dp;
    static char fname[PATH_MAX];
    int dirlen;
    struct stat st;

    if ((dirp = opendir(dname)) == NULL)
        return NULL;

    dirlen = strlen(dname);
    memcpy(fname, dname, dirlen);
    if(!dirlen || fname[dirlen - 1] != '/')
	fname[dirlen++] = '/';
    listp = new list;

    while ((dp = readdir(dirp)) != NULL) {
        struct fileinfo *fi;

        if (! dotFiles && dp->d_name[0] == '.' &&
          strcmp(dp->d_name, "..") != 0)
            continue;

        fi = (struct fileinfo *) malloc(sizeof (struct fileinfo));

        fi->fileName = strdup(dp->d_name);

	strcpy(fname + dirlen, dp->d_name);
	fi->islink = FALSE;
	if(!lstat(fname, &st))
	    fi->islink = (st.st_mode & S_IFMT) == S_IFLNK;
	if(stat(fname, &st))
	    fi->type = fi->islink ? S_IFLNK : -1;
	else
	    fi->type = st.st_mode & S_IFMT;
	   
	// the original author(s) had no sense of efficiency or stack limits
	// but I'll leave this broken for now
        if (longMode) {
            char buf[256];
            LongModeLine(dname, fi->fileName, buf);
            fi->dispName = strdup(buf);
        } else {
            fi->dispName = strdup(fi->fileName);
        }

        fi->pos = fi->len = 0;
        fi->env = NULL;     /* Not marked (no env) */

        (listp)->InsertSorted((char *)fi, (list_greaterfptr)CompareFilenameProc);
    }

    closedir(dirp);
    return listp;
}

/*
 * Free a list
 */

static int FreeProc(struct fileinfo  *fi, long  rock)
{
    free(fi->fileName);
    free(fi->dispName);
    return TRUE;
}

static void DestroyList(class list  *list)
{
    (list)->Enumerate((list_efptr) FreeProc, 0);
    delete list;
}

/*
 * ListIntoText clears the text, then inserts one line for each file.
 * The starting position for each file's text (and the length) is
 * updated in each list entry. A carriage return is inserted after
 * each entry.  Highlight styles are not wrapped around the highlighted
 * (env != NULL) entries, but regular styles are.
 */

static void StyleByType(class dired *self, struct fileinfo *fi)
{
    const char *stname;
    switch(fi->type) {
      case S_IFREG: stname = "dir_file"; break;
      case S_IFDIR: stname = "dir_dir"; break;
      case S_IFLNK: stname = "dir_badlink"; break;
      case S_IFIFO: case S_IFSOCK: stname = "dir_fifo"; break;
      case S_IFCHR: case S_IFBLK: stname = "dir_dev"; break;
      default:  stname = "dir_unknown"; break;
    }
    style *style = self->styleSheet->Find(stname);
    if(!style) {
	style = new class style;
	style->SetName(stname);
	self->styleSheet->Add(style);
    }
    self->rootEnvironment->WrapStyle(fi->pos, fi->len, style);

    if(fi->islink) {
	style = self->styleSheet->Find("dir_link");
	if(!style) {
	    style = new class style;
	    style->SetName("dir_link");
	    self->styleSheet->Add(style);
	}
	self->rootEnvironment->WrapStyle(fi->pos, fi->len, style);
    }
}

static int InsTextProc(struct fileinfo  *fi, class dired  *dired)
{
    fi->pos = (dired)->GetLength();
    fi->len = strlen(fi->dispName);

    (dired)->InsertCharacters( fi->pos, fi->dispName, fi->len);
    (dired)->InsertCharacters( fi->pos + fi->len, "\n", 1);
    StyleByType(dired, fi);

    return TRUE;
}



static void ListIntoText(class dired  *self, class list  *list)
{

    (self)->SetReadOnly( FALSE);
    (self)->Clear();
    SetupStyles(self);
    (self->flist)->Enumerate((list_efptr) InsTextProc, (char *)self);
    (self)->SetReadOnly( TRUE);
    (self)->NotifyObservers( 0);
}

/*
 * Class procedures
 */

static void SetupStyles(class dired  *self)
{
    struct attributes templateAttribute;
    class style *stylep;

    templateAttribute.key = "template";
    templateAttribute.value.string = "dired";
    templateAttribute.next = NULL;

    (self)->SetAttributes( &templateAttribute);

    stylep = (self->styleSheet)->Find( "select");
    if (stylep == NULL) {
        stylep = new style;
        (stylep)->AddNewFontFace( fontdesc_Bold);
    }

    self->markedStyle = stylep;

    stylep = (self->styleSheet)->Find( "global");
    if (stylep != NULL)
        (self)->SetGlobalStyle( stylep);
}

dired::dired()
{
    this->dir = NULL;
    this->flist = NULL;
    this->longMode = FALSE;
    this->dotFiles = FALSE;

    (this)->SetReadOnly( TRUE);
    (this)->SetExportEnvironments( FALSE);

    THROWONFAILURE( TRUE);
}

dired::~dired()
{
    if (this->flist != NULL)
        DestroyList(this->flist);
    delete this->markedStyle;
}

/*
 * Overrides
 *
 * SetAttributes is used to tell dired which dir to use.
 * It's not very proper, but it's necessary so that the buffer
 * package has to know a bare minimum about dired.
 *
 * NOTE: the "dir" attribute should be last in the attr list
 */

void dired::SetAttributes(struct attributes  *attributes)
{
    (this)->text::SetAttributes( attributes);

    for (; attributes != NULL; attributes = attributes->next)
        if (strcmp(attributes->key, "dir") == 0)
            (this)->SetDir( attributes->value.string);
        else if (strcmp(attributes->key, "longmode") == 0)
            (this)->SetLongMode( attributes->value.integer);
        else if (strcmp(attributes->key, "dotfiles") == 0)
            (this)->SetDotFiles( attributes->value.integer);
}

/*
 * Prevent checkpoints
 */

long dired::GetModified()
{
    return 0;
}

/*
 * Methods
 */

/*
 * SetDir reads a directory into the list and into the document.
 * The current longMode and dotFiles are used.
 * If an error occurs in opening the directory, then the
 * routine: returns -1, does not at all affect the list or
 * text, and leaves the error code from opendir(2) in errno.
 */

long dired::SetDir(const char  *dname)
{
    char newDir[256];
    class list *newList;

    if (dname == NULL)
        return -1;

    filetype::CanonicalizeFilename(newDir, dname, sizeof (newDir));

    newList = DirIntoList(newDir, this->longMode, this->dotFiles);
    if (newList == NULL)
        return -1;

    if (this->dir != NULL)
        free(this->dir);
    this->dir = strdup(newDir);

    if (this->flist != NULL)
        DestroyList(this->flist);

    this->flist = newList;
    ListIntoText(this, newList);

    return 0;
}

char *dired::GetDir()
{
    return this->dir;
}

static int FindNameProc(struct fileinfo  *fi, const char  *name)
{
    return (strcmp(fi->fileName, name) != 0);
}

static struct fileinfo *LookupFile(class dired  *self, const char  *name)
{
    if (self->flist == NULL)
        return NULL;
    return (struct fileinfo *)
      (self->flist)->Enumerate((list_efptr) FindNameProc, name);
}

static int FindPosProc(struct fileinfo  *fi, long  pos)
{
    return (pos < fi->pos || pos > fi->pos + fi->len);
}

static struct fileinfo *LookupPos(class dired  *self, long  pos)
{
    if (self->flist == NULL)
        return NULL;
    return (struct fileinfo *)
      (self->flist)->Enumerate((list_efptr) FindPosProc, (char *)pos);
}

/*
 * Given a position in the document, returns the filename in
 *  in which the document position falls.
 */

char *dired::Locate(long  pos)
{
    struct fileinfo *fi = LookupPos(this, pos);
    return (fi == NULL) ? NULL : fi->fileName;
}

/*
 * Routine to wrap a specified style around a given file
 * entry.  Previous styles are removed.
 * If the style is NULL, then no style is wrapped but the
 * previous style is still removed.
 * Does not change the document unnecessarily.
 */

static void WrapStyle(class dired  *self, struct fileinfo  *fi, class style  *style)
{
    (self)->SetReadOnly( FALSE);

    if (fi->env != NULL) {
        if (fi->env->data.style == style)
             return;
	fi->env->Delete();
        (self)->RegionModified( fi->pos, fi->len);
        (self)->NotifyObservers( 0);
    }

    fi->env = NULL;

    if (style != NULL) {
	fi->env = self->rootEnvironment->WrapStyle(fi->pos, fi->len, style);
        (self)->RegionModified( fi->pos, fi->len);
        (self)->NotifyObservers( 0);
    }

    (self)->SetReadOnly( TRUE);
}

/*
 * Given a filename, mark it (if it wasn't marked)
 */

void dired::Mark(const char  *fname)
{
    struct fileinfo *fi;
    fi = LookupFile(this, fname);
    if (fi != NULL)
        WrapStyle(this, fi, this->markedStyle);
}

void dired::Unmark(const char  *fname)
{
    struct fileinfo *fi = LookupFile(this, fname);
    if (fi != NULL)
        WrapStyle(this, fi, NULL);
}

boolean dired::IsMarked(const char  *fname)
{
    struct fileinfo *fi = LookupFile(this, fname);
    return (fi != NULL && fi->env != NULL);
}

static int AnythingProc(struct fileinfo  *fi, long  rock)
{
    return (fi->env == NULL);
}

boolean dired::AnythingMarked()
{
    if (this->flist == NULL)
        return FALSE;
    return ((this->flist)->Enumerate((list_efptr) AnythingProc, 0) != NULL);
}

struct emargs {
    dired_efptr proc;
    boolean all;
    long rock;
};

/*
 * EnumerateAll calls proc for every listed file.
 * EnumerateMarked calls proc only for currently highlighted files.
 *
 * The enumerates call the proc with arguments (filename, rock).
 * If the proc returns FALSE, the enumeration stops and returns the
 * name of the file being processed.  Otherwise, the enumeration
 * continues until the end, in which case NULL is returned.
 */

static long EnumProc(struct fileinfo  *fi, struct emargs  *args)
{
    if (args->all || fi->env != NULL)
        return (long)(*args->proc)(fi->fileName, args->rock);
    return TRUE;
}

static char *DoEnumerate(class dired  *self, dired_efptr  proc, long  rock, boolean  all)
{
    struct fileinfo *result;
    struct emargs args;
    if (self->flist == NULL)
        return NULL;
    args.proc = proc, args.rock = rock, args.all = all;
    result = (struct fileinfo *) (self->flist)->Enumerate((list_efptr) EnumProc, (char *)&args);
    return (result == NULL) ? NULL : result->fileName;
}

char *dired::EnumerateMarked(dired_efptr  proc, long  rock)
{
    return DoEnumerate(this, proc, rock, FALSE);
}

char *dired::EnumerateAll(dired_efptr  proc, long  rock)
{
    return DoEnumerate(this, proc, rock, TRUE);
}
