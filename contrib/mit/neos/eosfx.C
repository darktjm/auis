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

/*
 * eosfx.c
 *
 * Glue between EOS applications and fx library.
*/

/* ************************************************************
 *  Copyright (C) 1989, 1990, 1991
 *  by the Massachusetts Institute of Technology
 *   For full copyright information see:'mit-copyright.h'     *
 ************************************************************ */

#include <andrewos.h>
#include <mit-copyright.h>

#include <stdio.h>
#include <eos_structs.h>
#include <eosfx.H>

/* This is a simple interface to utility routines in the fx library.  The call is here so other dynamic objects need not link against fx library.  The code itself is not duplicated in case the library changes. */


ATKdefineRegistry(eosfx, ATK, NULL);
static char *save_string(char  *s);


void eosfx::PaperClear(Paper  *p)
{
    paper_clear(p);
}

void eosfx::PaperCopy(Paper  *src , Paper  *dest)
{
    paper_copy(src, dest);
}



struct paperPositions *eosfx::LocatePaper(struct paperPositions  *list, long  x, Paper  *paper)
/*
  Finds the paper identified by a position within the textobj
  x is the position of the cursor within the text at the moment.
  0 <= x <= text_GetLength(self->textobj)
  the function searches through paperPositions->textpos to find out which
  paper is being referred to, then places the description of that
  paper into paper and returns a pointer to the paperPosition structure.
  */
{
    static struct paperPositions *node;

    if (list == NULL) {
	return NULL;
    }

    for (node = list; x < node->textpos && node->next != NULL; node = node->next);
    if (paper != NULL) paper_copy(&(node->paper->p), paper);
    return node;
}

void eosfx::AddPaperText(struct paperPositions  **list, Paperlist  paper, long  x, long  len)
/*
  Adds a description of this paper and its text position into the linked list. 
  The paper must be created in order! i.e. the x parameter of Addpaper should
  always be greater than the previous value:
  Precondition: (x >= self->Positions->textpos) || (self->Positions == NULL)
  */
{
    struct paperPositions *newpos = (struct paperPositions *) malloc(sizeof(struct paperPositions));

    newpos->textpos = x;
    newpos->textlength = len;
    newpos->paper   = paper;
    newpos->flags = 0;
    newpos->environment = NULL;
    newpos->next    = *list;
    *list = newpos;
}

void eosfx::DestroyPositions(struct paperPositions  **list)
/*
  Goes thru the linked list, freeing memory.
  The list is set to NULL when finished
  */
{
    struct paperPositions *temp;

    if (*list == NULL) return;

    while ((*list)->next != NULL) {
	temp = (*list)->next;
	free(*list);
	*list = temp;
    }
    free(*list);
    (*list) = NULL;
}

char *eosfx::OpenCourse(char  *course, FX  **fxp)
/* If any mishap occurs, a message is returned in static storage */
/* An fxp will be returned if reads are possible to the course */
/* If, because of server problems, you can read but not write, */
/* Then an error message will return, with a *VALID* fxp - so you can continue */
/* If neither reads nor writes are possible, an error message will return */
/* and fxp will be set to NULL */
{
    long code;
    static char errstring[128];

    *fxp = fx_open(course, &code);
    /* (fxp == NULL && code is error) or (fxp is valid for reads&writes) */
    /* or (fxp is valid for reads, invalid for writes && code is warning) */
    if (code) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while opening the course");
	return errstring;
    }
    return NULL;
}

void eosfx::Close(FX  **fxp)
{
    if (fxp && *fxp) {
	fx_close(*fxp);
	*fxp = NULL;
    }
}

char *eosfx::SendFile(char  *course, char  *filename, Paper  *paper, boolean  delete_c)
{
    FX *fxp;
    long code;
    static char errstring[128];
    char *t;

    if (t = eosfx::OpenCourse(course, &fxp)) {
	/* Error opening course - we can't continue, so abort operation */
	if (delete_c) unlink(filename);
	return t;
    }

    if (code = fx_send_file(fxp, paper, filename)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while sending the file");
	if (delete_c) unlink(filename);
	fx_close(fxp);
	return errstring;
    }

    if (delete_c) unlink(filename);
    fx_close(fxp);
    return NULL;
}

char *eosfx::RetrieveFile(FX  *fxp, Paper  *paper, char  *filename)
{
    long code;
    static char errstring[128];

    if (code = fx_retrieve_file(fxp, paper, filename)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while receiving ");
	strcat(errstring, paper->filename);
	unlink(filename);
	return errstring;
    }
    return NULL;
}

char *eosfx::Retrieve(FX  *fxp, Paper  *paper, FILE  *fp)
{
    long code;
    static char errstring[128];

    if (code = fx_retrieve(fxp, paper, fp)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while receiving ");
	strcat(errstring, paper->filename);
	return errstring;
    }
    return NULL;
}


char *eosfx::Move(FX  *fxp, Paper  *src , Paper  *dest)
{
    long code;
    static char errstring[128];

    if (code = fx_move(fxp, src, dest)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while changing status of paper to Taken");
	return errstring;
    }
    return NULL;
}

