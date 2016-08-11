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




/*  asmtextv.ch: Text subclass specialized for dealing with Assembly code. */

class asmtextview[asmtextv]: srctextview[srctextv] {

  overrides:
    EndComment(char key);
    PostMenus(struct menulist *menulist);
    PrependKeyState() returns struct keystate *;
    Reindent();
    SetDataObject(struct asmtext *dataobj);
    StartComment(char key);
    StartLineComment(char key);

  classprocedures:
    InitializeClass() returns boolean;
    InitializeObject(struct asmtextview *self) returns boolean;
    FinalizeObject(struct asmtextview *self);

  data:
    struct keystate *asm_state;
    struct menulist *asm_menus;    
};
