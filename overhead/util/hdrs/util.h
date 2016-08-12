/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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


#ifndef _UTIL_H_
#define _UTIL_H_ 1
                                 

#include <andrewos.h>

extern int fdplumb_SpillGuts();
extern int fdplumb_SpillGutsToFile(FILE *fp, int ExtraNewLines);
extern int dbg_creat(char *path, int mode);
extern int dbg_open(char *path, int flags, int mode);
extern FILE *dbg_fopen(char *path, char *type);
extern int dbg_close(int fd);
extern int dbg_fclose(FILE *fp);
extern int dbg_dup(int oldfd);
extern int dbg_dup2(int oldfd, int newfd);
extern int dbg_pipe(int fdarr[2]);
extern int dbg_socket(int af, int typ, int prot);
 
#ifdef POSIX_ENV
extern time_t gtime(struct tm *ct);
#endif
extern boolean IsOnVice(int fd);
extern boolean ViceIsRunning();
extern boolean fpacheck();
extern FILE *qopen(char *name, char **argv, char *mode);
extern int os_system(const char *cmd);
extern char *statustostr(WAIT_STATUS_TYPE *status, char *buf, int len);
extern char **strtoargv(char *command, char **argvbuf, int len);
extern char *argvtostr(char **argv,char *buf,int len);
extern int CheckServiceConfiguration();

extern int GetPty(int *master, int *pty); /* opens up both ends of a pty */
extern int GetPtyandName(int *master, int *pty, char *ptyname, int ptysize);
#ifdef WHITEPAGES_ENV
/* Compatibility routines */
extern int getvuid();             /* returns ViceID or -1 (sets errno on error) */
extern struct passwd *getvpwuid(int  vuid);     /* These work a lot like the non-V
                                        * versions */
extern struct passwd *getvpwnam(char  *vnam);
extern int setvpwent();
extern int endvpwent();
extern struct passwd *getvpwent();

extern struct passwd *getcpwuid(int  vuid , char  *vcell);     /* Take TWO arguments: a uid/nam and a
                                        * Vice cell name. */
extern struct passwd *getcpwnam(char  *vnam , char  *vcell);
extern int      cpw_error;

#else                                  /* WHITEPAGES_ENV */
/* Real backwards compatibility. */
#define getvuid getuid
#define getvpwuid getpwuid
#define getvpwnam getpwnam
#define getvpwent getpwent
#define setvpwent setpwent
#define endvpwent endpwent
#define getcpwuid(X, Y) getpwuid(X)
#define getcpwnam(X, Y) getpwnam(X)
#define cpw_error errno
#endif                                 /* WHITEPAGES_ENV */

extern int     FoldTRT[256];

extern boolean FoldedEQ(const char *a, const char *b);
extern boolean FoldedEQn(const char *a, const char *b, int n);
extern boolean FoldedWildEQ(const char *a, const char *b, boolean ignorecase);
#define FOLDEDEQ(s1,s2) (FoldTRT[(s1)[0]]==FoldTRT[(s2)[0]] && FoldedEQ(s1,s2))
#define FOLDEDEQN(s1,s2,n) (n <= 0 || (FoldTRT[(s1)[0]]==FoldTRT[(s2)[0]] && FoldedEQn(s1,s2,n)))

extern int lc_strcmp(const char *a, const char *b);
extern int lc_strncmp(const char *a, const char *b, int n);

extern char *gethome(char  *name);
extern char *getMyHome();

extern int GetHostDomainName(char  *buf , int  buflen);	/* works like gethostname() but extends with getdomainname() if necessary. */

struct configurelist {
    char           *programName;
    char           *key;
    char           *value;
    struct configurelist *next;
};

#define CONFIG_EOF -1
#define CONFIG_FOUNDENTRY 0
#define CONFIG_BADENTRY 1
#define CONFIG_COMMENT 2
#define CONFIG_FALSECONDITION 3
#define CONFIG_EMPTYLINE 4
#define CONFIG_NOKEY 5
#define CONFIG_NOVALUE 6

extern char *AndrewDir(char *suffix);
extern char *AndyDir(char *suffix);
extern char *LocalDir(char *suffix);
extern char *XLibDir(char *suffix);


