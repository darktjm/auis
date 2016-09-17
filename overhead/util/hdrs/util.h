/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* fdplumb*.c */
extern int fdplumb_SpillGuts(void);
extern int fdplumb_SpillGutsToFile(FILE *fp, int ExtraNewLines);
extern int dbg_creat(const char *path, int mode);
extern int dbg_open(const char *path, int flags, int mode);
extern FILE *dbg_fopen(const char *path, const char *type);
extern int dbg_close(int fd);
extern int dbg_fclose(FILE *fp);
extern int dbg_dup(int oldfd);
extern int dbg_dup2(int oldfd, int newfd);
extern int dbg_pipe(int fdarr[2]);
extern int dbg_socket(int af, int typ, int prot);
int dbg_socketpair(int dom, int typ, int prot, int sv[]);
FILE *dbg_popen(const char *path, const char *type);
int dbg_pclose(FILE *fp);
FILE *dbg_qopen(const char *path, const char * const argv[], const char *mode);
FILE *dbg_topen(const char *path, const char * const argv[], const char *mode, int *pgrp);
int dbg_qclose(FILE *fp);
int dbg_tclose(FILE *fp, int seconds, int *timeout);
int dbg_t2open(const char *name, char * const argv[], FILE **r, FILE **w);
int dbg_t2close(FILE *fp, int seconds, int *timedout);
#include <dirent.h>
DIR *dbg_opendir(const char *name);
void dbg_closedir(DIR *d);

/* gtime.c */
extern time_t gtime(struct tm *ct);

/* fpacheck.c */
extern boolean fpacheck(void);

/* system.c */
extern int os_system(const char *cmd);

/* procstuf.c */
extern char *statustostr(WAIT_STATUS_TYPE *status, char *buf, int len);
extern const char **strtoargv(char *command, const char **argvbuf, int len);
extern char *argvtostr(const char * const *argv,char *buf,unsigned int len);

extern int GetPty(int *master, int *pty); /* opens up both ends of a pty */
extern int GetPtyandName(int *master, int *pty, char *ptyname, int ptysize);
/* Real wp backwards compatibility. */
#define getvuid getuid
#define getvpwuid getpwuid
#define getvpwnam getpwnam
#define getvpwent getpwent
#define setvpwent setpwent
#define endvpwent endpwent
#define getcpwuid(X, Y) getpwuid(X)
#define getcpwnam(X, Y) getpwnam(X)
#define cpw_error errno

/* foldedeq.c */
extern const int     FoldTRT[256];

extern boolean FoldedEQ(const char *a, const char *b);
extern boolean FoldedEQn(const char *a, const char *b, int n);
extern boolean FoldedWildEQ(const char *a, const char *b, boolean ignorecase);
#define FOLDEDEQ(s1,s2) (FoldTRT[(unsigned char)(s1)[0]]==FoldTRT[(unsigned char)(s2)[0]] && FoldedEQ(s1,s2))
#define FOLDEDEQN(s1,s2,n) (n <= 0 || (FoldTRT[(unsigned char)(s1)[0]]==FoldTRT[(unsigned char)(s2)[0]] && FoldedEQn(s1,s2,n)))

extern int lc_strcmp(const char *a, const char *b);
extern int lc_strncmp(const char *a, const char *b, int n);

/* lcappend.c */
void LCappend(char *s1, const char *s2);

/* lcstring.c */
char *lcstring(char *d, const char *s, int n);

/* ucstring.c */
char *ucstring(char *d, const char *s, int n);

/* getla.c */
extern double getla(int index);
extern void getla_ShutDown(void);

/* gethome.c */
extern const char *gethome(const char  *name);

/* getmyhom.c */
extern const char *getMyHome(void);

/* hname.c */
extern int GetHostDomainName(char  *buf , int  buflen);	/* works like gethostname() but extends with getdomainname() if necessary. */

struct configurelist {
    char           *programName;
    char           *key;
    char           *value;
    struct configurelist *next;
};

/* andrwdir.c */
extern const char *AndrewDir(const char *suffix);

/* andydir.c */
extern const char *AndyDir(const char *suffix);

