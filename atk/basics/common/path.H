#ifndef _path_H_
#define _path_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
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
#include <stdio.h>
#include <time.h>

class path : public  observable {
public:
	TRACED_CLASS(path);

  
    
	path();
    
    static class path * Create(char *filepath)  ;
    static char * TruncatePath(char *filepath, char *resultPath, long limit, boolean tryHome)  ;
    static void FreeTruncatedPaths();
    static void InputTruncatedPathCache(FILE *fp);
    static void OutputTruncatedPathCache(FILE *fp);
    static boolean ModifyToParentDirectory(char *filepath, boolean isDirectory)  ;
    static const char * UnfoldFileName(const char *fromString, char *toString, const char *baseName)  ;
    static void FindFileInPath(char *retbuff, const char *path, const char *fname);
    static boolean RelativizePathname(char *buf, const char *basepath, int maxup);
  
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
/** @} */
#endif /* _path_H_ */