extern int ReadConfigureLine(FILE  *fp, char  *text, int  maxTextLength, char  **program, int  *programLength, char  **key, int  *keyLength, char  **value, int  *valueLength, char  **condition, int  *conditionLength);   /* Reads a line from a file in
                                        * configure file format - returns one
                                        * of the above values */
extern struct configurelist *ReadConfigureFile(char  *fileName);       /* reads a configure
                                                         * file given by
                                                         * filename */
extern char *GetConfig(struct configurelist  *header, char  *key, int  usedefault);           /* returns the value corresponding to a
                                        * key for a given configurelist */
extern char *GetConfiguration(char  *key);    /* returns the value for a key in the
                                        * AndrewSetup file */
extern int FreeConfigureList(register struct configurelist  *cList);   /* frees a configure list */

extern void addstringprofile(char *str);
extern char *getprofile (char  *var );
extern char     *getprofilestring();
extern int getprofileint (char    *var , int DefaultValue);
extern int getprofileswitch (char    *var , boolean DefaultValue);
extern int profileentryexists (char    *var , boolean DefaultValue);
extern char *GetProfileFileName();
extern char *GetFirstProfileFileName();
extern int refreshprofile();

extern int setprofilestring(char  *prog , char  *pref , char  *val);

extern void findfileinpath(char *buffer, char *path, char *fname);

extern char *ap_Shorten(char  *pathname);          /* ap_Shorten(path) tries to shorten
                                        * path using the current home dir */
extern char *ap_ShortenAlso(char  *pathname , char  *auxI , char  *auxH);      /* (path, otherID, otherHD does the
                                        * same but with otherID/otherHD also. */
extern char *ap_ShortenTo(char  *pathname , int  maxLen);        /* (pathname, maxlen) shortens to a
                                        * maximum length, abbreviating the
                                        * path prefix to hyphens. */
extern char *ap_ShortenAlsoTo(char  *pathname , char  *auxI , char  *auxH , int  maxLen);    /* (pathname, otherID, otherHD, maxlen)
                                        * does both things. */

extern int fwriteallchars(char  *Thing, int  NItems, FILE  *stream);      /* pass (text, num, stream) and it
                                        * retries fwrite on interruption */
extern int writeall(int  fd, char  *Buf, int  NBytes);            /* pass (fd, buf, nbytes) and it
                                        * retries write on interruption */

extern char *NiceTime(long int Time);            /* Converts long arg to printable time
                                        * string without newline */

struct LinePromState {
    int             BeginDataLevel;
    char            InDefine, Promoting, SawBlankLine;
};

extern int BE2LinePromoteInit(struct LinePromState  **refstate);

 /*
  * int BE2LinePromoteInit(refstate) struct LinePromState **refstate; Returns
  * < 0 for (malloc) failure, 0 for OK.  Initializes *refstate to point to
  * malloc'ed storage that will hold the LinePromote state.
  */

extern int BE2LinePromote(char  *line , struct LinePromState  *state);

 /*
  * int BE2LinePromote(line, state) char *Line; struct LinePromState *state;
  * Works only on BE2 Datastream messages* Returns 2 if this line (including
  * the newline) should be ``promoted'' from the beginning of a message to the
  * beginning of an error message that encapsulates the message.  Returns 0 if
  * the line should stay where it is. Returns 1 if this line should be
  * ``demoted'' to the very tail end of the encapsulated message.  Returns < 0
  * on errors.
  */

extern int BE2LinePromoteEnd(struct LinePromState  *state);

 /*
  * int BE2LinePromoteEnd(state) struct LinePromState *state; Cleans up the
  * malloc'ed storage and returns 0 if OK, non-zero on errors.
  */

extern char *UnixSignal(int	 signalNumber);          /* Pass it a signal and it returns a
                                        * static string giving its name */

extern int VenusFlush(char  *pname);          /* Hand it the name of a file to flush
                                        * from Venus cache */
extern int VenusFlushCallback(char  *pname);  /* Hand it the name of a file for which
                                        * to flush the callback */
extern int VenusFetch(char  *pname);          /* Hand it the name of a file to
                                        * pre-fetch into the Venus cache */

