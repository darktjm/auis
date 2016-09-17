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


 
/* type checking */
extern int SetIndent(struct rofftext  *self,int  u);
extern int SetTempIndent(struct rofftext  *self,int  u);
extern int Is_BOL(struct rofftext  *self);
extern int  Set_BOL(class rofftext *self);
extern int DestroyContext(IC  c);
extern int ung(struct rofftext  *self,int  c,Trickle  t);
extern int tpush(struct rofftext  *self,Trickle  t,const char  *filename,FILE  *f,const char  *s,boolean  push,int  argc,const char  * const argv[]);
extern int putregister(struct rofftext  *self,const char *name,int  value,enum RegFmt  fmt,int  inc,boolean  relative);
extern const char *getstring(struct rofftext  *self,const char *name);
extern int putstring(struct rofftext  *self,const char *name,const char *value);
extern int getarg(struct rofftext  *self,Trickle  t,char  *buf,int  n,boolean  copymode);
extern int put(struct rofftext  *self,int c);
extern int DoBreak(struct rofftext  *self);
extern int get(struct rofftext  *self,Trickle  t);
extern int Scan(struct rofftext  *self,Trickle  t,const char  *cmd);
