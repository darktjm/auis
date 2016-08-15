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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/menulist.C,v 3.12 1995/07/14 18:52:45 rr2b Stab74 $";
#endif


 

/* menulist.c
 * Provides an abstraction for cooperative menu usage among views.
 */
ATK_IMPL("menulist.H")

#include <menulist.H>
#include <proctable.H>
#include <environ.H>
#define class_StaticEntriesOnly
#include <view.H>
#undef class_StaticEntriesOnly
#include <messitem.H>

#define MLITEM_UNDEF   0
#define MLITEM_DELETE  1
#define MLITEM_REPLACE 2

#define MPITEM_UNDEF   0
#define MPITEM_NOLOAD  1
#define MPITEM_LOAD    2

struct mlitem {
  char *oldname, *newname;
  short state;
  struct mlitem *next;
};

struct mpitem {
  char *name;
  short load;
  struct mlitem *ml;
  struct mpitem *next;
};


ATKdefineRegistry(menulist, ATK, NULL);


static void 
ExplodeMenuString(char  *str, char  *paneStr, long  paneStrLen, long  *panePriority, 
		  char  *selectionStr, long  selectionStrLen, long  *selectionPriority)
{
    char *p;
    char *pLimit;
    boolean inSelection = FALSE;
    boolean inPriority = FALSE;
    long priority = 0;

    p = paneStr;
    pLimit = paneStr + paneStrLen;
    for ( ; ; str++) switch (*str)  {
	case '\0':
	case ',':
	    *p = '\0';
	    if (inSelection)  {
		if (selectionPriority != NULL)
		    *selectionPriority = priority;
		return;
	    }
	    else if (*str == '\0')  {
		strncpy(selectionStr, paneStr, selectionStrLen);
		selectionStr[selectionStrLen - 1] = '\0';
		paneStr[0] = '\0';
		if (selectionPriority != NULL)
		    *selectionPriority = priority;

	/* Pane priority of -1 is special to cmenu_AddPane. 
	   It essentially means put this pane at the end of the list... 
	 In the InstallMenus code we special case the empty 
	title pane to give it a priority of 0. Life sucks... */
		if (panePriority != NULL)
		    *panePriority = -1;
		return;
	    }
	    if (panePriority != NULL) 
		*panePriority =  (priority == 0) ? -1 : priority;
	    p = selectionStr;
	    pLimit = selectionStr + selectionStrLen;
	    inSelection = TRUE;
	    inPriority = FALSE;
	    priority = 0;
	    break;
	case '~':
	    inPriority = TRUE;
	    break;
	default:
	    if (inPriority)
		priority = priority * 10 + *str - '0';
	    else
		if (p < pLimit)  *p++ = *str;
	    break;
    }
}
    
static struct mpitem *currentmp = NULL; 
static struct mpitem *firstmp = NULL; 

static struct mlitem *mlitem_New(const char  *s1 , const char  *s2)
{
  struct mlitem *ml;
  ml = (struct mlitem *) malloc(sizeof(struct mlitem));
  if (s1 && *s1) {
    ml->oldname = (char *) malloc(strlen(s1) + 1);
    strcpy(ml->oldname,s1);
  } else 
    ml->oldname = NULL;
  if (s2 && *s2) {
    ml->newname = (char *) malloc(strlen(s2) + 1);
    strcpy(ml->newname,s2);
  } else 
    ml->newname = NULL;
  ml->state = MLITEM_UNDEF;
  ml->next = NULL;
  return ml;
}

static  struct mlitem *mlitem_Exists(struct mpitem  *mp, const char  *str)
{
    struct mlitem *tmpml = mp->ml;
    while(tmpml) {
	if(tmpml->oldname && str) {
	    if(strcmp(tmpml->oldname, str)==0) return tmpml;
	}
	tmpml=tmpml->next;
    }
    return tmpml;
}

static void mlitem_Add(struct mpitem  *mp, struct mlitem  *ml)
{
  struct mlitem *tmpml = mp->ml;
  if (mp == NULL || ml == NULL)
    return;
  if (mp->ml == NULL)
    mp->ml = ml;
  else { 
    while (tmpml->next)
      tmpml = tmpml->next;
    tmpml->next = ml;
  }
}

static struct mpitem *mpitem_New(char  *str)
{
  struct mpitem *mp;
  mp = (struct mpitem *) malloc(sizeof(struct mpitem));
  mp->name = (char *) malloc(strlen(str) + 1);
  strcpy(mp->name,str);
  mp->next = NULL;
  mp->ml = NULL;
  mp->load = MPITEM_LOAD;
  if (currentmp == NULL) firstmp=mp; else currentmp->next = mp;
  currentmp = mp;
  return mp;
}

