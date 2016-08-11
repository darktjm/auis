#ifndef _srctextview_H_
#define _srctextview_H_
/* File srctextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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
 
   srctextview: a textview subclass specialized for dealing with source code. */

#include <textview.H>
class srctextview: public textview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void PrepareInsertion(boolean insertingNewLine);
    void PostMenus(class menulist *menulist);
    void LoseInputFocus();
    void ReceiveInputFocus();
    void SetDataObject(class dataobject *dataobj);

    virtual void BalanceParens();
    virtual void Compress();
    virtual void CompressAll();
    virtual boolean ConfirmReadOnly();
    virtual void DecompressAll(long dummy);
    virtual void DeleteMenuItem(char menuitem[]);
    virtual void EndComment(char key);
    virtual boolean FindSubFile(char *filename, char *bufname, char *procname, char *searchpath);
    virtual void ForceUpperOn();
    virtual void ForceUpperOff();
    virtual void GotoColumn(int rock);
    virtual void HandleEndOfLineStyle(long pos);
    virtual void HandleNewlineAndRetrn(long key, boolean reindentThisLine, boolean preindentNewLine);
    virtual void InsertComment();
    virtual void InsertLineComment();
    virtual void MatchParens(char key);
    virtual void Paren(char key);
    virtual class keystate *PrependKeyState();
    virtual void PutFileIntoNewWindow(char *bufname, char *proc, char *filename);
    virtual void RedoStyles();
    virtual void Reformat();
    virtual void Reindent();
    virtual void RenameIdent();
    virtual void SelfInsert(char key);
    virtual void SelfInsertReindent(char key);
    virtual void StartComment(char key);
    virtual void StartLineComment(char key);
    virtual void StyleLabel(char key);
    virtual void StyleString(char key);
    virtual void WaitCursorOn();
    virtual void WaitCursorOff();
    virtual void WhatColumn();

    static boolean InitializeClass();
    srctextview();
    virtual ~srctextview();

    class keystate *src_state;
    class menulist *src_menus;    
};

#endif /* _srctextview_H_ */
