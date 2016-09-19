/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* special chars for rofftext.c
 *
 */

#include <andrewos.h>

#include <rofftext.H>
#include <hash.H>

#define Mangle(self,name,def) (self->SpecialChars)->Store(name,strdup(def))

void InitChars(class rofftext  *self);


void InitChars(class rofftext  *self)
{
    Mangle(self,"em","-");
    Mangle(self,"hy","-");
    Mangle(self,"bu","o");
    Mangle(self,"14","1/4");
    Mangle(self,"12","1/2");
    Mangle(self,"34","3/4");
    Mangle(self,"fi","fi");
    Mangle(self,"fl","fl");
    Mangle(self,"ff","ff");
    Mangle(self,"Fi","ffi");
    Mangle(self,"Fl","ffl");
    Mangle(self,"**","*");
    Mangle(self,"pl","+");
    Mangle(self,"mi","-");
    Mangle(self,"eq","=");
    Mangle(self,"aa","'");
    Mangle(self,"ga","`");
    Mangle(self,"ul","_");
    Mangle(self,"sl","/");
    Mangle(self,"br","|");
    Mangle(self,"or","|");
    Mangle(self,"ci","O");
    Mangle(self,"fm","'");

    /* greek letters */
    Mangle(self,"*a","A");
    Mangle(self,"*b","B");
    Mangle(self,"*g","/");
    Mangle(self,"*d","D");
    Mangle(self,"*e","S");
    Mangle(self,"*z","Q");
    Mangle(self,"*y","N");
    Mangle(self,"*h","O");
    Mangle(self,"*i","i");

    /* math */
    Mangle(self,">=",">=");
    Mangle(self,"<=","<=");
    Mangle(self,"!=","!=");
    Mangle(self,"==","==");
    Mangle(self,"ap","~");
    Mangle(self,"->","->");
    Mangle(self,"<-","<-");
    Mangle(self,"ua","^");
    Mangle(self,"da","v");
    Mangle(self,"mu","x");
    Mangle(self,"di","/");
    Mangle(self,"+-","+-");
    Mangle(self,"no","!");

    /* more to come */
}