char *eosfx::Delete(FX  *fxp, Paper  *paper)
{
    long code;
    static char errstring[128];

    if (code = fx_delete(fxp, paper)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while deleting the paper");
	return errstring;
    }
    return NULL;
}

void eosfx::ListDestroy(Paperlist_res  **plist)
{
    fx_list_destroy(plist);
}

char *eosfx::List(FX  *fxp, Paper  *p, Paperlist_res  **ret)
{
    long code;
    static char errstring[128];

    if (code = fx_list(fxp, p, ret)) {
	strcpy(errstring, error_message(code));
	strcat(errstring, " while getting the list of papers\n");
	return errstring;
    }
    return NULL;
}


char *
eosfx::AclList(FX  *fxp, char  *name, stringlist_res  **list)
{
  long code;
  static char errstring[128];

  if (code = fx_acl_list(fxp, name, list)) {
    strcpy(errstring, error_message(code));
    strcat(errstring, " while reading access list\n");
    return errstring;
  }
  return NULL;
}

void
eosfx::AclDestroy(stringlist_res  **list)
{
  fx_acl_list_destroy(list);
}


char *
eosfx::AclAdd(FX  *fxp, char  *aclname , char  *name)
{
  long code;
  static char errstring[128];

  if (code = fx_acl_add(fxp, aclname, name)) {
    sprintf(errstring, "%s while adding %s into list\n",
	    error_message(code), name);
    return errstring;
  }
  return NULL;
}

char *
eosfx::AclDel(FX  *fxp, char  *aclname , char  *name)
{
  long code;
  static char errstring[128];

  if (code = fx_acl_del(fxp, aclname, name)) {
    sprintf(errstring, "%s while deleting %s from list\n",
	    error_message(code), name);
    return errstring;
  }
  return NULL;
}

char *
eosfx::Directory(FX  *fxp, stringlist_res  **list)
{
  long code;
  static char errstring[128];

  if (code = fx_directory(fxp, list)) {
    strcpy(errstring, error_message(code));
    strcat(errstring, " while reading list of courses\n");
    return errstring;
  }
  return NULL;
}



#define New_Array(obj, count) (obj *) calloc((count), sizeof(obj))

/* Code originally written by Rob Shaw 
  * for previous version of eos
      * Hacked about a lot by Nick Williams
      * for new, improved version! 
	  */

char *eosfx::LocalUnique(char  *template_c)
{
    FILE *fd;
    static char *result;
    static char *nametry;
    char *oldsuffix, *tmpname;
    char *t;
    char *index(), *rindex();
    int i;

    result = New_Array(char, strlen(template_c) + 10);
    nametry = New_Array(char, strlen(template_c) + 10);
    strcpy(result, template_c);

    /* Change all 'orrible characters to underscore */
    for (i = 0; result[i] != '\0'; i++)
	if (index(" *\t&;$^!()[]?<>`\'\"", result[i]) != NULL)
	    result[i] = '_';

    /* See if name is already unique */
    if ((fd = fopen(result,"r")) == NULL) {
	/* fopen failed, so we got a good 'un */
	return(result);
    }
    fclose(fd);

    /* Split name into result.oldsuffix */
    oldsuffix = (char *) malloc(strlen(result)+1);
    t = rindex(result, '.');
    if (t)
	strcpy(oldsuffix, t);
    else
	strcpy(oldsuffix, "");

    if (t) {
	i = strlen(result) - strlen(oldsuffix);
	for (tmpname = result; i>0 ; i--)
	    tmpname++;
	tmpname[i] = '\0';
    }

    /* Okay, Let's try throwing numbers on the end, until we find a non-existing
	 one... */
    for (i=1; i < 1000; i++) {
	sprintf(nametry, "%s_%d%s\0", result, i, oldsuffix);
	if ((fd = fopen(nametry,"r")) == NULL) {
	    break;
	} 
	fclose(fd);
	/* Of course, there is now a race condition between fclose(fd) and
	 * the file actually being used...
	 */
    }
    return(nametry);
}

char *eosfx::PathTail(char  *path)
/* Returns the last element of a pathname */
{
    char *rindex();
    static char *result;

    if ((result = rindex(path, '/')) == NULL)
	return path;

    return ++result;
}

static char *save_string(char  *s)
{
    char *ret = malloc(strlen(s)+1);
    (void) strcpy(ret, s);
    return ret;
}

char *eosfx::SaveString(char  *s)
{
    return save_string(s);
}

void eosfx::PaperCopyContents(Paper  *src , Paper  *dest)
{
    *dest = *src;
    dest->author = save_string(dest->author);
    dest->owner = save_string(dest->owner);
    dest->desc = save_string(dest->desc);
    dest->filename = save_string(dest->filename);
    dest->location.host = save_string(dest->location.host);
}

void eosfx::PaperFreeContents(Paper  *p)
{
    free(p->author);
    free(p->owner);
    free(p->desc);
    free(p->filename);
    free(p->location.host);
    free(p);
}