/* localdir.c */
extern const char *LocalDir(const char *suffix);

/* xbasedir.c */
extern const char *XLibDir(const char *suffix);

/* config.c */
extern char ProgramName[];
#ifdef LOCAL_ANDREW_SETUP_ENV
#define conf_NumConfigNames 6
#else
#define conf_NumConfigNames 5
#endif
extern const char *conf_ConfigNames[conf_NumConfigNames + 1];
extern int conf_ConfigUsed, conf_ConfigErrno;
#define CONFIG_EOF -1
#define CONFIG_FOUNDENTRY 0
#define CONFIG_BADENTRY 1
#define CONFIG_COMMENT 2
#define CONFIG_FALSECONDITION 3
#define CONFIG_EMPTYLINE 4
#define CONFIG_NOKEY 5
#define CONFIG_NOVALUE 6
extern int ReadConfigureLine(FILE  *fp, char  *text, int  maxTextLength, const char  **program, int  *programLength, const char  **key, int  *keyLength, const char  **value, int  *valueLength, const char  **condition, int  *conditionLength);   /* Reads a line from a file in
                                        * configure file format - returns one
                                        * of the above values */
extern struct configurelist *ReadConfigureFile(const char  *fileName);       /* reads a configure
                                                         * file given by
                                                         * filename */
extern const char *GetConfig(const struct configurelist  *header, const char  *key, int  usedefault);           /* returns the value corresponding to a
                                        * key for a given configurelist */
extern const char *GetConfiguration(const char  *key);    /* returns the value for a key in the
                                        * AndrewSetup file */
extern struct configurelist *ReadStringConfig(char *str);
extern void FreeConfigureList(struct configurelist  *cList);   /* frees a configure list */

/* profile.c */
extern void addstringprofile(char *str);
extern const char *getprofile (const char  *var );
extern int getprofileint (const char    *var , int DefaultValue);
extern int getprofileswitch (const char    *var , boolean DefaultValue);
extern int profileentryexists (const char    *var , boolean DefaultValue);
extern const char *GetProfileFileName(void);
extern const char *GetFirstProfileFileName(void);
extern void refreshprofile(void);

/* desym.c */
extern int DeSymLink(const char *inp, char *outp, int newRoots);

/* setprof.c */
extern int setprofilestring(const char  *prog , const char  *pref , const char  *val);

/* findfile.c */
extern void findfileinpath(char *buffer, const char *path, const char *fname);

/* abbrpath.c */
/* note: "const" pathname only applies if it isn't shorten's static buffer */
extern const char *ap_Shorten(const char  *pathname);          /* ap_Shorten(path) tries to shorten
                                        * path using the current home dir */
extern const char *ap_ShortenAlso(const char  *pathname , const char  *auxI , const char  *auxH);      /* (path, otherID, otherHD does the
                                        * same but with otherID/otherHD also. */
extern const char *ap_ShortenTo(const char  *pathname , int  maxLen);        /* (pathname, maxlen) shortens to a
                                        * maximum length, abbreviating the
                                        * path prefix to hyphens. */
extern const char *ap_ShortenAlsoTo(const char  *pathname , const char  *auxI , const char  *auxH , int  maxLen);    /* (pathname, otherID, otherHD, maxlen)
                                        * does both things. */

/* fselect.c */
extern int fselect(int nfds, FILE **rfiles, FILE **wfiles, FILE **xfiles, struct timeval *timeout);

/* fwrtallc.c */
extern int fwriteallchars(const char  *Thing, int  NItems, FILE  *stream);      /* pass (text, num, stream) and it
                                        * retries fwrite on interruption */

/* writeall.c */
extern int writeall(int  fd, const char  *Buf, int  NBytes);            /* pass (fd, buf, nbytes) and it
                                        * retries write on interruption */

/* nicetime.c */
extern const char *NiceTime(long int Time);            /* Converts long arg to printable time
                                        * string without newline */

struct LinePromState {
    int             BeginDataLevel;
    char            InDefine, Promoting, SawBlankLine;
};

