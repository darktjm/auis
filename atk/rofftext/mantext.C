/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* mantext: link to rofftext -man. */



#include <andrewos.h>
ATK_IMPL("mantext.H")
#include <mark.H>
#include <rofftext.H>

#include <mantext.H>

#define INITIAL 100
#define INCREMENT 1000


ATKdefineRegistryNoInit(mantext, rofftext);


mantext::mantext()
{
    this->nLines = 0;
    this->nAlloc = INITIAL;
    this->filename[0] = '\0';
    this->lineMark = (class mark **)malloc(this->nAlloc*sizeof(class mark *));
    if (this->lineMark == NULL) THROWONFAILURE( FALSE);
    (this)->SetLinePos( 0L, 0L);
    THROWONFAILURE( TRUE);
}

long mantext::Read(FILE  *file, long  id )
{
    long tmpRetValue;
    class rofftext *r = (class rofftext *)this;
    char **t;

    /* copy the filename that was put into rofftext via SetAttributes */
    if (r->filename != NULL) strcpy(this->filename, r->filename);
    r->inputfiles = t = (char **)malloc(2 * sizeof(char *));
    t[0] = NULL;
    t[1] = NULL;
    r->filename = NULL;

    r->macrofile = TMACMANFILE;
    r->RoffType = FALSE;
    r->HelpMode = FALSE;

    tmpRetValue = (r)->rofftext::Read( file, (long)r); 

    return tmpRetValue;

}

long mantext::GetLinePos(long  line )
{
    line -= 2;
    if (line > this->nLines) line = this->nLines;
    if (line < 0) line = 0;
    return (this->lineMark[line])->GetPos();
}

void mantext::SetLinePos(long  line, long  pos )
{
    if (line < 0) return;
    if (line >= this->nAlloc) {
	this->nAlloc += INCREMENT;
	this->lineMark = (class mark **)realloc(this->lineMark, this->nAlloc*sizeof(class mark *));
    }
    if (this->lineMark == NULL) return;
    this->lineMark[line] = (this)->CreateMark( pos, 0);
    this->nLines = line;
    /*  must make sure we have not skipped some */
}


void mantext::GetFilename(char  *filename )
{
    strcpy(filename, this->filename);
}