static  struct mpitem *mpitem_Exists(const char  *str)
{
  struct mpitem *tmp = firstmp;

  if (str == NULL || *str == '\0') return NULL;
  while (tmp && strcmp(tmp->name,str) != 0)
    tmp = tmp->next;
  return tmp;
}

static short mpitem_DoResolv(const char  *str)
{
  struct mpitem *tmp = NULL;
  if ((tmp=mpitem_Exists(str)) != NULL)
    return tmp->load;
  return MPITEM_UNDEF;
}

#define INITIALSIZE 512

/* Hacked routine to rea a "whole file" into memory. */
static char *MapMenuFile(const char  *filename, long  *fileLength /* OUT */)
        {

    int fd;
    char *buffer;
    long length = 0;

    if ((fd = open(filename, O_RDONLY, 0)) >= 0) {

        struct stat statBuf;

        if (fstat(fd, &statBuf) >= 0) {

            long bufferSize; /* Current size of malloced block. */
            long bytesRead = 0;

            /* Find the size. In the case of special files, use a suitable default. */
            if ((statBuf.st_mode & S_IFMT) == S_IFREG)
                bufferSize = statBuf.st_size ;
            else
                bufferSize = INITIALSIZE;

            buffer = (char *) malloc(bufferSize + 1); /* +1 for NUL at end. */

            while (buffer != NULL && (bytesRead = read(fd, buffer + length, bufferSize - length )) > 0) {
                length += bytesRead;
                if (length >= bufferSize) {
                    bufferSize *= 2;
                    buffer = (char *) realloc(buffer, bufferSize + 1); /* +1 for NUL at end. */
                }
            }
            if (bytesRead < 0) {
                free(buffer);
                buffer = NULL;
            }
            else
                buffer[length] = '\0';
        }
        else
            buffer = NULL;

        close(fd);
    }
    else
        buffer = NULL;

    if (fileLength != NULL)
        *fileLength = length;
    return buffer;
}

#define UnmapMenuFile(mappedMemory) free(mappedMemory)

static int ReadMenuFile(const char  *filename, boolean  executeImmediately)
{

    char *buffer;
    long length;

    if ((buffer = MapMenuFile(filename, &length)) != NULL) {

        char *p;
        int currentLine;

        currentLine = 0;

        p = buffer;
        while (p < buffer + length) {
	    char str[128];
	    char strvalue[128];
	    int i = 0;
            ++currentLine;
	    /* Skip to the end of line. */
	    while (*p != '\n' && *p != '\0' && *p != ' ')
		str[i++] = *p++;
	    while (*p == ' ') p++;
	    
	    str[i] = '\0';
	    if (str[0] != '\0' && *p != '\n' && *p != '\0') {
	      struct mpitem *mp;
	      struct mlitem *ml;	
	      char s1[128], s2[128];
	      char strcmd[128];
	      i = 0;
	      while (*p != '\n' && *p != '\0' && *p != ' ')
		strcmd[i++] = *p++;
	      strcmd[i] = '\0';
	      if ((mp=mpitem_Exists(str)) == NULL)
		mp = mpitem_New(str);
	      if (strcmp(strcmd,"deleteallmenus") == 0)
		mp->load = MPITEM_NOLOAD;
	      else if (strcmp(strcmd,"deletemenu") == 0 || 
		       strcmp(strcmd,"replacemenu") == 0) {
		while (*p == ' ') p++;
		i = 0;
		if (*p == '"') p++;
		while (*p != '\n' && *p != '\0' && *p != '"')
		  s1[i++] = *p++;
		s1[i] = '\0';
		if (*p == '"') p++;
		if (strcmp(strcmd,"replacemenu") == 0) {
		  i = 0;
		  while (*p == ' ') p++;
		  if (*p == '"') p++;
		  while (*p != '\n' && *p != '\0' && *p != '"')
		    s2[i++] = *p++;
		  s2[i] = '\0';
		  if (*p == '"') p++;
		}
		if ((ml=mlitem_Exists(mp,s1)) == NULL) {
		  ml = mlitem_New(s1,s2);
		  mlitem_Add(mp,ml);
		}
		if (strcmp(strcmd,"replacemenu") == 0)
		  ml->state = MLITEM_REPLACE;
		else
		  ml->state = MLITEM_DELETE;
		mp->load = MPITEM_LOAD;
	      }
	    }
	    while (*p != '\n' && *p != '\0') /*end of line */
	      p++;
	    if (*p == '\n') p++;
	  }

        UnmapMenuFile(buffer);

        return 0;
    }
    else
        return -1;
}    


