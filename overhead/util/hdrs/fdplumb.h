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


 

#ifndef PLUMBFDLEAKS
#define PLUMBFDLEAKS
#include <stdio.h>
#include <fcntl.h>

#define open dbg_open
extern int dbg_open(char  *path, int  flags , int  mode);

#define fopen dbg_fopen
extern FILE * dbg_fopen(char  *path , char  *type);

#define close dbg_close
extern int dbg_close(int  fd);

#define fclose dbg_fclose
extern int dbg_fclose(FILE  *fp);

#define vclose dbg_vclose
extern dbg_vclose(int  fd);

#define vfclose dbg_vfclose
extern dbg_vfclose(FILE  *fp);

#define popen dbg_popen
extern FILE *dbg_popen(char  *path , char  *type);

#define pclose dbg_pclose
extern dbg_pclose(FILE  *fp);

#define topen dbg_topen
extern FILE *dbg_topen(char  *path , char  *argv[] , char  *mode, int  *pgrp);

#define tclose dbg_tclose
extern dbg_tclose(FILE  *fp, int  seconds , int  *timedout);

#define t2open dbg_t2open
extern dbg_t2open(char  *name , char  *argv[], FILE  **r , FILE  **w);

#define t2close dbg_t2close
extern dbg_t2close(FILE  *fp, int  seconds , int  *timedout);

#define qopen dbg_qopen
extern FILE *dbg_qopen(char  *path , char  *argv[] , char  *mode);

#define qclose dbg_qclose
extern dbg_qclose(FILE  *fp);

#define creat dbg_creat
extern int dbg_creat(char  *path, int  mode);

#define dup dbg_dup
extern int dbg_dup(int  oldfd);

#define dup2 dbg_dup2
extern int dbg_dup2(int  oldfd , int  newfd);

#define pipe dbg_pipe
extern int dbg_pipe(int  fdarr[2]);

#define socket dbg_socket
extern int dbg_socket(int  af , int  typ , int  prot);

#if !defined(hp9000s300) && !defined(M_UNIX)
#define socketpair dbg_socketpair
extern int dbg_socketpair(int  dom , int  typ , int  prot , int  sv[2]);
#endif

#define opendir dbg_opendir
extern DIR *dbg_opendir(char  *name);

#define closedir dbg_closedir
extern void dbg_closedir(DIR  *d);

extern int fdplumb_LogAllFileAccesses;
extern int fdplumb_SpillGuts();
#endif /* PLUMBFDLEAKS */

ENDCPLUSPLUSPROTOS
 
