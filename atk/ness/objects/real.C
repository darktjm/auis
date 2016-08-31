/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
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

#include <andrewos.h>
#include <math.h>
#include <ness.H>

/* 
	Real function comments from /usr/man/man3/math.3m

	acos(x)		sin.3m		inverse trigonometric functions
	acosh(x)		asinh.3m		inverse hyperbolic functions
	asin(x)		sin.3m		inverse trigonometric functions
	asinh(x)		asinh.3m		inverse hyperbolic functions
	atan(x)		sin.3m		inverse trigonometric functions
	atanh(x)		asinh.3m		inverse hyperbolic functions
	atan2(x, y)	sin.3m		inverse trigonometric functions
	-- cabs(z)	hypot.3m		complex absolute value (not implemented)
	cbrt(x)		sqrt.3m		cube root
	ceil(x) => int	floor.3m		interger no less than
	cos(x)		sin.3m		trigonometric function
	cosh(x)		sinh.3m		hyperbolic function
	erf(x)		erf.3m		error function
	erfc(x)		erf.3m		complementary error function
	exp(x)		exp.3m		exponential
	expm1(x)	exp.3m		exp(x) - 1
	fabs(x)		floor.3m		absolute value
	floor(x) => int	floor.3m		integer no greater than
	hypot(x, y)	hypot.3m		Euclidean distance
	j0(x)		j0.3m		bessel function
	j1(x)		j0.3m		bessel function
	jn(n, x)		j0.3m		bessel function
	lgamma(x)	lgamma.3m	log gamma function
	log(x)		exp.3m		natural logarithm
	log10(x)		exp.3m		logarithm to base 10
	log1p(x)		exp.3m		log(1+x)
	pow(x, y)		exp.3m		exponential x**y
	sin(x)		sin.3m		trigonometric function
	sinh(x)		sinh.3m		hyperbolic functions
	sqrt(x)		sqrt.3m		square root
	tan(x)		sin.3m		trigonometric function
	tanh(x)		sinh.3m		hyperbolic function
	y0(x)		j0.3m		bessel function
	y1(x)		j0.3m		bessel function
	yn(n, x)		j0.3m		bessel function

	round(x) => int
	float1(i) => real
	float2(x, i) => x, real

*/


void realUnary(char  op, unsigned char *iar);
void realOther(char  op, unsigned char *iar);

/* single argument real functions */
	void
realUnary(char  op, unsigned char *iar) {
	union stackelement *NSP = NSPstore;
	if (NSP->d.hdr != dblHdr) 
		RunError(": arg is not a real value (uninitialized ?)", iar);
	switch(op) {
#if !SY_OS2  /* The EMX GNU math lib is currently very weak.  Skip this stuff. */
		case 'a':	NSP->d.v = acos(NSP->d.v);   break;
		case 'c':	NSP->d.v = asin(NSP->d.v);   break;
		case 'e':	NSP->d.v = atan(NSP->d.v);   break;
		case 'i':	NSP->d.v = cos(NSP->d.v);   break;
		case 'j':	NSP->d.v = cosh(NSP->d.v);   break;
		case 'k':	NSP->d.v = erf(NSP->d.v);   break;
		case 'l':	NSP->d.v = erfc(NSP->d.v);   break;
		case 'm':	NSP->d.v = exp(NSP->d.v);   break;
		case 'o':	NSP->d.v = fabs(NSP->d.v);   break;
		case 'r':	NSP->d.v = j0(NSP->d.v);   break;
		case 's':	NSP->d.v = j1(NSP->d.v);   break;
		case 'u':	NSP->d.v = log(NSP->d.v);   break;
		case 'v':	NSP->d.v = log10(NSP->d.v);   break;
		case 'y':	NSP->d.v = sin(NSP->d.v);   break;
		case 'z':	NSP->d.v = sinh(NSP->d.v);   break;
		case 'A':	NSP->d.v = sqrt(NSP->d.v);   break;
		case 'B':	NSP->d.v = tan(NSP->d.v);   break;
		case 'C':	NSP->d.v = tanh(NSP->d.v);   break;
		case 'D':	NSP->d.v = y0(NSP->d.v);   break;
		case 'E':	NSP->d.v = y1(NSP->d.v);   break;
		case '_': NSP->d.v = -(NSP->d.v);   break;

#if ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV)
		case 't':	NSP->d.v = lgamma(NSP->d.v);  break;
#else /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */
		case 't': {
			/* this approximation is derived from Stirling's
			approximation to the gamma function as shown in
			Exercise 6 of Section 1.2.11.2 of Knuth V.1 (2nd ed.)
			The formula when  x >= a > 0 is
			    gamma(x+1) = sqrt(2*pi*x) * (x/e)^x * (1+O(1/x)) 
			I suspect this formula is considerably inaccurate
			for small values of x.  
			It does not hold for negative values.		-wjh
			*/
			double x = NSP->d.v;
			double p = x-1;
			double s = 1 + 1 / (12*p);
			p *= x-1;
			s += 1 / (288*p);
			p *= x-1;
			s -= 139/(51840*p);
			p *= x-1;
			s -= 571 / (248832 * p);
			NSP->d.v = log(2*3.1415926535/2)
				- log(x-1)/2  +  x * log(x-1) + log(s);
		}	break;
#endif /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */

#if ! SY_U5x && ! SY_AIXx
		case 'b':	NSP->d.v = acosh(NSP->d.v);   break;
		case 'd':	NSP->d.v = asinh(NSP->d.v);   break;
		case 'f':	NSP->d.v = atanh(NSP->d.v);   break;
		case 'g':	NSP->d.v = cbrt(NSP->d.v);    break;
		case 'n':	NSP->d.v = expm1(NSP->d.v);   break;
		case 'w':	NSP->d.v = log1p(NSP->d.v);   break;
#else /* ! SYSV && ! AIX */
		case 'b':	/* acosh */
			NSP->d.v = log(NSP->d.v + sqrt(NSP->d.v*NSP->d.v-1.0));
			break;
		case 'd':	/* asinh */
			NSP->d.v = log(NSP->d.v + sqrt(NSP->d.v*NSP->d.v+1.0));
			break;
		case 'f':	/* atanh */
			NSP->d.v = 
				((log(1.0)+NSP->d.v)-(log(1.0)-NSP->d.v))/2.0;
			break;
		case 'g':	/* cbrt */
			NSP->d.v = exp(log(NSP->d.v)/3.0);    break;
		case 'n':	/* expm1 */
			NSP->d.v = exp(NSP->d.v)-1.0;   break;
		case 'w':	/* log1p */
			NSP->d.v = log(1.0+NSP->d.v);   break;
#endif /* (!SYSV && !AIX) */
#endif /* OS/2 */
		default:
			RunError(":unimplemented operation requested", iar);
	}
}

	void
