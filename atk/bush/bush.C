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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/bush/RCS/bush.C,v 1.21 1996/09/03 19:05:56 robr Exp $";
#endif

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Bush Data-object

MODULE	bush.c

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Bush Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)

END-SPECIFICATION  ************************************************************/

struct map_item { 
  int	 uid;
  char	*uname;
  char	*ucell;
};

#include <andrewos.h>
ATK_IMPL("bush.H")

#include <pwd.h>
#ifdef AFS_ENV
#include <afs/param.h>
#include <util.h>		    /*getv*(), getc*() routine family*/
#ifdef WHITEPAGES_ENV
#include <wp.h>			    /*White Pages*/
#endif /* WHITEPAGES_ENV */
#include <netinet/in.h>	    /* For AFS 3.2 */
#include <afs/vice.h>
#include <afs/venus.h>
#endif /* AFS_ENV */

#include <dataobject.H>
#include <filetype.H>
#include <im.H>
#include <vector.H>
#include <environ.H>
#include <apts.H>
#include <bush.H>  /* includes tree.ih */

#define	GivenDirName		    (self->given_dir_name)
#define	RootPath		    (self->root_pathname)
#define	UidUnameMap		    (self->uid_uname_map)
#define	MyCellName		    (self->mycellname)
#define	Debug			    (self->debug)
#define	TREE			    ((self)->tree)
#define	Root			    ((TREE)->RootNode())
#define	RootPathName		    ((self)->root_pathname)
#define	DIR(tn)			    ((struct Dir_*)(TREE)->NodeDatum(tn))
#define	DirMode(tn)		    (DIR(tn)->mode)
#define	DIRPATH(tn)		    (DIR(tn)->path)
#define	DirName(tn)		    (DIR(tn)->name)
#define	DirTimeStamp(tn)	    (DIR(tn)->time_stamp)
#define	RootDirPath		    (DIRPATH(Root))
#define	DirEntries(tn)		    (DIR(tn)->Dir_Entries)
#define	DirEntriesCount(tn)	    (DirEntries(tn)->count)
#define	DirEntryPtr(tn)		    (DirEntries(tn)->Entry)    
#define	DirEntry(tn,i)		    (DirEntries(tn)->Entry[i])
#define	DirEntryMode(tn,i)	    (DirEntry(tn,i)->mode)
#define	DirEntryPos(tn,i)	    (DirEntry(tn,i)->position)
#define	DirEntryName(tn,i)	    (DirEntry(tn,i)->name)
#define	DirEntryLinkName(tn,i)	    (DirEntry(tn,i)->link_name)
#define	DirEntryType(tn,i)	    (DirEntry(tn,i)->type)
#define	DirEntryOwner(tn,i)	    (DirEntry(tn,i)->owner)
#define	DirEntryNLinks(tn,i)	    (DirEntry(tn,i)->nlinks)
#define	DirEntryTimeStamp(tn,i)	    (DirEntry(tn,i)->time_stamp)
#define	DirEntrySize(tn,i)	    (DirEntry(tn,i)->size)
#define	DirEntryPerms(tn,i)	    (DirEntry(tn,i)->permissions)
#define	DirEntryParent(tn,i)	    (DirEntry(tn,i)->parent)
#define	DirEntryInstance(tn,i)	    (DirEntry(tn,i)->instance)
#define	DirEntryParentDir(tn,i)	    (DIR(DirEntryParent(tn,i)))    
#define	RootDir			    (DIR(Root))
#define	Parent(tn)		    ((TREE)->ParentNode(tn))
#define	Child(tn)		    ((TREE)->ChildNode(tn))
#define	Left(tn)		    ((TREE)->LeftNode(tn))
#define	Right(tn)		    ((TREE)->RightNode(tn))
#define	ParentDir(tn)		    (DIR(Parent(tn)))
#define	ChildDir(tn)		    (DIR(Child(tn)))
#define	LeftDir(tn)		    (DIR(Left(tn)))
#define	RightDir(tn)		    (DIR(Right(tn)))