void InitMenuFile()
{
    const char *al=environ::Get("ANDREWLANGUAGE");
    const char *alf=environ::Get("ANDREWLANGUAGEMENUFILE");
    if(alf==NULL) alf=environ::GetProfile("AndrewLanguageMenuFile");
    if(al==NULL) al=environ::GetProfile("AndrewLanguage");
    if(alf==NULL && al) {
	char MenuFile[MAXPATHLEN];
	strcpy(MenuFile, environ::AndrewDir("/lib/"));
	strcat(MenuFile, al);
	strcat(MenuFile, ".menu");
	ReadMenuFile(MenuFile, TRUE);
    } else if(alf) ReadMenuFile(alf, TRUE);
}

static long nextMLVersion = 0;
static boolean initedmenufile=FALSE;
static boolean translatemenus=TRUE;

menulist::menulist()
{
    if(!initedmenufile) {
	InitMenuFile();
	initedmenufile=TRUE;
	translatemenus=environ::GetProfileSwitch("TranslateMenus", TRUE);
    }
    this->regionID = -1;
    this->curIM = NULL;
    this->version = 0;
    this->menuVersion = 0;
    this->installVersion = -1;
    this->object = NULL;
    this->menus = NULL;
    this->refcount = (int *) malloc(sizeof(int));
    *this->refcount = 1;
    this->curMenu = NULL;
    this->menuChainBefore = NULL;
    this->menuChainAfter = NULL;
    this->curChainBefore = NULL;
    this->curChainAfter = NULL;
    this->selectMask=0;
    this->oldMask=0;
    THROWONFAILURE( TRUE);
}

menulist::~menulist()
        {

    if (*this->refcount == 1) {
	(this)->ClearML();
	free(this->refcount);
    }
    else if (*this->refcount > 1) {
	*this->refcount -= 1;
    }
    (this)->ClearChain();
}

void menulist::SetView(class view  *view)
        {

    this->object = (ATK  *) view;
}

class menulist *menulist::Create(class view  *view)
        {

    class menulist *thisMenu;

    thisMenu = new menulist;
    (thisMenu)->SetView( view);
    return thisMenu;
}

class menulist *menulist::DuplicateML(class view  *view)
        {

    class menulist *newMenus;

    newMenus = menulist::Create(view);
    free(newMenus->refcount);
    *this->refcount += 1;
    newMenus->refcount = this->refcount;
    newMenus->menus = this->menus;
    return newMenus;
}

/* Copy a menu list's menu items. Used to implement copy-on-write for item lists.
 * This routine should only be called when *menulist->refcount > 1
 */
static void copyItems(class menulist  *menulist)
{
    struct itemlist *traverse = menulist->menus;

    menulist->menus = NULL;
    for (; traverse != NULL; traverse = traverse->next) {
        struct itemlist *thisItem;

        thisItem = (struct itemlist *) malloc(sizeof(struct itemlist));
        thisItem->string = (char *) malloc(strlen(traverse->string) + 1);
        strcpy(thisItem->string, traverse->string);
        thisItem->proc = traverse->proc;
        thisItem->functionData = traverse->functionData;
	thisItem->enableMask = traverse->enableMask;
        thisItem->next = menulist->menus;

        menulist->menus = thisItem;
    }
    *menulist->refcount -= 1;
    menulist->refcount = (int *) malloc(sizeof(int));
    *menulist->refcount = 1;
}

static void SetPrio(int  p, char  *sp)
{
    if(p<1) sp[0]='\0';
    else sprintf(sp, "~%d", p);
}
    