/* Caveat: the CancelStore function is no longer implemented in in-kernel AFS (10/11/88) */
extern int VenusCancelStore(int  fid);    /* Hand it a fid and Venus won't store
                                        * the file on its close */

/* Functions to describe a user's authentication in multiple cells */
struct CellAuth {
    char           *CellName;          /* description for this cell */
    int             ViceID;            /* the ViceID within that cell */
    char           *UserName;          /* pw_name (login ID) for user in this
                                        * cell */
    char           *PersonName;        /* pw_gecos (personal name) for user in
                                        * this cell */
    char           *homeDir;           /* pwdir (home directory) for user in
                                        * this cell */
    int             WpError;           /* White pages error, if any, from
                                        * trying to get UserName and
                                        * PersonName. -1 means UserName,
                                        * PersonName, and homeDir not
                                        * initialized; 0 (wperr_NoError) means
                                        * that all are OK. */
    int             IsPrimary;         /* whether this cell is the primary one
                                        * for the user. */
    int             UsesAMSDelivery;   /* 0 initially; -1 for no, +1 for yes */
    unsigned long   ExpireTime;        /* When this token will expire (or 0 if
                                        * it's not valid now) */
    int             IsLocal;           /* whether this auth is local or
                                        * AFS-based. */
};
extern void EraseCellMemory();     /* Makes us get a new array of auth
                                        * descriptors next time. */
extern int FindCell(char  *cellName, struct CellAuth  **ppCellAuth);            /* int FindCell(cellName, ppCellAuth)
                                        * char *cellName; struct CellAuth
                                        * **ppCellAuth; Return a pointer to
                                        * our authentication for cell
                                        * cellName, via ppCellAuth. Return 0
                                        * if it was found, or an error code
                                        * (>0 for permanent, <0 for
                                        * temporary). Return 1 if we don't
                                        * have any authentication in that
                                        * cell, or if there's no such cell.
                                        * Return 2 if we're completely
                                        * unauthenticated. */

/* MISSING DEFINITION FindHomeCell() */
extern int       FindHomeCell();        /* int FindHomeCell(ppCellAuth) struct
                                        * CellAuth **ppCellAuth; Like
                                        * FindCell, except that it returns a
                                        * pointer to the $HOME cell, if there
                                        * is one. Return 1 if there's no home
                                        * cell. */

extern int FindAnyCell(struct CellAuth  **ppCellAuth);         /* int FindAnyCell(ppCellAuth) struct
                                        * CellAuth **ppCellAuth; Like
                                        * FindCell, except that it returns a
                                        * pointer to any authenticated cell,
                                        * if there is one. */

extern int FindNextCell(struct CellAuth  **ppCellAuth);        /* int FindNextCell(ppCellAuth) struct
                                        * CellAuth **ppCellAuth; Generate the
                                        * authenticated cells.  Starts and
                                        * ends with *ppCellAuth == NULL. */

extern void FillInCell(struct CellAuth  *cellAuth);          /* void FillInCell(cellAuth) struct
                                        * CellAuth *cellAuth; Fill in the
                                        * white pages values for the given
                                        * cell pointer; an error (or success)
                                        * code is left in cellAuth->WpError. */
extern int ca_UpdateCellAuths();  /* int ca_UpdateCellAuths(); Update the
                                        * ExpireTime fields in our CellAuth
                                        * structures. */

extern int vclose(int  fd);              /* Close a fileid and wait for it to
                                        * complete in Vice */
extern int vfclose(FILE  *f);             /* fclose a FILE* and wait for it to
                                        * complete in Vice */
extern int vdown(int  err);               /* return TRUE iff the errno value
                                        * passed as argument says that Vice or
                                        * Venus was down */
extern int tfail(int  errorNumber);               /* return TRUE iff the errno value
                                        * passed as arg is a temp failure */

#ifdef FILE
extern FILE *topen(char  *name , char  *argv[] , char  *mode, int  *pgrp);               /* like popen() but enable timeouts for
                                        * the close */
extern FILE *qopen(char  *name , char  *argv[] , char  *mode);               /* topen() with simplified calling
                                        * sequence */