#define	no_space		    2
#define	no_superior		    3
#define	no_inferior		    4
#define	scan_failure		    5
#define	read_failure		    6

#define	AllocNameSpace(s,t)	    apts::CaptureString(s,t)

tree_Specification DirTree[] = {tree_Order(tree_PreOrder), 0};

char				baseName[] = "/afs"; /*Pathname to give to pioctl()*/
#define	MAX_PIOCTL_BUFF_SIZE	1000
 


ATKdefineRegistry(bush, apt, NULL);
static int ExtractNodePath( register class bush     *self, register char		   *source , register char		   **path );
static int ExtractNodeName( register char		 *source, register char	        **name );
static char * gethomecell( register class bush   *self, register char		 *filename );
static char * getcell( register class bush   *self, register char		 *filename );
static char * getname( register class bush   *self, register int   	  uid, register char		 *cell );

static int
NodeFilter(SCANDIRSELARG_TYPE *dir )
  {
  return(!(*dir->d_name == '.' && 
	  (*(dir->d_name+1) == '.' || 
	 *(dir->d_name+1) == '\0')));
}

class bush *
bush::Create( char			 *init_dir )
    {
  register class bush  *self = NULL;

  if(self = new bush) {
    if(init_dir && (init_dir[0] != '\0'))
      strcpy(GivenDirName,init_dir);
    else im::GetDirectory(GivenDirName);
    (self)->InitTree(GivenDirName);
    (self)->BuildSubDirs((self)->TreeRoot());
  }
  OUT(bush_Create);
  return(self);
}

bush::bush( )
    {
    class bush *self=this;
  TREE = tree::Create(DirTree,this);
  RootPath = NULL;
  *GivenDirName = 0;
  Debug = 0;
  UidUnameMap = vector::Create(30,3);
  MyCellName[0] = '\0';
  THROWONFAILURE((TRUE));
}

bush::~bush() {
    ClearTree();
}

void bush::ClearTree() {
    bush *self=this;
    if(Root) {
	struct Dir_ *oldDir=DIR(Root);
	if(oldDir) {
	    if(oldDir->path) free(oldDir->path);
	    if(oldDir->name) free(oldDir->name);
	    if(Child(Root))
		(this)->DestroySubDirs( Root);
	    (this)->DestroyDirEntries( Root);
	    free(oldDir);
	}
    }
}

static int
ExtractNodePath( register class bush *self, register char *source, register char **path )
{
  register long     status = 0, i = 0, len;
  char		    full_path[MAXPATHLEN + 1], 
		    workingPathName[MAXPATHLEN + 1];
  register char	   *ptr;

  IN(ExtractNodePath);
  getwd(workingPathName);
  strcpy(full_path,workingPathName);
  switch(*source) {
    case '.':
      while(*(source + i) == '.' && *(source + i + 1) == '.' &&
  	     (*(source + i + 2) == '\0' || *(source + i + 2) == '/')) {
	if(ptr = (char*)strrchr(full_path,'/')) 
	  *ptr = '\0';
	if(*(source + i + 2) == '\0') {
	  i += 2;
	  break;
	}
	else i += 3;
      }
      if(i && *(source + i) != '\0') {
	strcat(full_path,"/");
	strcat(full_path,source + i);
      }
      source = full_path;
      break;
    default:
      if(*source != '/') {
	strcat(full_path,"/");
	strcat(full_path,GivenDirName);
	source = full_path;
      }
  }
  len = strlen(source);
  if ((len > 1) && source[len - 1] == '/')
      source[len - 1] = (char) 0;
  if(!status && (*path = (char*)malloc(strlen(source)+1)))
    strcpy(*path,source);
  else status = -1;
  OUT(ExtractNodePath);
  return(status);
}

