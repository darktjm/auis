#ifndef _srctextview_H_
#define _srctextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File srctextview.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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
/** @} */
#endif /* _srctextview_H_ */
