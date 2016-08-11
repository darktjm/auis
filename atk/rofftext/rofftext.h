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


 
/* type checking */
extern SetIndent(struct rofftext  *self,int  u);
extern SetTempIndent(struct rofftext  *self,int  u);
extern Is_BOL(struct rofftext  *self);
extern int  Set_BOL(class rofftext *self);
extern DestroyContext(IC  c);
extern ung(struct rofftext  *self,int  c,Trickle  t);
extern tpush(struct rofftext  *self,Trickle  t,char  *filename,FILE  *f,char  *s,boolean  push,int  argc,char  *argv[]);
extern putregister(struct rofftext  *self,char  *name,int  value,enum RegFmt  fmt,int  inc,boolean  relative);
extern char *getstring(struct rofftext  *self,char  *name);
extern putstring(struct rofftext  *self,char  *name,char  *value);
extern getarg(struct rofftext  *self,Trickle  t,char  *buf,int  n,boolean  copymode);
extern put(struct rofftext  *self,int c);
extern DoBreak(struct rofftext  *self);
extern get(struct rofftext  *self,Trickle  t);
extern Scan(struct rofftext  *self,Trickle  t,char  *cmd);