static int
ExtractNodeName( register char	*source, register char **name )
{
  register long		 status = 0, len;
  register char		*ptr = NULL;

  if(source && (len = strlen(source)) > 0) {
      if((len > 1) && source[len-1] == '/')
	  source[len-1] = (char) 0;
      if((ptr = (char *)strrchr(source, '/')) && *(source + 1) != '\0')
	  source = ++ptr;
      if(*name = (char *)malloc(strlen(source) + 1))
	  strcpy(*name, source);
      else
	  status = -1;
  }
  else status = -1;
  return(status);
}

void
bush::InitTree( register char		 *root_path )
    {
    class bush *self=this;
  tree_type_node	 root = NULL;
  struct Dir_		*rootDir = (struct Dir_ *)calloc(1,sizeof(struct Dir_));
  char			*nodeName = NULL, tmp[MAXPATHLEN];
  struct stat		 stats;

  IN(bush_InitTree);
 
  if(RootPath) {
    free(RootPath);
    RootPath = NULL;
  }
  ExtractNodePath(this,root_path,&RootPath);
  if(access(RootPath,F_OK) < 0) {
      printf("bush: directory '%s' does not exist or cannot be searched.\n",
	       RootPath);
      free(RootPath);
      im::GetDirectory(GivenDirName);
      AllocNameSpace(GivenDirName,&RootPath);
  }
  else strcpy(GivenDirName,root_path);
  strcpy(tmp,RootPath);
  ExtractNodeName(tmp,&nodeName);
  AllocNameSpace(nodeName,&rootDir->name);
  free(nodeName);
  if(stat(RootPath,&stats) < 0) {
      printf("bush: error '%s' encountered while scanning '%s'.\n", sys_errlist[errno]);
      return;
  }
  else {
      gethomecell(this,baseName);
    root = (TREE)->CreateRootNode(rootDir->name,(long)rootDir);
    rootDir->tn = root;
    AllocNameSpace(RootPath,&rootDir->path);
    (this)->ScanDir(root);
    DirMode(root).selected = TRUE;
  }
  OUT(bush_InitTree);
}

void
bush::DestroySubDirs( register tree_type_node     tn )
    {
    class bush *self=this;
  IN(bush_DestroySubDirs);
  (this)->FreeSubDirs(tn);
  (TREE)->DestroyNodeChildren(tn);
  IN(bush_DestroySubDirs);
}

static char *
gethomecell( register class bush   *self, register char		 *filename )
{
#ifdef AFS_ENV
#if 0
    struct ViceIoctl	 blob;
  int			 outcome;

  blob.in_size  = sizeof(baseName);
  blob.in       = baseName;
  blob.out_size = MAX_PIOCTL_BUFF_SIZE;
  blob.out      = MyCellName;
  
  outcome = pioctl(baseName,VIOC_GET_PRIMARY_CELL,&blob,1);
  if(outcome) {
    blob.in_size  = sizeof(baseName);
    blob.in       = baseName;
    blob.out_size = MAX_PIOCTL_BUFF_SIZE;
    blob.out      = MyCellName;

    outcome = pioctl(baseName,VIOC_GET_WS_CELL,&blob,1);
    if(outcome) 
      sprintf(MyCellName,"%s","andrew.cmu.edu");
    return(MyCellName);
  }
#else
  char *home=environ::GetHome(NULL);
  if(home && GetCellFromFileName(home, MyCellName, sizeof(MyCellName))==0) return MyCellName;
  if(GetCurrentWSCell(MyCellName, sizeof(MyCellName))==0) return MyCellName;  
#endif
#endif /* AFS_ENV */
  return(NULL);
}

static char *
getcell( register class bush   *self, register char		 *filename )
    {
#ifdef AFS_ENV
  static char		 residence[MAX_PIOCTL_BUFF_SIZE];
#if 0
  struct ViceIoctl	 blob;

  blob.in_size  = sizeof(filename);
  blob.in       = filename;
  blob.out_size = MAX_PIOCTL_BUFF_SIZE;
  blob.out      = residence;

  if(pioctl(filename,VIOC_FILE_CELL_NAME,&blob,1))
    return(MyCellName);
  return(residence);
#else
  if(GetCellFromFileName(filename, residence, sizeof(residence))==0) return residence;
  else return "";
#endif
#else
  return("");
#endif /* AFS_ENV */
}

