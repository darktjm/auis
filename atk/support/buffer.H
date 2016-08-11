ATK_INTER
#ifndef _buffer_H_
#define _buffer_H_
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


 

struct bufferContents { /* May not have to be in this file. */
    class view *bufferView; /* The actual view that is looking at the data object. */
    class view *bufferInputFocus; /* The view that is taking input. */
    class view *bufferApplicationView; /* The view the window is pointing at. (May not be neccesary). */
    boolean used; /* Probably should be boolean. */
};

/* Flag values for buffer_GetBufferOnFile function. */
#define buffer_MustExist 1 /* File must exist for operation to succeed. */
#define buffer_ForceNew 2 /* Make a new_c buffer even if one already exists. */
#define buffer_ReadOnly 4 /* Buffer will be readonly regardless of file status. */
#define buffer_RawMode 8 /* REad in in raw mode using a text object */

/* Flag values for buffer_WriteToFile function. */
#define buffer_ReliableWrite 1 /* Use vclose to ensure the file is written to server or an error code is returned. */
#define buffer_MakeBackup 2 /* Keep a backup file if the user preferences so indicates. */

#include <observable.H>
#include <bufferlist.H>

	typedef class view *(*buffer_evfptr)(class view *app, class view *view, class view *focus, long d);

class buffer: public  observable {
public:
	virtual ATKregistryEntry *ATKregistry();

         void ObservedChanged(class observable *object, long value);
    
	virtual class view * EnumerateViews(buffer_evfptr mapFunction, long functionData)  ; /* ??? */
        virtual void SetData(class dataobject *data);
        virtual void SetName(char *name);
        virtual void SetFilename(char *fileName);
        virtual void SetWriteVersion(long version);
        virtual void SetCkpVersion(long version);
        virtual void SetCkpClock(long clock);
        virtual void SetCkpLatency(long latency);
        virtual void SetScratch(boolean scratch);
        virtual class view * GetView(class view **inputFocus, class view **targetView, char *ViewName)  ;
        virtual boolean Visible()  ;
        virtual void RemoveView(class view *unusedView);
        virtual int ReadFile(char *filename)  ;
        virtual int WriteToFile(char *filename, long flags)  ;
        /* Disk file date may differ from GetTouchDate if file is modified externally. */
        virtual long GetFileDate()  ;
	virtual void SetDefaultViewname(char *name);
	virtual void SetDestroyData(boolean destroy);
	virtual void SetReadOnly(boolean readOnly);
	virtual void SetLastTouchDate(long dataStamp);
	virtual void SetCheckpointFilename();
	virtual void SetIsModified(boolean value);
	virtual void SetIsRawFile(boolean value);
    
        inline class dataobject * GetData() { return ((this)->bufferData); }
        inline char * GetName() { return ((this)->bufferName); }
        inline char * GetFilename() { return ((this)->filename); }
        inline char * GetCkpFilename() { return ((this)->ckpFilename); }
        inline long GetWriteVersion() { return ((this)->writeVersion); }
        inline long GetCkpVersion() { return ((this)->ckpVersion); }
        inline long GetCkpClock() { return ((this)->ckpClock); }
        inline long GetCkpLatency() { return ((this)->ckpLatency); }
        inline boolean GetScratch() { return ((this)->scratch); }
        inline boolean GetMadeBackup() { return ((this)->madeBackup); }
        inline void SetMadeBackup(boolean value) { ((this)->madeBackup = (value)); }
        /* Unavailable/unapplicable dates are returned as 0 */
        inline long GetLastTouchDate() { return ((this)->lastTouchDate); }
	inline char * GetDefaultViewname() { return ((this)->viewname); }
        inline boolean GetReadOnly() { return (this->readOnly); }
	inline boolean GetIsModified() { return (this->isModified); }
        inline boolean GetIsRawFile() { return (this->isRawFile); }
    
	static class buffer * Enumerate(bufferlist_efptr mapFunction, long functionData)  ;
	static class buffer * Create(char *bufferName, char *fileName, char *objetName, class dataobject *data)  ;
	static class bufferlist * GetGlobalBufferList()  ;

        
	static boolean InitializeClass();

	
/* Lookup functions */
        static class buffer * FindBufferByName(char *bufferName)  ;
        static class buffer * FindBufferByFile(char *filename)  ;
        static class buffer * FindBufferByData(class dataobject *data)  ;

/* File functions. */
        static class buffer * GetBufferOnFile(char *filename, long flags)  ;
        static void GuessBufferName( char *filename, char *bufferName, int nameSize);
        static void GetUniqueBufferName(char *proposedBufferName, char *bufferName, int nameSize);
        
	virtual ~buffer();
        
	buffer();
        static void SetDefaultObject(char *objectname);

    
        class dataobject *bufferData; /* The dataobject associated with this buffer. Can't be NULL.*/
        char *bufferName; /* This buffers name. Must be filled in. */
        char *filename; /* The filename this dataobject is saved in. May be NULL. */
        char *ckpFilename; /* Checkpoint file, filled in automatically. */
        struct bufferContents *viewList; /* Array of view triples viewing this buffer's dataobj. */
        int viewsAllocated, viewsUsed; /* viewsAllocated = number of slots in above table. viewsUsed = number of slots containing views. */
        long ckpVersion, writeVersion, ckpClock, ckpLatency; /* Dataobject version last checkpointed, last written. ckpClock is a sequence clock used for intelligent checkpointing. */
        long lastTouchDate; /* When was the file last read or written by the buffer */
        boolean destroyData; /* Are we supposed to destroy this when we are destroyed. */
        boolean scratch; /* Indicates that this buffer is temporary in nature. Namely, don't checkpoint it. */
        boolean readOnly; /* This is only a hint. If the dataobject ignores the read only attribute, this won't be true. */
	boolean madeBackup; /* This is used to tell if we have made a backup file yet this session. */
	boolean	isRawFile;  /* This is set to indicate that the file read in was read in in raw mode */
	boolean isModified;
	boolean askedAboutSymlink; /* TRUE iff the user wants to be asked about clobbering symlinks AND s/he's already been asked for this instance */
	boolean clobberSymlink;	/* TRUE means replace symlinks when writing */
        char *viewname;
};

#endif /* _buffer_H_ */