ATK_INTER
#ifndef _path_H_
#define _path_H_
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


/*
The path class is for caching information about a pathname --
whether or not it names a directory, and if so, what the 
directories and files are that it contains.

The information returned by
IsDirectory, GetFiles, GetDirs, GetNumFiles and GetNumDirs
is only as recent as the last call to Scan. Scan will stat the
directory, and re-read it only if necessary.
*/

#include <observable.H>

class path : public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();

  
    
	path();
    
	virtual ~path();
    static class path * Create(char *filepath)  ;
    static char * TruncatePath(char *filepath, char *resultPath, long limit, boolean tryHome)  ;
    static void FreeTruncatedPaths();
    static void InputTruncatedPathCache(FILE *fp);
    static void OutputTruncatedPathCache(FILE *fp);
    static boolean ModifyToParentDirectory(char *filepath, boolean isDirectory)  ;
    static const char * UnfoldFileName(const char *fromString, char *toString, char *baseName)  ;
    static void FindFileInPath(char *retbuff, const char *path, const char *fname);
    static boolean RelativizePathname(char *buf, char *basepath, int maxup);
  
    virtual boolean Scan(boolean statEverything)  ;
    virtual void ReleaseFiles(char **filelist);
    virtual void ReleaseDirs(char **dirlist);
    virtual void Input(FILE *fp);
    virtual void Output(FILE *fp);
    virtual boolean IsDirectory()  ;
    virtual char ** GetFiles()  ;
    virtual char ** GetDirs()  ;
    virtual long GetNumFiles()  ;
    virtual long GetNumDirs()  ;
    virtual char * GetTruncatedPath()  ;
  
    inline char * GetPath() { return ((this)->filepath); }
  
    char *filepath;
    char *truncatedPath;
    time_t changetime;
    boolean haveScanned;
    boolean knowIsDir;
    boolean mayBeWrong; /* if the last scan didn't stat everything */
    boolean isDir;
    char **files;
    char **dirs;
    long numFiles;
    long numDirs;
};

#endif /* _path_H_ */
