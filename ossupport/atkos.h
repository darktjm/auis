/* Copyright 1993 Carnegie Mellon University All rights reserved. */
#ifndef ATKOS_H
#define ATKOS_H 1

/** \addtogroup libatkos libatkos.a
 * @{ */

#include <stdio.h> /* needed so that we can declare Andrew_tmpfile. */

extern void ATKUseExportsFiles(char *arg, char *dot, char *slash);
extern void ATKMinimizeLibs(char *arg, char *dot, char *slash);

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

/** @} */

ENDCPLUSPLUSPROTOS
#endif /* ATKOS_H */
