/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* typetext.c
 * Code for typescript to ignore styles and dataobjects in read text.
 *
 */

#define GETANDTEST(C,file) ((C = getc(file)) != EOF && C != '\0')
#define TESTEND(C) (C == EOF || C == '\0')


#include <andrewos.h>
ATK_IMPL("typetext.H")
#include <typetext.H>
#include <style.H>
#include <stylesheet.H>


ATKdefineRegistryNoInit(typetext, text);

long typetext::HandleKeyWord(long  pos, char  *keyword, FILE  *file  )
{
    long c;
    if ((strcmp(keyword, "textdsversion") == 0) || 
	(strcmp(keyword, "define") == 0) ||
	(strcmp(keyword, "template") == 0) ){
   	 	while (GETANDTEST(c,file) && c != '}') ;
    		if(TESTEND(c)) return 0;
    		while (GETANDTEST(c,file) && c != '\n');
	}
    if(strcmp(keyword, "view") == 0)
	return (this)->text::HandleKeyWord( pos, keyword, file);
    return 0;
}

long typetext::HandleCloseBrace(long  pos, FILE  *file  )
{
    return 0;
}

long typetext::HandleBegindata(long  pos,FILE  *file)
{
return (this)->text::HandleBegindata(pos,file);
}

const char *typetext::ViewName()
    {
    return "typescript";
}

typetext::typetext()
        {
    this->hashandler = FALSE;
    (this)->SetCopyAsText(TRUE);
    (this)->SetWriteAsText(TRUE);
    (this)->SetObjectInsertionFlag(FALSE);
    (this)->ReadTemplate( "typescript", FALSE);
    THROWONFAILURE( TRUE);
}

long typetext::GetModified()
    {
    return 0;
}

