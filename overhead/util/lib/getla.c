/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andyenv.h>
#include <system.h>
#include <fdplumb.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <nlist.h>

#include <util.h>

#if defined(ultrix) && defined(mips)
#include <sys/fixpoint.h>
#endif

#if defined(NeXT) || SY_OS2
#define NOGETLA
#endif

#ifdef NOGETLA 
double getla(int index)
{
    return (double) 0.0;
}
void getla_ShutDown()
{
}
#elif defined(sys_ix86_Linux)
double getla(int index)
{
    double lav[3];
    getloadavg(lav, 3);
    return index < 3 ? lav[index] : 0.0;
}
void getla_ShutDown(void)
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

double getla(int index)
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