#endif                                 /* FILE */
extern int tclose(FILE  *ptr, int  seconds , int  *timedout);              /* pclose() sensitive to subprocess
                                        * timeouts */
extern int qclose(FILE  *ptr);              /* tclose() with simplified calling
                                        * sequence */
extern int t2open(char  *name , char  *argv[], FILE  **r , FILE  **w);              /* topen() with both reading and
                                        * writing of subprocesses */
extern int t2close(FILE  *ptr, int  seconds , int  *timedout);             /* close the t2open() */

extern char *ULsindex(char  *big , char  *small);            /* Searches for substring of arg1 that
                                        * matches arg2, ignoring alpha case,
                                        * and returns a pointer to this
                                        * substr. If no match is found,
                                        * returns a 0. */
extern int ULstrcmp(register char  *s1 , register char  *s2);            /* Compares two arg strings, ignoring
                                        * alpha case, like strcmp. */
extern int ULstrncmp(char  *s1 ,char  *s2,int  n);	       /* Compares two arg strings, ignoring
                                        * alpha case, like strncmp. */
extern int ULstlmatch (char  *big,char  *small );          /* Returns 1 iff initial characters of
                                        * arg1 match arg2, ignoring alpha
                                        * case, else 0. */

extern char *NewString(char  *srcptr);           /* Return a dynamically-allocated copy
                                        * of the single arg string, or 0 if
                                        * allocation fails. */
#define FREESTRINGVAR(s)   \
		((s) = (char *)((s) ? (free(s), NULL) : NULL))
                                        /* Deallocate value from NewString.
                                        *  Set variable to NULL. */

extern void FreeString(char *srcptr);		/* Deallocate
                                        * string from NewString.  */

extern char *UnixError(int	 errorNumber);           /* Pass it an errno value and it
                                        * returns a static (canned) string
                                        * describing the error */

extern int GetCurrentWSCell(char  *Buf, int    size);    /* (Buf, size) */
extern int GetCellFromFileName(char  *FileName, char  *Buf, int    size); /* (Filename, Buf, size) */

extern void SetInitialArgs(int  argc , char  **argv , char  **envp);      /* SetInitialArgs(argc, argv,
                                        * envp|NULL)--pass proc title stuff
                                        * after copying everything wanted from
                                        * argv, and from envp, too, if that's
                                        * passed also. */
extern void SetProcTitle(char  *str, ...);        /* SetProcTitle(str, a1, a2, a3, a4,
                                        * a5)--like printf into the process
                                        * title. */

extern unsigned long getaddr ();	       /* Return our internet address as a 
				        * long in network byte order.  
					* Returns zero if it can't find one. */

extern void output64chunk(int c1, int c2, int c3, int pads, FILE *fp);
extern int hexchar(int c);
extern int char64(int c);
extern int from64(FILE *, FILE *);
extern void to64(FILE *, FILE *);
extern int fromqp(FILE *, FILE *);
#define ORDINALIZE(i) \
  (((((i)%100)==11)||(((i)%100)==12)||(((i)%100)==13))?"th": \
   (((i)%10)==1)?"st": \
   (((i)%10)==2)?"nd": \
   (((i)%10)==3)?"rd":"th")
/* Use ORDINALIZE like this:
   printf("the %d%s element is %s,\n",
          i, ORDINALIZE(i), a[i]);
*/

#define CARDINALIZE(i,z,o,m) \
  (((i)==0)?(z): \
   ((i)==1)?(o):m)
/* Use CARDINALIZE like this:
   printf(CARDINALIZE(n, 
                      "There are no elements.\n",
		      "There is one element.\n",
		      "There are %d elements.\n"),
	  n); */


#ifndef ERRHDLR_H
#define ERRHDLR_H

#include <setjmp.h>
#include <sys/param.h>

#define EH_ERR_MSG_BUF_SIZE (4*MAXPATHLEN)

typedef struct {
  jmp_buf jb;
} EH_environment;

extern char EH_Error_Msg[];
extern EH_environment *_error_handler_env;

