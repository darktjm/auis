/* Copyright 1995 Carnegie Mellon University -- All rights reserved

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
#ifndef _webcom_H_
#define _webcom_H_
ATK_INTER

#include <atom.H>


#define WEBCOM_SUCCESS_NOFILE 1
#define WEBCOM_SUCCESS_WEB 2
#define WEBCOM_SUCCESS_OTHER 3
#define WEBCOM_SUCCESS_EXTERNAL 4
#define WEBCOM_REQUESTCANCELED 5
#define WEBCOM_LOADERROR 6
#define WEBCOM_UNKNOWN 7
#define WEBCOM_ERROR 8
#define WEBCOM_SUCCESS 9
#define WEBCOM_UNKNOWNOBJECT 10
#define WEBCOM_FORKFAILED 11
#define WEBCOM_WAITING 12
#define WEBCOM_MIMEREAD 13

#define WEBCOM_NoStatus 0 
#define WEBCOM_HasURL 01 /* contains a url */
#define WEBCOM_LoadPending 02 /*  a load is in progress */
#define WEBCOM_Loaded 04/*  the load has completed (i.e. the web file has been transfer to  */
#define WEBCOM_LoadCancelled 010/*  the load was canceled by the user */
#define WEBCOM_LoadFailed 020 /*  A load has failed, however an error message may be read by  */
#define WEBCOM_WebReadable 040 /*  the object is known to be a text/plain or text/html type,  */
#define WEBCOM_External 0100 /*  the file requires an external viewer.  */
#define WEBCOM_UnknownType 0200 /*  the file requires an unavailable or unknown external  */
#define WEBCOM_AUISReadable 0400 /*  can be viewed AUIS by an object other then Web */
#define WEBCOM_NoFile 01000/*  there is no file to display (ex. an ftp file transfer ) */
#define WEBCOM_HasWeb 02000 /*  a web object is present. */
#define WEBCOM_WebRead 04000 /*   WEBCOM_ReadWeb has successfully completed */
#define WEBCOM_WebReadFailed 010000 /*   WEBCOM_ReadWeb has failed */
#define WEBCOM_WebViewed 020000 /*   WEBCOM_ExternalView has successfully completed(whether it  */
#define WEBCOM_WebViewFailed 040000 /*   WEBCOM_ExternalView has failed. */
#define WEBCOM_Preserved 0100000 /*  the cached temp file should not be deleted.  */
#define WEBCOM_HasType 0200000 /* the type has been recorded. */
#define WEBCOM_LoadStatus (WEBCOM_LoadPending | WEBCOM_Loaded | WEBCOM_LoadCancelled | WEBCOM_LoadFailed )
#define WEBCOM_ClearLoadStatus(A) A->status &= ~WEBCOM_LoadStatus
#define WEBCOM_SetLoadStatus(A,STAT) {WEBCOM_ClearLoadStatus(A) ; A->status |= STAT; }

#define WEBCOM_ViewRaw 1
#define WEBCOM_Reload 2
#define WEBCOM_Post 4
#define WEBCOM_TopLevel 8

class webcom;
typedef void  (*webcom_cbptr)(void *, webcom *, int);


#include <observable.H>
class webcom: public observable{
public:
	virtual ATKregistryEntry *ATKregistry();

	static boolean InitializeClass();
	webcom();
	virtual ~webcom();

	static class webcom *CreateFromStream(FILE *f) ;
	static class webcom *Create(char *url, class webcom *parent, 
		int flags, const char *postdata=NULL) ;
	static char * GenPass(char *name, char *passwd)  ;
	static char * promptForPasswd(class view *view)  ;
	static int SetCacheSize(int count)  ;
	static int GetCacheSize(int count)  ;
	static int PromptForURL(class view *v)  ;
	static int GoTo(class textview *v)  ;
	static void KillChild(int restart) ; 
	static void Cancel(char *buf);
	static class webcom *FindWebcomFromData(
			class dataobject *dat)  ;
	/* Transition procedures */
	virtual int Display(class view*v)  ;
	virtual void Load(webcom_cbptr NotifyProc, void *rock)  ;
	virtual int Wait(class view *v)  ;
	virtual class web * GetWeb()  ;
	virtual class dataobject * GetObject() ;
	virtual void SetWeb(dataobject *pweb); 
	virtual class buffer * MakeBuffer( class frame *frame,
		boolean browse)  ;
	virtual void Adopt(class webcom *child);
	virtual void Cull();
	virtual void SetPasswd( char *passwd);
	virtual char * GetPasswd()  ;
	virtual class webcom * GetParent()  ;
	virtual int ReadWeb()  ;
	virtual char * GetParentURL()  ;
	virtual long Status()  ;
	virtual void SetPreserve( boolean doit);
	virtual long  Size()  ;
	virtual char * Error()  ;
	virtual char * GetURL () ;
	virtual char * GetTitle () ;
	virtual void SetTitle (char *title);
	virtual FILE * Open(boolean readcontext)  ;
	virtual void Close(FILE *f);
	virtual class atom *  Type()  ;
	virtual int  ExternalView()   ;
	virtual char *  CanView()  ;
	virtual int Delete()  ;
	virtual int Reload()  ;
	virtual char * GetTmpName()  ;
	virtual class image  * getimage(char *url,char *type)  ;
	virtual char * getfullname(char *name)  ;
	virtual void SetContext(char *begin);
        virtual void ReadContext(FILE *file,int id);
        static void EncodeForURL(flex &url, const char *fragment);
	inline class atom  * GetType() { return ((this)->type); }
	
	class atom *type,*url;
	int status;
	class dataobject *data; /* may be web */
	class buffer *buffer;
	char *context;
	class webcom *parent,*nextpending;
	class webcom *next;
	webcom_cbptr proc;	/* procedure to call for event */
	void *procdata;	/* data to be passed to proc */
	class atom *mimetype;
	class atom *title;
	unsigned long reqid;
	char *file;
        char *errfile;
        boolean raw;
        boolean reload;
        char *postfile;
};

#endif /* _webcom_H_ */