static char *
getname( register class bush   *self, register int   	  uid, register char		 *cell )
      {
  register int		     i = 0;
  register struct map_item  *item = NULL;
  char			    *uname = NULL;
  register struct passwd    *pw = NULL;
#ifdef AFS_ENV
  for( i = 0 ; i < (UidUnameMap)->Count() ; i++ ) {
    item = (struct map_item*)(UidUnameMap)->Item(i);
    if((uid == item->uid) && cell && item->ucell && !strcmp(cell,item->ucell)) {
      uname = item->uname;
      break;
    }
  }
  if(!uname) {
    if(pw = (struct passwd *) getcpwuid(uid,cell)) {
      item = (struct map_item*)calloc(1,sizeof(struct map_item));
      item->uid = uid;
      AllocNameSpace(pw->pw_name,&item->uname);
      AllocNameSpace(cell,&item->ucell);
      (UidUnameMap)->AddItem((long)item);
      uname = item->uname;
    }
    else {
      char	    uid_str[200];

      item = (struct map_item*)calloc(1,sizeof(struct map_item));
      item->uid = uid;
      sprintf(uid_str,"%u@%s",uid,cell);
      AllocNameSpace(uid_str,&item->uname);
      AllocNameSpace(cell,&item->ucell);
      (UidUnameMap)->AddItem((long)item);
      uname = item->uname;
    }
  }
#else /* AFS_ENV */
  for( i = 0 ; i < (UidUnameMap)->Count() ; i++ ) {
    item = (struct map_item*)(UidUnameMap)->Item(i);
    if(uid == item->uid) {
      uname = item->uname;
      break;
    }
  }
  if(!uname) {
    if(pw = getpwuid(uid)) {
      item = (struct map_item*)calloc(1,sizeof(struct map_item));
      item->uid = uid;
      AllocNameSpace(pw->pw_name,&item->uname);
      AllocNameSpace("",&item->ucell);
      (UidUnameMap)->AddItem((long)item);
      uname = item->uname;
    }
    else {
      char	    uid_str[200];

      item = (struct map_item*)calloc(1,sizeof(struct map_item));
      item->uid = uid;
      sprintf(uid_str,"%u",uid);
      AllocNameSpace(uid_str,&item->uname);
      AllocNameSpace("",&item->ucell);
      (UidUnameMap)->AddItem((long)item);
      uname = item->uname;
    }
  }
#endif /* AFS_ENV */
  return(uname);
}

