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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/getla.c,v 2.21 1996/02/14 16:14:30 robr Stab74 $";
#endif


 

#include <andyenv.h>
#include <system.h>
#include <fdplumb.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <nlist.h>

#if defined(ultrix) && defined(mips)
#include <sys/fixpoint.h>
#endif

#if defined(NeXT) || SY_OS2 || defined(sys_ix86_Linux)
#define NOGETLA
#endif

#ifdef NOGETLA 
double getla(index)
int index;
{
    return (double) 0.0;
}
getla_ShutDown()
{
}
#else

static struct nlist Nl[] =
{
#ifdef PMAX_ENV
	{ "avenrun" },
#else /* PMAX_ENV */
	{ "_avenrun" },
#endif /* PMAX_ENV */
#define	X_AVENRUN	0
	{ 0 },
};

static int kmem = -1, fpastate = -1, fpacount = 0;

double getla(index)
int index;
{
#ifdef sun
    long avenrun[3];	/* For any kind of sun */
#else /* sun */
#if defined(ultrix) && defined(mips)
    fix avenrun[3];
#else /* mips & ultrix */
    double avenrun[3];
#endif /* mips & ultrix */
#endif /* sun */

    if (kmem < 0) {
	kmem = open("/dev/kmem", O_RDONLY, 0);
	if (kmem < 0) return -1.0;
#ifdef FIOCLEX
	(void) ioctl(kmem, FIOCLEX, 0);
#endif /* FIOCLEX */
	nlist("/vmunix", Nl);
	if (Nl[0].n_type == 0) {
	    close(kmem);
	    kmem = -1;
	    return -1.0;
	}
    }
    if (fpastate < 0 || (fpastate > 0 && fpacount <= 0)) {
	fpastate = (fpacheck() == 0 ? 1 : 0);
	fpacount = 100;	/* Check it every 100 calls */
    }
    if (fpacount >= 0) --fpacount;
    if (fpastate <= 0) return -1.0;	/* Default if the FPA is bad. */

    if (lseek(kmem, (long) Nl[X_AVENRUN].n_value, 0) == -1 ||
	 read(kmem, avenrun, sizeof(avenrun)) < sizeof(avenrun)) return -1.0;

#ifdef SUN_ENV
#ifdef MACH
#define FSCALE (1<<8)
#endif
    return (double) avenrun[index] / FSCALE;
#else /* SUN_ENV */
#if defined(mips) && defined(ultrix)
    return FIX_TO_DBL(avenrun[index]);
#else /* mips & ultrix */
    return avenrun[index];
#endif /* mips & ultrix */
#endif /* SUN_ENV */
}

getla_ShutDown()
{
    if (kmem >= 0) {
	close(kmem);
	kmem = -1;
    }
}

#endif