/* lineprom.c */
extern int BE2LinePromoteInit(struct LinePromState  **refstate);
 /*
  * int BE2LinePromoteInit(refstate) struct LinePromState **refstate; Returns
  * < 0 for (malloc) failure, 0 for OK.  Initializes *refstate to point to
  * malloc'ed storage that will hold the LinePromote state.
  */
extern int BE2LinePromote(const char  *line , struct LinePromState  *state);
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

/* usignal.h */
extern const char *UnixSignal(int	 signalNumber);          /* Pass it a signal and it returns a
                                        * static string giving its name */

/* tfail.c */
extern int tfail(int  errorNumber);               /* return TRUE iff the errno value
                                        * passed as arg is a temp failure */

/* topen.c */
extern FILE *topen(const char  *name , const char  * const argv[] , const char  *mode, int  *pgrp);               /* like popen(void) but enable timeouts for
                                        * the close */
extern FILE *qopen(const char  *name , const char  * const argv[] , const char  *mode);               /* topen(void) with simplified calling
                                        * sequence */

extern int tclose(FILE  *ptr, int  seconds , int  *timedout);              /* pclose(void) sensitive to subprocess
                                        * timeouts */
extern int qclose(FILE  *ptr);              /* tclose(void) with simplified calling
                                        * sequence */

/* t2open.c */
extern int t2open(const char  *name , char  * const argv[], FILE  **r , FILE  **w);              /* topen(void) with both reading and
                                        * writing of subprocesses */
extern int t2close(FILE  *ptr, int  seconds , int  *timedout);             /* close the t2open(void) */

/* ulsindex.c */
extern char *ULsindex(const char  *big , const char  *small);            /* Searches for substring of arg1 that
                                        * matches arg2, ignoring alpha case,
                                        * and returns a pointer to this
                                        * substr. If no match is found,
                                        * returns a 0. */

/* ulstrcmp.c */
extern int ULstrcmp(const char  *s1 , const char  *s2);            /* Compares two arg strings, ignoring
                                        * alpha case, like strcmp. */
extern int ULstrncmp(const char  *s1 ,const char  *s2,int  n);	       /* Compares two arg strings, ignoring
                                        * alpha case, like strncmp. */

/* ulstlmat.c */
extern int ULstlmatch (const char  *big,const char  *small );          /* Returns 1 iff initial characters of
                                        * arg1 match arg2, ignoring alpha
                                        * case, else 0. */

/* formerly newstr.o */
#define NewString strdup
#define FREESTRINGVAR(s)   \
		((s) = (char *)((s) ? (free(s), NULL) : NULL))
                                        /* Deallocate value from NewString.
                                        *  Set variable to NULL. */

extern void FreeString(char *srcptr);		/* Deallocate
                                        * string from NewString.  */

/* uerror.c */
extern const char *UnixError(int	 errorNumber);           /* Pass it an errno value and it
                                        * returns a static (canned) string
                                        * describing the error */

/* titles.c */
extern void SetInitialArgs(int  argc , char  **argv , char  **envp);      /* SetInitialArgs(argc, argv,
                                        * envp|NULL)--pass proc title stuff
                                        * after copying everything wanted from
                                        * argv, and from envp, too, if that's
                                        * passed also. */
extern void SetProcTitle(const char  *str, ...);        /* SetProcTitle(str, a1, a2, a3, a4,
                                        * a5)--like printf into the process
                                        * title. */

/* getaddr.c */
extern unsigned long getaddr (void);	       /* Return our internet address as a 
				        * long in network byte order.  
					* Returns zero if it can't find one. */

/* encode.c */
extern void output64chunk(int c1, int c2, int c3, int pads, FILE *fp);
extern int hexchar(char c);
extern int char64(char c);
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


/* errhdlr.c */
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

extern void *emalloc(long  size);
extern void *erealloc(void  *old, long  newsize);
#define CopyString strdup

/*
  Module definitions:
*/

#define EH_module_system 0
#define EH_module_prs 1
#define EH_module_alq 2

#endif /* ERRHDLR_H */

#endif /* _UTIL_H_ */

ENDCPLUSPLUSPROTOS
 
