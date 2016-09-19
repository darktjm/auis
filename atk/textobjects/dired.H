ATK_INTER
#ifndef _dired_H_
#define _dired_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
 * dired.H
 *
 * This object is a subclass of text whose purpose is to
 * read a specified directory and keep a list of the files in
 * a text object.
 *
 * Allows each file in the list to be marked or unmarked.  Marked
 * files are wrapped in a highlighting style.  A way is provided to
 * enumerate through all files or just the marked ones.
 *
 * The fileinfo structure associates information with each file.
 * Pointers to fileinfo structures are used for data values in a list.
 */
typedef void *(*dired_efptr)(char *name, long rock);
struct fileinfo {
    char *fileName, *dispName;
    long pos, len;              /* Where in text */
    class environment *env;    /* NULL if not marked */
};

#include <text.H>
class dired: public  text {
public:
	virtual ATKregistryEntry *ATKregistry();



    
	dired();
    
	virtual ~dired();


     long GetModified()  ;
     void SetAttributes(struct attributes *attributes);


    virtual long SetDir(const char *dname)  ;
    virtual char * GetDir()  ;            /* Null if none */

    virtual char * Locate(long pos)  ;    /* Filename */

    virtual void Mark(const char *fname);
    virtual void Unmark(const char *fname);
    virtual boolean IsMarked(const char *fname)  ;
    virtual boolean AnythingMarked()  ;

    virtual char * EnumerateAll(dired_efptr proc, long rock)  ;
    virtual char * EnumerateMarked(dired_efptr proc, long rock)  ;


    inline void SetLongMode(boolean on) { ((this)->longMode = (on)); }
    inline boolean GetLongMode() { return ((this)->longMode); }
    inline void SetDotFiles(boolean on) { ((this)->dotFiles = (on)); }
    inline boolean GetDotFiles() { return ((this)->dotFiles); }


    class style *markedStyle;

    char *dir;                  /* Being edited */
    class list *flist;         /* List of struct fileinfo */

    boolean longMode, dotFiles;
};

#endif /* _dired_H_ */