int
bush::ScanDir( register tree_type_node tn )
{
    class bush *self=this;
    register long i = 0, status = ok, count = 0;
    register char *ptr = NULL;
    DIRENT_TYPE **anchor = NULL;
    struct stat stats, lstats;
    char fullEntryName[MAXPATHLEN+25], buf[MAXPATHLEN];
    char workingDir[MAXPATHLEN];
    int cc = 0;

  IN(bush_ScanDir);
  if(!tn) return(scan_failure);
  getwd(workingDir);
  chdir(DIRPATH(tn));
  if(stat(DIRPATH(tn),&stats)) {
    status = scan_failure;
    DirMode(tn).stat_failed = TRUE;
  }
  else DirTimeStamp(tn) = stats.st_mtime;
  if(status == ok  && 
     (count = scandir(DIRPATH(tn), &anchor, SCANDIRSELFUNC(NodeFilter), SCANDIRCOMPFUNC(alphasort))) < 0) {
    status = read_failure;
    DirMode(tn).scan_failed = TRUE;
  }
  else if(status == ok) {
    if(Child(tn))
	(this)->DestroySubDirs( tn);
    (this)->DestroyDirEntries( tn);
    if(count)
      if(!(DirEntries(tn) = (struct Dir_Entries*) 
	calloc(1, sizeof(struct Dir_Entries))))
	  status = no_space;
      else {
        DirEntryPtr(tn) = 
	    (struct Dir_Entry **) calloc(count,sizeof(struct Dir_Entry*));
	DirEntriesCount(tn) = count;
      }
    for( i = 0; i < count && status == ok; i++ ) {
      DirEntry(tn,i) = (struct Dir_Entry*)calloc(1,sizeof(struct Dir_Entry));
      DirEntryPos(tn,i) = i;
      AllocNameSpace(anchor[i]->d_name, &DirEntryName(tn,i));
      ptr = DirEntryName(tn,i);
      while(*ptr != '\0') {
	if(!(isascii(*ptr) && isprint(*ptr))) *ptr = '.';
	ptr++;
      }
      sprintf(fullEntryName,"%s/%s",DIRPATH(tn),anchor[i]->d_name);
      if(lstat(fullEntryName,&stats) < 0) 
	DirEntryMode(tn,i).stat_failed = TRUE;
      else {
	if((stats.st_mode & S_IFMT) == S_IFDIR)	
	  DirEntryType(tn,i).dir = TRUE;
#ifdef S_IFLNK
	else if((stats.st_mode & S_IFMT) == S_IFLNK) {
	  DirEntryType(tn,i).soft_link = TRUE;
	  if(cc = readlink(fullEntryName,buf,MAXPATHLEN)) {
	    buf[cc] = '\0';
	    AllocNameSpace(buf,&DirEntryLinkName(tn,i));
	  }
	  if(lstat(buf,&lstats) >= 0) {
	    stats = lstats;
	    if((stats.st_mode & S_IFMT) == S_IFDIR)
	      DirEntryType(tn,i).dir = TRUE;
	    else DirEntryType(tn,i).filevar = TRUE;
	  }
	}
#endif /* S_IFLNK */
	else DirEntryType(tn,i).filevar = TRUE;
	AllocNameSpace(getname(this,stats.st_uid,getcell(this,fullEntryName)),
		       &DirEntryOwner(tn,i));
	DirEntryTimeStamp(tn,i) = stats.st_mtime;
	DirEntrySize(tn,i) = stats.st_size;
	DirEntryPerms(tn,i) = stats.st_mode;
	DirEntryNLinks(tn,i) = stats.st_nlink;
      }
      DirEntryParent(tn,i) = tn;
      if(anchor[i]) free(anchor[i]);
    }
  }
  if(anchor) free(anchor);
  DirMode(tn).do_rescan = FALSE;
  chdir(workingDir);
  OUT(bush_ScanDir);
  return(status);
}

void
bush::BuildSubDirs( register tree_type_node     tn )
    {
  class bush *self=this;
  register long		     i = 0, count = 0;
  tree_type_node	     newTreeNode = NULL;
  struct Dir_		    *newDir = NULL;
  char			     newDirPath[MAXPATHLEN];

  IN(bush_BuildSubDirs);
  if(tn && DirEntries(tn)) {
    count = DirEntriesCount(tn);
    while(i < count) {
      if(DirEntry(tn,i) && !(DirEntryMode(tn,i).destroyed) && 
	  DirEntryType(tn,i).dir) {
        newDir = (struct Dir_ *) calloc(1,sizeof(struct Dir_));
	sprintf(newDirPath,"%s%s%s",DIRPATH(tn),
		!strcmp(DIRPATH(tn),"/") ? "": "/",DirEntryName(tn,i));
	AllocNameSpace(newDirPath,&newDir->path);
	AllocNameSpace(DirEntryName(tn,i),&newDir->name);
	newTreeNode = (TREE)->CreateChildNode( DirEntryName(tn, i), (long) newDir, tn);
	DirMode(newTreeNode).do_rescan = TRUE;
	DirEntryInstance(tn, i) = newTreeNode;
	newDir->tn = newTreeNode;
      }
      i++;
    }
  }
  OUT(bush_BuildSubDirs);
}

