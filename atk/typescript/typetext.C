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


ATKdefineRegistry(typetext, text, NULL);

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

