#ifndef ATKSYMTAB
#define ATKSYMTAB 1
/* Copyright 1994 Carnegie Mellon University All rights reserved.
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

// ATKSymTab: a hash table containing symbol names.
// Typical use will be in collecting the set of import or export
// filenames used by a dynamic object or executable.
// Deletion will not be supported.
// Supported operations will be:
// boolean AddSymbol(char *) returns TRUE iff the
// symbol was successfully added.
// boolean FindSymbol(char *) returns TRUE iif the
// symbol was found.

#include <andrewos.h>

// ATKSymList: used internally by ATKSymtab to contain
// the contents of each bucket.
class ATKSymList {
  private:
    static const int BumpSize;
    char *syms;
    int size;
    int used;
    int extra;
  public:
    ATKSymList();
    ATKSymList(int extra);
    ~ATKSymList();
    void *AddSymbol(const char *sym);
    void *FindSymbol(const char *sym);
    void ResetIterator();
    char *NextSymbol();
    void SetExtra(int ex);
};
    
class ATKSymTab {
  private:
    static const int HashSize;
    ATKSymList *sl;
    int Hash(const char *sym);
  public:
    ATKSymTab();
    ATKSymTab(int extra);
    ~ATKSymTab();
    void *AddSymbol(const char *sym);
    void *FindSymbol(const char *sym);
    void ResetIterator();
    char *NextSymbol();
};

#endif /* ATKSYMTAB */