#define EH_ret_code(module, code) ((int)(((int)(module)*0x10000)+(ABS((int)(code))&0xffff)))
#define EH_module(ehcode) (((int)ehcode)/0x10000)
#define EH_code(ehcode) (((int)ehcode)%0x10000)

#define EH_err(errcode, errmsg) \
  (strncpy(EH_Error_Msg, (errmsg), EH_ERR_MSG_BUF_SIZE), \
   longjmp(_error_handler_env->jb, (errcode)))

#define EH_cond_error_on(test, errcode, errmsg) \
  if (test) EH_err((errcode),(errmsg))

#define EH_propagate_error(comment) \
  (strncat(EH_Error_Msg, ", ", EH_ERR_MSG_BUF_SIZE-strlen(EH_Error_Msg)-1), \
   strncat(EH_Error_Msg, (comment), EH_ERR_MSG_BUF_SIZE-strlen(EH_Error_Msg)-1), \
   longjmp(_error_handler_env->jb, _err_code))

#define EH_restore_handler \
  _error_handler_env = _saved_handler_env

#define EH_begin \
{ \
  int _err_code; \
  EH_environment *_saved_handler_env, _new_handler_env; \
  _saved_handler_env = _error_handler_env; \
  if ((_err_code = setjmp(_new_handler_env.jb)) == 0) { \
    _error_handler_env = &(_new_handler_env);

#define EH_handle \
  } else { \
    EH_restore_handler;

#define EH_end \
  } \
  EH_restore_handler; \
}

#define EH_break         EH_restore_handler; break
#define EH_continue      EH_restore_handler; continue
#define EH_goto          EH_restore_handler; goto
#define EH_return        EH_restore_handler; return
#define EH_return_val(x) EH_restore_handler; return(x)

/*
  Standard routines:
     These routines will signal errors, so be prepared to catch those errors.
*/

extern char *emalloc(long  size);
extern char *erealloc(char  *old, long  newsize);

/*
  Module definitions:
*/

#define EH_module_system 0
#define EH_module_prs 1
#define EH_module_alq 2

#endif /* ERRHDLR_H */


#ifdef AFS30_ENV
/* Error codes for aq_XXX functions.  Actually, their negatives are returned. */
#define ALQ_ERRNO 1
#define ALQ_ENONAME 2
#define ALQ_EPARSEACL 3
#define ALQ_NOT_GROUP 4
#define ALQ_SYSTEM_GROUP 5
#define ALQ_EPRS_BASE 20

extern int aq_GroupP(char  *group , char  *groupcell);
/* Returns 1 if the name "group" is a groupname, 0 if not, <0 on errors. */

extern int aq_GetGroupMembers(char  *group , char  *groupcell , char  **outBuf);  
/* Return the members of the given group as a newline-separated list in 
   outBuf, which is modified to point to a malloc'd string.  The return 
   value is 0 if all is OK and negative on errors: a return value of -1 
   means to look in errno. */

extern int aq_UserInGroup(char  *user , char  *usercell , char  *group , char  *groupcell);
/* Return whether the given user is in the given group in the given cell.  
   1 means YES, 0 means NO, negative numbers are error codes; 
   -1 means to look in errno. */


extern long int aq_UserRightsToDir(char  *user , char  *usercell , char  *dir);
/* Return the access rights that the given user has to the given dir.  
   Negative numbers are error codes; -1 means to look in errno. */

extern int aq_CheckUserAllRightsToDir(char  *user , char  *usercell , char  *dir , long int rights);
/* Check whether the given user has all of a collection of rights to 
   the given directory.  Return 1 if YES, 0 if NO; negative numbers 
   are error codes, and -1 means to look in errno. */

extern int aq_CheckUserAnyRightToDir(char  *user , char  *usercell , char  *dir , long int rights);
/* Check whether the given user has any of a collection of rights to the 
   given directory.  Return 1 if YES, 0 if NO; negative numbers are 
   error codes, and -1 means to look in errno. */

extern char *aq_GetLastErrorMessage();
/* Returns the text string of the last error message.  Points to static 
   storage, do NOT free. */

#endif /* AFS30_ENV */
#endif /* _UTIL_H_ */

ENDCPLUSPLUSPROTOS
 