void
bush::DestroyDirEntries( register tree_type_node   tn )
    {
  class bush *self=this;
  register long		   i = 0, count = 0;

  IN(bush_DestroyDirEntries);
  if(tn && DirEntries(tn)) {
    count = DirEntriesCount(tn);
    while(i < count) {
      if(DirEntry(tn,i)) {
	if(DirEntryLinkName(tn,i)) {
	  free(DirEntryLinkName(tn,i));
	  DirEntryLinkName(tn,i) = NULL;
	}
	if(DirEntryName(tn,i)) {
	  free(DirEntryName(tn,i));
	  DirEntryName(tn,i) = NULL;
	}
	if(DirEntryOwner(tn,i)) {
	  free(DirEntryOwner(tn,i));
	  DirEntryOwner(tn,i) = NULL;
	}
	if(DirEntry(tn,i)) {
	  free(DirEntry(tn,i));
	  DirEntry(tn,i) = NULL;
	}
      }
      i++;
    }
    if(DirEntryPtr(tn)) {
      free(DirEntryPtr(tn));
      DirEntryPtr(tn) = NULL;
    }
    if(DirEntries(tn)) {
      free(DirEntries(tn));
      DirEntries(tn) = NULL;
    }
  }
  OUT(bush_DestroyDirEntries);
}

void
bush::DestroyDirEntry( register tree_type_node   tn )
    {
    class bush *self=this;
    IN(bush_DestroyDirEntry);
    if(tn && (this)->Dir(tn)) {
	if(DirEntries(tn)) (this)->DestroyDirEntries(tn);
	if(DIRPATH(tn)) {
	    free(DIRPATH(tn));
	    DIRPATH(tn) = NULL;
	}
	if(DirName(tn)) {
	    free(DirName(tn));
	    DirName(tn) = NULL;
	}
	free((this)->Dir(tn));
	(TREE)->SetNodeDatum(tn,0);
    }
    OUT(bush_DestroyDirEntry);
}

void
bush::FreeSubDirs( register tree_type_node   tn )
    {
  class bush *self=this;
  register tree_type_node  tmp = NULL;
  register int		   level = 0;

  IN(bush_FreeSubDirs);
  if((tmp = tn) && ((level = (TREE)->NodeLevel(tmp)) > 0))
    while((tmp = (TREE)->NextNode(tmp)) && ((TREE)->NodeLevel(tmp) > level))
      (this)->DestroyDirEntry(tmp);
}

boolean
bush::ScanRequired( register tree_type_node   tn )
    {
  class bush *self=this;
  boolean	     status = FALSE;
  struct stat	     stats;

  IN(bush_ScanRequired);
  if(!DirMode(tn).destroyed &&
     (stat(DIRPATH(tn),&stats) ||
      DirTimeStamp(tn) != stats.st_mtime ||
      DirMode(tn).do_rescan))
	status = TRUE;
  OUT(bush_ScanRequired);
  return(status);
}

int
bush::DestroyEntry( register tree_type_node     tn, register struct Dir_Entry  *Entry )
      {
  class bush *self=this;
  char			     item[MAXPATHLEN*2];
  register long		     status = 0;

  sprintf(item,"%s/%s",DIRPATH(tn),Entry->name);
  if(Entry->type.dir) {
    static char	*argv[4] = {"rm","-rf",NULL,NULL};
    argv[2] = item;
    status = (this)->PerformSystemAction("/bin/rm",argv);
  }
  else status = unlink(item);
  if(!status) {
    Entry->mode.destroyed = TRUE;
    DirMode(tn).do_rescan = TRUE;
  }
  return(status);
}

