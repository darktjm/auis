/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#ifndef ATKOS_H
#define ATKOS_H 1

#include <stdio.h> /* needed so that we can declare Andrew_tmpfile. */

#ifdef ATK_IN_RELATIVIZE
extern void ATKUseExportsFiles(char *arg, char *dot, char *slash);
extern void ATKMinimizeLibs(char *arg, char *dot, char *slash);
#endif

BEGINCPLUSPLUSPROTOS
#ifdef NEED_SCANDIR
int scandir(const char *dir, struct dirent ***namelist,
              int (*select)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **a, const struct dirent **b);
#endif

#ifdef NEED_ATKINIFINI
extern void ATK_DoIniFini();
#endif

#ifdef NEED_ANSI_TMPFILES
extern char *Andrew_tmpnam();
extern FILE *Andrew_tmpfile();

#define tmpnam(buf) Andrew_tmpnam(buf)
#define tmpfile() Andrew_tmpfile()

/* the following #defines may be overridden in the system.h file */
#ifndef P_tmpdir
#define P_tmpdir "/tmp/"
#endif
#ifndef L_tmpnam
#define L_tmpnam (sizeof(P_tmpdir)+15)
#endif
#ifndef TMP_MAX
#define TMP_MAX 52 /* maybe this should be larger, this is a conservative guess. */
#endif
#endif /* NEED_ANSI_TMPFILES */

#ifdef NEED_LOCKF
extern int Andrew_lockf(int fid);
extern int Andrew_unlockf(int fid);
#undef osi_ExclusiveLockNoBlock
#define osi_ExclusiveLockNoBlock(fid) Andrew_lockf(fid)
#undef osi_UnLock
#define osi_UnLock(fid) Andrew_unlockf(fid)
#endif

#ifdef NEED_ISINF_USING_FINITE
#define isinf(x) ((!isnan(x)) && (!finite(x)))
#endif

ENDCPLUSPLUSPROTOS
#endif /* ATKOS_H */
