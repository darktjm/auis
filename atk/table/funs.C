/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/

/*
 * %%%% the need to undefine _C_func appears to be a bug in AIX...pgc
 */
#if SY_AIX221
#ifdef _C_func
#undef _C_func
#endif /* ifdef _C_func */
#endif /* if SY_AIX221 */

#include <andrewos.h> /* sys/time.h */

#include <math.h>

#define AUXMODULE
#include <table.H>

#include <shared.h>

#define e_TRUE 1.0
#define e_FALSE 0.0

NO_DLL_EXPORT extern const int daysinmonth[];


#define false myfalse
#define true mytrue
int isrange (extended_double  *x);
static void getrow (class table  * T, extended_double  *result, int      r , int      c, int      argc, extended_double  *argv);
static void getcol (class table  * T, extended_double  *result, int      r , int      c, int      argc, extended_double  *argv);
static void fsum (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static void fcount (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static void vlookup (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static double   iffer (double   x , double   y , double   z);
static double   false ();
static double true ();
static double frand ();
static double fnot (double   x);
static double fand (double   x , double   y);
static double orf (double   x , double   y);
static void fiserr (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static void fisinf (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
int     idate (int      y, int      m, int      d);
static void fdate (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static double   fday (double   fdate);
static double   fmonth (double   fdate);
static double fyear (double   fdate);
static void errorfunc (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv);
static double   fmodulo (double   x , double   y);
static double   fpi ();
static double fround (double   x , double   yy);
static double   today ();
void enterfuns ();


int isrange (extended_double  *x)
{
    return (IsBogus(x) && strcmp (ExtractBogus(x), "range") == 0);
}

static void getrow (class table  * T, extended_double  *result, int      r , int      c, int      argc, extended_double  *argv)
{
    if (argc != 0)
	MakeBogus(result, "No args expected");
    else
	MakeStandard(result, (double) (r + 1));
}

static void getcol (class table  * T, extended_double  *result, int      r , int      c, int      argc, extended_double  *argv)
{
    if (argc != 0)
	MakeBogus(result, "No args expected");
    else
	MakeStandard(result, (double) (c + 1));
}

static void fsum (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{
    double  x = 0.0;
    extended_double *p = argv;

    while (p - argv < argc)
	if (isrange(p)) {
	    int     r, c;
	    struct chunk chunk;
	    p++;
	    chunk.TopRow = (int) (StandardValue(p++));
	    chunk.LeftCol = (int) (StandardValue(p++));
	    chunk.BotRow = (int) (StandardValue(p++));
	    chunk.RightCol = (int) (StandardValue(p++));
	    rangeLimit (T, &chunk);
	    for (r = chunk.TopRow; r <= chunk.BotRow; r++)
		for (c = chunk.LeftCol; c <= chunk.RightCol; c++) {
		    if (!(T)->IsJoinedToAnother( r-1, c-1)) {
			rcref (T, result, r, c, 0);
			if (!IsStandard(result))
			    return;
			x += StandardValue(result);
		    }
		}
	}
	else {
	    if (!IsStandard(p)) {
		*result = *p;
		return;
	    }
	    x += StandardValue(p);
	    p++;
	}
    MakeStandard(result, x);
}

static void fcount (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{
    int     x = 0;
    extended_double *p = argv;

    while (p - argv < argc)
	if (isrange(p)) {
	    int     r, c;
	    struct chunk chunk;
	    p++;
	    chunk.TopRow = (int) (StandardValue(p++));
	    chunk.LeftCol = (int) (StandardValue(p++));
	    chunk.BotRow = (int) (StandardValue(p++));
	    chunk.RightCol = (int) (StandardValue(p++));
	    rangeLimit (T, &chunk);
	    for (r = chunk.TopRow; r <= chunk.BotRow; r++) {
		for (c = chunk.LeftCol; c <= chunk.RightCol; c++) {
		    if (!(T)->IsJoinedToAnother( r-1, c-1) && (T)->GetCell( r-1, c-1)->celltype == table_ValCell)
			x++;
		}
	    }
	}
	else {
	    if (IsStandard(p))
		x++;
	    p++;
	}
    MakeStandard(result, (double) (x));
}

#if 0 /* unused??? need to find out why standard fmin/fmax used instead of this - tjm */
static void fmax (class table  * T, extended_double  *result, int      rr, int      cc, int      argc, extended_double  *argv)
{
    double  x = 0;
    int     any = 0;
    extended_double *p = argv;

    while (p - argv < argc)
	if (isrange(p)) {
	    int     r, c;
	    struct chunk chunk;
	    p++;
	    chunk.TopRow = (int) (StandardValue(p++));
	    chunk.LeftCol = (int) (StandardValue(p++));
	    chunk.BotRow = (int) (StandardValue(p++));
	    chunk.RightCol = (int) (StandardValue(p++));
	    rangeLimit (T, &chunk);
	    for (r = chunk.TopRow; r <= chunk.BotRow; r++)
		for (c = chunk.LeftCol; c <= chunk.RightCol; c++) {
		    if (!(T)->IsJoinedToAnother( r-1, c-1)) {
			rcref (T, result, r, c, 0);
			if (!IsStandard(result))
			    return;
			if (any++)
			    x = StandardValue(result) > x ? StandardValue(result) : x;
			else
			    x = StandardValue(result);
		    }
		}
	}
	else {
	    if (!IsStandard(p)) {
		*result = *p;
		return;
	    }
	    if (any++)
		x = StandardValue(p) > x ? StandardValue(p) : x;
	    else
		x = StandardValue(p);
	    p++;
	}
    if (any)
	MakeStandard(result, x);
    else
	eval(T, result, rr, cc, "-1/0");
}

static void fmin (class table  * T, extended_double  *result, int      rr, int      cc, int      argc, extended_double  *argv)
{
    double  x = 0;
    int     any = 0;
    extended_double *p = argv;

    while (p - argv < argc)
	if (isrange(p)) {
	    int     r, c;
	    struct chunk chunk;
	    p++;
	    chunk.TopRow = (int) (StandardValue(p++));
	    chunk.LeftCol = (int) (StandardValue(p++));
	    chunk.BotRow = (int) (StandardValue(p++));
	    chunk.RightCol = (int) (StandardValue(p++));
	    rangeLimit (T, &chunk);
	    for (r = chunk.TopRow; r <= chunk.BotRow; r++)
		for (c = chunk.LeftCol; c <= chunk.RightCol; c++) {
		    if (!(T)->IsJoinedToAnother( r-1, c-1)) {
			rcref (T, result, r, c, 0);
			if (!IsStandard(result))
			    return;
			if (any++)
			    x = StandardValue(result) < x ? StandardValue(result) : x;
			else
			    x = StandardValue(result);
		    }
		}
	}
	else {
	    if (!IsStandard(p)) {
		*result = *p;
		return;
	    }
	    if (any++)
		x = StandardValue(p) < x ? StandardValue(p) : x;
	    else
		x = StandardValue(p);
	    p++;
	}
    if (any)
	MakeStandard(result, x);
    else
	eval(T, result, rr, cc, "1/0");
}
#endif

static void vlookup (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{
    double  x;
    extended_double *p = argv;
    int     r;
    struct chunk chunk;
    if (argc != 6 || !isrange(&argv[1]))
	syntaxError (/* "bad arguments" */);
    x = StandardValue(p++);
    p++;
    chunk.TopRow = (int) (StandardValue(p++));
    chunk.LeftCol = (int) (StandardValue(p++));
    chunk.BotRow = (int) (StandardValue(p++));
    chunk.RightCol = (int) (StandardValue(p++));
    rangeLimit (T, &chunk);
    for (r = chunk.BotRow; r >= chunk.TopRow; r--) {
	if (!(T)->IsJoinedToAnother( r-1, chunk.LeftCol-1)) {
	    if ((T)->IsJoinedToAnother( r-1, chunk.RightCol-1)) {
		MakeBogus(result, "LOOKUP!");
		return;
	    }
	    rcref (T, result, r, chunk.LeftCol, 0);
	    if (!IsStandard(result))
		return;
	    if (StandardValue(result) <= x) {
		rcref (T, result, r, chunk.RightCol, 1);
		return;
	    }
	}
    }
    MakeBogus(result, "LOOKUP!");
}

static double   iffer (double   x , double   y , double   z)
{
    return x ? y : z;
}

static double   false ()
{
    return e_FALSE;
}

static double true ()
{
    return e_TRUE;
}

static double frand ()
{
    return (double) (RANDOM () & (0x1000000 - 1)) / (float) 0x1000000;
}

static double fnot (double   x)
{
    return x ? e_FALSE : e_TRUE;
}

static double fand (double   x , double   y)
{
    return (x != 0 && y != 0);
}

static double orf (double   x , double   y)
{
    return (x != 0 || y != 0);
}

static void fiserr (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{

    if (argc != 1) {
	MakeBogus(result, "Wrong # of parameters");
	return;
    }
    if (IsBogus(&argv[0])) {
	MakeStandard(result, e_TRUE);
	return;
    }
    double val=StandardValue(&argv[0]);
    if(isnan(val))
	MakeStandard(result, e_TRUE);
    else
	MakeStandard(result, e_FALSE);
}

static void fisinf (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{
    if (argc != 1) {
	MakeBogus(result, "Wrong # of parameters");
	return;
    }
    if (IsBogus(&argv[0])) {
	MakeStandard(result, e_FALSE);
	return;
    }
    double val=StandardValue(&argv[0]);
    if (isinf(val))
	MakeStandard(result, e_TRUE);
    else
	MakeStandard(result, e_FALSE);
}

int     idate (int      y, int      m, int      d)
{
    int     leapyear = 0;
    int     ans,
            i;
    y -= 1900;
    m--;
    d--;
    if (y < 0 || y > 199)
	return 0;
    if (m < 0 || m >= 12)
	return 0;
    i = y >> 2;
    ans = i * 1461;
    i += i;
    y = y - (i + i);
    leapyear = (!y);
    if (y) {
	ans += 366;
	while (--y)
	    ans += 365;
    }
    if (d < 0 || d >= daysinmonth[m] + (m==1 ? leapyear : 0)) {
	return 0;
    }
    while (m--)
	ans += daysinmonth[m] + (m==1 ? leapyear : 0);
    ans += d;
    return ans + (ans < 59);
}

static void fdate (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)
{
    int     y, m, d;
    int     leapyear = 0;
    int     ans, i;

    if (argc != 3) {
	MakeBogus(result, "Wrong # of parameters");
	return;
    }
    if (!IsStandard(&argv[0])) {
	*result = argv[0];
	return;
    }
    if (!IsStandard(&argv[1])) {
	*result = argv[1];
	return;
    }
    if (!IsStandard(&argv[2])) {
	*result = argv[2];
	return;
    }

    y = ((int) (StandardValue(&argv[0]) + 0.5)) - 1900;
    m = ((int) (StandardValue(&argv[1]) + 0.5)) - 1;
    d = ((int) (StandardValue(&argv[2]) + 0.5)) - 1;
    
    if (y < 0 || y > 199) {
	MakeBogus(result, "year out of range");
	return;
    }
    if (m < 0 || m >= 12) {
	MakeBogus(result, "month out of range");
	return;
    }
    i = y >> 2;
    ans = i * 1461;
    i += i;
    y = y - (i + i);
    leapyear = (!y);
    if (y) {
	ans += 366;
	while (--y)
	    ans += 365;
    }
    if (d < 0 || d >= daysinmonth[m] + (m == 1 ? leapyear : 0)) {
	MakeBogus(result, "day out of range");
	return;
    }
    while (m--)
	ans += daysinmonth[m] + (m == 1 ? leapyear : 0);
    ans += d;
    MakeStandard(result, (double) (ans + (ans < 59)));
}

static double   fday (double   fdate)
{
    int     date;
    int     m;

    date = (int) (fdate + 0.5);
    date--;
    if (date > 58)
	date++;
    date = date % 1461;
    if (date == 59)
	return 29.0;
    else {
	if (date > 59)
	    date--;
	date = date % 365;
	for (m = 0; date >= 0; m++)
	    date -= daysinmonth[m];
	return (double) (date + daysinmonth[m - 1] + 1);
    }
}

static double   fmonth (double   fdate)
{
    int     date;
    int     m;

    date = (int) (fdate + 0.5);
    date--;
    if (date > 58)
	date++;
    date = date % 1461;
    if (date == 59)
	return 2.0;
    else{
	if (date > 59)
	    date--;
	date = date % 365;
	for (m = 0; date >= 0; m++)
	    date -= daysinmonth[m];
	return (double) (m);
    }
}

static double fyear (double   fdate)
{
    int     date;
    int     y;

    date = (int) (fdate + 0.5);
    date--;
    if (date > 58)
	date++;
    y = date / 1461;
    date -= y * 1461;
    y = y + y;
    y = y + y;
    if (date > 59)
	date--;
    y += date / 365;
    return (double) (y + 1900);
}

static void errorfunc (class table  * T, extended_double  *result, int      rr , int      cc, int      argc, extended_double  *argv)

{
    MakeBogus(result, "ERROR!");
}

static double   fmodulo (double   x , double   y)
{
    return x - y * floor (x / y);
}

static double   fpi ()
{
    return M_PI;
}

static double fround (double   x , double   yy)
{
    int     y;
    double  p;

    y = ((int) (yy + 1000.5)) - 1000;
    for (p = 1.0; y > 0; y--)
	p *= 10.0;
    for (; y < 0; y++)
	p /= 10.0;
    return floor (x * p + 0.5) / p;
}

static double   today ()
{
    struct timeval  tv;

    gettimeofday (&tv, 0);
    return (double) (25568 + tv.tv_sec / 86400);
}

void enterfuns () {
    enterfun("r", (double  (*)())getrow, -1);
    enterfun("c", (double  (*)())getcol, -1);
    enterfun("abs", (double  (*)())fabs, 1);
    enterfun("floor", (double  (*)())floor, 1);
    enterfun("ceil", (double  (*)())ceil, 1);
    enterfun("exp", (double  (*)())exp, 1);
    enterfun("ln", (double  (*)())log, 1);
    enterfun("log", (double  (*)())log10, 1);
    enterfun("sqrt", (double  (*)())sqrt, 1);
    enterfun("sin", (double  (*)())sin, 1);
    enterfun("cos", (double  (*)())cos, 1);
    enterfun("asin", (double  (*)())asin, 1);
    enterfun("acos", (double  (*)())acos, 1);
    enterfun("atan", (double  (*)())atan, 1);
    enterfun("atan2", (double  (*)())atan2, 2);
    enterfun("if", (double  (*)())iffer, 3);
    enterfun("false", (double  (*)())false, 0);
    enterfun("true", (double  (*)())true, 0);
    enterfun("rand", (double  (*)())frand, 0);
    enterfun("not", (double  (*)())fnot, 1);
    enterfun("iserr", (double  (*)())fiserr, -1);
    enterfun("isinf", (double  (*)())fisinf, -1);
    enterfun("and", (double  (*)())fand, 2);
    enterfun("or", (double  (*)())orf, 2);
    enterfun("date", (double  (*)())fdate, -1);
    enterfun("day", (double  (*)())fday, 1);
    enterfun("month", (double  (*)())fmonth, 1);
    enterfun("year", (double  (*)())fyear, 1);
    enterfun("error", (double  (*)())errorfunc, -1);
    enterfun("mod", (double  (*)())fmodulo, 2);
    enterfun("pi", (double  (*)())fpi, 0);
    enterfun("round", (double  (*)())fround, 2);
    enterfun("today", (double  (*)())today, 0);
    enterfun("sum", (double  (*)())fsum, -1);
    enterfun("count", (double  (*)())fcount, -1);
    enterfun("max", (double  (*)())(double  (*)(double, double))fmax, -1);
    enterfun("min", (double  (*)())(double  (*)(double, double))fmin, -1);
    enterfun("vlookup", (double  (*)())vlookup, -1);
}