void menulist::AddToML(char  *string, struct proctable_Entry  *menuProc, long  functionData /* Actually any 32 bit crufty... */, long  mask)
                    {

    struct itemlist *thisItem;
    boolean link = FALSE;
    int doResolvMenu=MPITEM_UNDEF;
    struct mpitem *mp=NULL;
    struct mlitem *ml=NULL;
    char transbuf[1024];
    char cname[1024];
    char iname[1024];
    long cprio=0;
    long iprio=0;
    char scprio[16];
    char siprio[16];
    
    if (string == NULL)
        return;

    if(menuProc && menuProc->type) {
	doResolvMenu=mpitem_DoResolv((menuProc->type)->GetTypeName());
	if(doResolvMenu==MPITEM_NOLOAD) return;
	mp=mpitem_Exists((menuProc->type)->GetTypeName());
	if(mp) {
	    ml=mlitem_Exists(mp, string);
	    if(ml && ml->state==MLITEM_REPLACE) {
		cname[0]=iname[0]='\0';
		ExplodeMenuString(string, cname, sizeof(cname), &cprio, iname, sizeof(iname), &iprio);
		cname[0]=iname[0]='\0';
		ExplodeMenuString(ml->newname, cname, sizeof(cname), NULL, iname, sizeof(iname), NULL);
		SetPrio(cprio, scprio);
		SetPrio(iprio, siprio);
		sprintf(transbuf, "%s%s%s%s%s", cname, scprio, (cname[0]=='\0' && cprio < 1)?"":",", iname), siprio;
		string=transbuf;
	    }
	}
    }
    if(translatemenus) {
	cname[0]=iname[0]='\0';
	ExplodeMenuString(string, cname, sizeof(cname), &cprio, iname, sizeof(iname), &iprio);
	SetPrio(cprio, scprio);
	SetPrio(iprio, siprio);
	sprintf(transbuf, "%s%s%s%s%s", messitem::Replace(cname), scprio,(cname[0]=='\0' && cprio < 1)?"":",", messitem::Replace(iname), siprio);
	string=transbuf;
    }
	
    
    if (*this->refcount > 1)
        copyItems(this);

    for (thisItem = this->menus; thisItem != NULL && (strcmp(thisItem->string, string) != 0); thisItem = thisItem->next);

    if (thisItem == NULL) {
	char *p;
        thisItem = (struct itemlist *) malloc(sizeof(struct itemlist));
        thisItem->string = (char *) malloc(strlen(string) + 1);
        strcpy(thisItem->string, string);
        link = TRUE;
    }
    thisItem->proc = menuProc;
    thisItem->functionData = functionData;
    thisItem->enableMask=mask;
    if (link) { /* Only link it in after the data is valid. */
        thisItem->next = this->menus;
        this->menus = thisItem;
    }
    this->version = this->menuVersion = nextMLVersion;
}

void menulist::DeleteFromML(char  *string)
        {

    struct itemlist *traverse, **previous = &(this->menus);

    if (string == NULL)
        return;

    if (*this->refcount > 1)
        copyItems(this);

    for (traverse = this->menus; traverse != NULL && (strcmp(traverse->string, string) != 0); traverse = traverse->next) {
        previous = &(traverse->next);
    }

    if (traverse != NULL) {
        *previous = traverse->next;
        free(traverse->string);
        free(traverse);
	this->version = this->menuVersion = nextMLVersion;
    }
}

boolean menulist::SetMask(long  mask)
{
    if(mask!=this->selectMask){
	this->selectMask=mask;
	return TRUE;
    }else
	return FALSE;
}

void menulist::ClearML()
    {

    struct itemlist *traverse, *next;

    if (*this->refcount == 1)  {
        for (traverse = this->menus; traverse != NULL; traverse = next) {
            free(traverse->string);
            next = traverse->next;
            free(traverse);
        }
    }
    else {
        if (*this->refcount < 1)
            fprintf(stderr, "menulist: internal error, refcount < 1 in ClearML\n");
        else {
            *this->refcount -= 1;
            this->refcount = (int *) malloc(sizeof(int));
            *this->refcount = 1;
        }
    }
    this->menus = NULL;
    this->version = this->menuVersion = nextMLVersion;
}

boolean menulist::NextME(char  **outString, long  *outData, struct proctable_Entry  **outProc)
                {

    if (this->curMenu != NULL) {
        *outString = this->curMenu->string;
        *outData = this->curMenu->functionData;
        *outProc = this->curMenu->proc;
        this->curMenu = this->curMenu->next;
        return TRUE;
    }
    else
        return FALSE;
}

class menulist *menulist::NextBeforeMC()
    {

    register class menulist *value;

    if (this->curChainBefore != NULL) {
        value = this->curChainBefore->menulist;
        this->curChainBefore = this->curChainBefore->next;
        return value;
    }
    return NULL;
}

class menulist *menulist::NextAfterMC()
    {

    register class menulist *value;

    if (this->curChainAfter != NULL) {
        value = this->curChainAfter->menulist;
        this->curChainAfter = this->curChainAfter->next;
        return value;
    }
    return NULL;
}

