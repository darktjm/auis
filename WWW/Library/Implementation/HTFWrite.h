/*                                                         HTFWriter:  File Writer for libwww
                                      C FILE WRITER
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   It is useful to have both FWriter and Writer for environments in which fdopen() doesn't
   exist for example.
   
   This module is implemented by HTFWrite.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTFWRITE_H
#define HTFWRITE_H

#include "HTStream.h"
#include "HTFormat.h"/*

Set up a Stream to a File

   This function puts up a new stream given an open file descripter. If the file is not to
   be closed afterwards, then set leave_open = NO.
   
 */
extern HTStream * HTFWriter_new (FILE *  fp,
                                             BOOL       leave_open);/*

  CONVERTERS
  
   HTSaveAndCallBack will save to a cache file and call the request->callback function
   with the filename as parameter.
   
 */
#ifndef pyramid
extern HTConverter HTSaveAndExecute, HTSaveLocally, HTSaveAndCallBack,
       HTThroughLine;
#endif/*

Black Hole Stream

   This stream simply absorbs data without doing anything what so ever.
   
 */
extern HTStream * HTBlackHole (void);/*

Get Local File Name

   This function tries really hard to generate a non-existent file name on the local file
   system. You can specify the path of the file so that the file name is generated
   relatively to this. Two modes of file name generations exist: It can either generate a
   hash based file name or take the last part of the URL and generate a more human
   readable file name.
   
 */
extern char *HTFWriter_filename (char *          path,
                                        char *          url,
                                         CONST char *   suffix,
                                         unsigned int   limit,
                                         BOOL           flat);/*

The Client Cache

   The cache contains details of temporary disk files which contain the contents of remote
   documents.   There is also a list of cache items for each URL in its anchor object.
   
  AN ITEM IN THE CACHE
  
 */
typedef struct _HTCacheItem {
    HTParentAnchor *    anchor;
    HTFormat            format;         /* May have many formats per anchor */
    char *              filename;
    time_t              load_time;
    time_t              load_delay;
    int                 reference_count;
} HTCacheItem;
/*

  THE CACHE ITSELF
  
   There is one global cache. No, it's not reentrant code then.  If we find a use for >1
   cache then we can change it. Currently the obvious advantage is when all things which
   could gain from caching use the same one.
   
 */
extern HTList * HTCache;        /* made up of HTCacheItem items */
/*

  THE CACHE LIMIT
  
   The cache limit is the number of files which are kept. Yes, I know, the amount of disk
   space wouldbe more relevant. So this may change. Currentlyit is present to 100 but may
   be changed by the application by writing into this variable.
   
 */
extern int      HTCacheLimit;
/*

  CLEAR A CACHE
  
 */
extern void HTCacheClear (HTList * cache);
/*

  TO REMOVE ALL CACHE FILES KNOWN TO THIS SESSION
  
 */
extern void HTCacheDeleteAll (void);/*

  STREAM FOR WRITING TO CACHE
  
   Note that HTSaveAndCallBack will also generate a cache file.
   
 */
#ifndef pyramid
extern HTConverter HTCacheWriter;
#endif

#endif/*

   End of definition module */