int
bush::MoveEntry( register tree_type_node     tn, register struct Dir_Entry  *Entry, register char		     *newName )
        {
  class bush *self=this;
  char			     oldPath[MAXPATHLEN*2], newPath[MAXPATHLEN];
  register long		     status;

  sprintf(oldPath,"%s/%s",DIRPATH(tn),Entry->name);
  sprintf(newPath,"%s/%s",DIRPATH(tn),newName );
  if((status = rename(oldPath,newPath)) != -1 )
    AllocNameSpace(newName,&Entry->name);
  return(status);
}

int
bush::RenameDir( register tree_type_node     tn, register char		     *newPath , register char		     *newName )
      {
  class bush *self=this;
  register long		     status = ok, i = 0;
  register char		    *newFullName = NULL;

  IN(bush_RenameDir);
  newFullName = (char*)malloc(strlen(newPath)+strlen(newName)+2);
  sprintf(newFullName,"%s/%s",newPath,newName);
  if(status = rename(DIRPATH(tn),newFullName)) return(status);
  else {
    AllocNameSpace(newFullName,&DIRPATH(tn));
    AllocNameSpace(newName,&DirName(tn));
    for(i=0;i<DirEntriesCount(Parent(tn));i++)
      if(DirEntryInstance(Parent(tn),i)==tn) {
	AllocNameSpace(DirName(tn),&DirEntryName(Parent(tn),i));
	break;
      }
  }	
  OUT(bush_RenameDir);
  return(status);
}

long
bush::Read( register FILE		     *file, register long		      id )
      {
  class bush *self=this;
  char			     RootPathIfInset[MAXPATHLEN];
  long			     status = dataobject_NOREADERROR;

  IN(bush_Read);
  this->dataobject::id = (this)->UniqueID();
  fscanf(file,"%s",GivenDirName);
  im::GetDirectory(RootPathIfInset);
  (this)->InitTree(RootPathIfInset);
  (this)->BuildSubDirs((this)->TreeRoot());
  chdir((this)->DirPath((this)->TreeRoot()));
  fscanf(file,"%s",RootPathIfInset); /* to get past enddata token */
  OUT(bush_Read);
  return(status);
}

long
bush::Write( register FILE		 *file, register long		  id, register int		  level )
        {
  class bush *self=this;
  IN(bush_Write);
  if(this->writeID != id) {
    this->writeID = id;
    if(level) {
      fprintf(file,"\\begindata{%s,%d}\n",
	       (this)->GetTypeName(),
	       (this)->UniqueID());
      fprintf(file,"%s",DIRPATH((this)->TreeRoot()));
      fprintf(file,"\n\\enddata{%s,%d}\n",
	       (this)->GetTypeName(),
	       (this)->UniqueID());
    }
    else {
      fprintf(file,"\\begindata{%s,%d}\n",
	       (this)->GetTypeName(),
	       (this)->UniqueID());
      fprintf(file,"\n%s\n",DIRPATH((this)->TreeRoot()));
      fprintf(file,"\n\\enddata{%s,%d}\n",
	       (this)->GetTypeName(),
	       (this)->UniqueID());
    }
  }
  OUT(bush_Write);
  return((long)this);
}

char *
bush::ViewName( )
  {
  return("bushv");
}

int
bush::PerformSystemAction( register char		     *name, register char		     **argv )
      {
    int			     pid = 0;
    WAIT_STATUS_TYPE status;

  if(environ::GetProfileSwitch("SecurityConscious", FALSE)) {
      fprintf(stderr, "SecurityConsciousness does not allow bush to run programs.\n");
      return(-1);
  }
  if((pid = osi_vfork()) == 0) {
    register int	fd;

    NEWPGRP();
    fd = open("/dev/null",O_WRONLY,0644);
    if(fd >= 0) dup2(fd,1);
    close(fd);
    execvp(name,argv);
    /* flow should never reach here, but just in case.... */
    return(-1);
  }	
  while(pid != wait(&status));
  return(WAIT_EXIT_STATUS(status));
}