void menulist::ChainBeforeML(class menulist  *chainee, long  key)
            {

    struct headerlist *tempHeader, *next, **previous;

    if (chainee == NULL) /* Need to handle posting of NULL since it is used to clear menus. */
        return;

    previous = &(this->menuChainBefore);
    for (tempHeader = this->menuChainBefore; tempHeader != NULL; tempHeader = next) {
        if (tempHeader->menulist == chainee) {
            if (tempHeader->assocKey != key)
                tempHeader->assocKey = key;
            return;
        }
        else if (tempHeader->assocKey == key) {
            *previous = next = tempHeader->next;
            free(tempHeader);
            continue;
        }
        next = tempHeader->next;
    }
    
    previous = &(this->menuChainAfter);
    for (tempHeader = this->menuChainAfter; tempHeader != NULL; tempHeader = next) {
        if (tempHeader->menulist == chainee || tempHeader->assocKey == key) {
            *previous = next = tempHeader->next;
            free(tempHeader);
            continue;
        }
        next = tempHeader->next;
    }

    tempHeader = (struct headerlist *) malloc(sizeof(struct headerlist));
    tempHeader->menulist = chainee;
    tempHeader->next = this->menuChainBefore;
    tempHeader->assocKey = key;
    this->menuChainBefore = tempHeader;
    this->version = nextMLVersion;
}

void menulist::ChainAfterML(class menulist  *chainee, long  key)
            {

    struct headerlist *tempHeader, *next, **previous;

    if (chainee == NULL) /* Need to handle posting of NULL since it is used to clear menus. */
        return;

    previous = &(this->menuChainBefore);
    for (tempHeader = this->menuChainBefore; tempHeader != NULL; tempHeader = next) {
        if (tempHeader->menulist == chainee || tempHeader->assocKey == key) {
            *previous = next = tempHeader->next;
            free(tempHeader);
            continue;
        }
        next = tempHeader->next;
    }
   
    previous = &(this->menuChainAfter);
    for (tempHeader = this->menuChainAfter; tempHeader != NULL; tempHeader = next) {
        if (tempHeader->menulist == chainee) {
            if (tempHeader->assocKey != key)
                tempHeader->assocKey = key;
            return;
        }
        else if (tempHeader->assocKey == key) {
            *previous = next = tempHeader->next;
            free(tempHeader);
            continue;
        }
        next = tempHeader->next;
    }

    tempHeader = (struct headerlist *) malloc(sizeof(struct headerlist));
    tempHeader->menulist = chainee;
    tempHeader->next = this->menuChainAfter;
    tempHeader->assocKey = key;
    this->menuChainAfter = tempHeader;
    this->version = nextMLVersion;
}

void menulist::UnchainML(long  key)
        {

    struct headerlist *traverse, **previous;

    previous = &(this->menuChainBefore);
    for (traverse = this->menuChainBefore; traverse != NULL && (traverse->assocKey != key); traverse = traverse->next)
        previous = &(traverse->next);

    if (traverse == NULL) {
        previous = &(this->menuChainAfter);
        for (traverse = this->menuChainAfter; traverse != NULL && (traverse->assocKey != key); traverse = traverse->next)
            previous = &(traverse->next);
    }

    if (traverse != NULL) {
        *previous = traverse->next;
        free(traverse);
        this->version = nextMLVersion;
    }
}

class menulist *menulist::GetChainedML(long  key)
        {

    struct headerlist *traverse;

    for (traverse = this->menuChainBefore; traverse != NULL; traverse = traverse->next)
        if (traverse->assocKey != key)
            return traverse->menulist;
    for (traverse = this->menuChainAfter; traverse != NULL; traverse = traverse->next)
        if (traverse->assocKey != key)
            return traverse->menulist;
    return NULL;
}

void menulist::ClearChain()
    {

    boolean didSomething = FALSE;
    struct headerlist *traverse, *next;

    for (traverse = this->menuChainBefore; traverse != NULL; traverse = next) {
        next = traverse->next;
        free(traverse);
        didSomething = TRUE;
    }
    this->menuChainBefore = NULL;
    for (traverse = this->menuChainAfter; traverse != NULL; traverse = next) {
        next = traverse->next;
        free(traverse);
        didSomething = TRUE;
    }
    this->menuChainAfter = NULL;

    if (didSomething)
        this->version = nextMLVersion;
}

int menulist::NextMLVersion()
    {
    return nextMLVersion;
}

void menulist::IncrementMLVersion()
    {
    ++nextMLVersion;
}
