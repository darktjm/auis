#ifndef ATKSYMTAB
#define ATKSYMTAB 1
/* Copyright 1994 Carnegie Mellon University All rights reserved. */

/** \addtogroup libATKLink libATKLink.a
 * @{ */

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

/** @} */

#endif /* ATKSYMTAB */
