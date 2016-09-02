/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
*Permission to use, copy, modify, and distribute this software and its 
*documentation for any purpose and without fee is hereby granted, provided 
*that the above copyright notice appear in all copies and that both that 
*copyright notice and this permission notice appear in supporting 
*documentation, and that the name of IBM not be used in advertising or 
*publicity pertaining to distribution of the software without specific, 
*written prior permission. 
*                        
*THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
*TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
*MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
*HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
*DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
*DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
*OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
*WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
* $
*/

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

struct fileinfo {
    char *fileName, *dispName;
    long pos, len;              /* Where in text */
    struct environment *env;    /* NULL if not marked */
};

class dired: text {

classprocedures:
    InitializeObject(struct dired *self) returns boolean;
    FinalizeObject(struct dired *self);

overrides:
    GetModified() returns long;
    SetAttributes(struct attributes *attributes);

methods:
    SetDir(char *dname) returns long;
    GetDir() returns char *;            /* Null if none */

    Locate(long pos) returns char *;    /* Filename */

    Mark(char *fname);
    Unmark(char *fname);
    IsMarked(char *fname) returns boolean;
    AnythingMarked() returns boolean;

    EnumerateAll(procedure proc, long rock) returns char *;
    EnumerateMarked(procedure proc, long rock) returns char *;

macromethods:
    SetLongMode(boolean on) ((self)->longMode = (on))
    GetLongMode() ((self)->longMode)
    SetDotFiles(boolean on) ((self)->dotFiles = (on))
    GetDotFiles() ((self)->dotFiles)

data:
    struct style *markedStyle;

    char *dir;                  /* Being edited */
    struct list *flist;         /* List of struct fileinfo */

    boolean longMode, dotFiles;
};