realOther(char  op, unsigned char *iar) {
	union stackelement *NSP = NSPstore;
	double x;
	long l;
	register struct dblstkelt *left ;

	/* check arguments */
	switch (op) {
	case '+':
	case '-':
	case '*':
	case '/':
	case 'a':
	case 'b':
	case 'x':	/* pow(x, y) NSP->d.v = pow(NSP->d.v);   break; */
		/* binary real operations */
		if (NSP->d.hdr != dblHdr)
			RunError(":right operand is not a real value", iar);
		left = &(&(NSP->d))[1];
		if (left->hdr != dblHdr)
			RunError(":left operand is not a real value", iar);
		x = NSP->d.v;
		NSPopSpace(dblstkelt);
		break;
	case  'c':
	case  'd':
	case  'e':
		/* real to integer */
		if (NSP->d.hdr != dblHdr)
			RunError(":operand is not a real value", iar);
		x = NSP->d.v;
		NSPopSpace(dblstkelt);
		NSPushSpace(longstkelt);
		NSP->l.hdr = longHdr;
		break;
	case  'f':
	case  'g':
		/* real to boolean */
		if (NSP->d.hdr != dblHdr)
			RunError(":operand is not a real value", iar);
		x = NSP->d.v;
		NSPopSpace(dblstkelt);
		NSPushSpace(boolstkelt);
		NSP->b.hdr = boolHdr;
		break;
	case 'j':
		/* integer to real */
		if (NSP->l.hdr != longHdr)
			RunError(":operand is not an integer value", iar);
		l = NSP->l.v;
		NSPopSpace(longstkelt);
		NSPushSpace(dblstkelt);
		NSP->d.hdr = dblHdr;
		break;
	case 'k':		/* jn(n, x) */
	case 'l':		/* yn(n, x) */
		/* (integer, real) => real */
		left = &(&(NSP->d))[1];
		if (left->hdr != longHdr)
			RunError(":left operand is not an integer value", iar);
		if (NSP->d.hdr != dblHdr)
			RunError(":right operand is not a real value", iar);
		x = NSP->d.v;
		l = ((struct longstkelt *)left)->v;
		NSPopSpace(longstkelt);  /* pop dbl, pop long, push dbl */
		NSP->d.hdr = dblHdr;
		break;
	}

	/* at this point the operands are in approriate places (x, l, top of stack) and 
		the top stack element is prepared to be the result */
	switch (op) {
	/* binary real operations */
	case '+':
		NSP->d.v = NSP->d.v + x;  break;
	case '-':
		NSP->d.v = NSP->d.v - x;  break;
	case '*':
		NSP->d.v = NSP->d.v * x;  break;
	case '/':
		NSP->d.v = NSP->d.v / x;  break;
	case 'a':
		NSP->d.v = atan2(NSP->d.v, x);  break;
	case 'b':
		NSP->d.v = hypot(NSP->d.v, x);  break;
	case 'x':
		NSP->d.v = pow(NSP->d.v, x);  break;

	/* real to integer */

	/* The casts of (long) actually cause conversion, I HOPE.
	If so they are unlike all other casts, which do not
	cuase conversion but cause direct assignment of the bits.  -wjh */

	case 'c':		/* round(x) */
		NSP->l.v = (long)floor(x+0.5);  break;
	case 'd':
		NSP->l.v = (long)floor(x);  break;
	case 'e':
		NSP->l.v = (long)ceil(x);  break;

	/* real to boolean */
#if ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) && !SY_OS2
	case 'f':
		NSP->b.v = (isnan(x) == 1) ? TRUE : FALSE;   break;

	case 'g':
#ifdef IBM032_ENV
		NSP->b.v = (finite(x) == 1) ? TRUE : FALSE;   break;
#else /* IBM032_ENV */
		/* isinf() is defined on SUN.  Let's assume it works for all
			other than IBM032, VAX, and HPUX */
		NSP->b.v = (! isinf(x) == 1) ? TRUE : FALSE;   break;
#endif /* IBM032_ENV */

#else  /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */
	case 'f':
	case 'g':
		RunError(":isnan and finite are not available", iar);
		break;	
#endif /* ! SY_U5x && ! SY_AIXx && !defined(VAX_ENV) && !defined(PMAX_ENV) */


	/* integer to real */
	case 'j':		/* float(x) */
		NSP->d.v = l;  break;

#if !SY_OS2
	/* (integer, real) => real */
	case 'k':
		NSP->d.v = jn(l, x);  break;
	case 'l':
		NSP->d.v = yn(l, x);  break;
#endif
	default:
		RunError(":unimplemented operation requested", iar);
	}
}
