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

%start tiptopsexp

%{

#include <eli.h>

long            atol();
EliSym_t      *symptr;
EliCons_t     *consptr, *consptr2;
EliStr_t      *strptr;
EliSexp_t     *nodeptr;
int             noerr;

%}

%union {
    EliSexp_t     *nodetype;
}

%token <nodetype> INTEGER SYMBOL LPAREN RPAREN SQUOTE STRING
%type <nodetype> topsexp sexp atom list sexps special quote

%%

tiptopsexp : { noerr = TRUE; EliProcessInfo.u_wrap = FALSE; } topsexp { YYACCEPT; }
           ;

topsexp :   sexp
		{
		    EliProcessInfo.u_parseval = $1;
		}
	|   error
		{
		    if (!noerr)
		        EliError(EliProcessInfo.curglobs, ELI_ERR_BAD_SYNTAX, NULL, "INTERNAL [eliyyparse (syntax error)]");
		}
	;

sexp	:   atom
		{
		    $$ = $1;
		}
	|   list
		{
		    $$ = $1;
		}
	|   special
		{
		    $$ = $1;
		}
	;

atom	:   INTEGER
		{
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			YYERROR;			/* Is this the right thing to do? */
		    EliSexp_SetInt(EliProcessInfo.curglobs, $$, atol(EliProcessInfo.yparsebuf));
		}
	|   SYMBOL
		{
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			YYERROR;
		    if (!(symptr = EliFindSym(EliProcessInfo.curglobs, EliProcessInfo.yparsebuf))) {
			if (!(symptr = eliSymTab_FindOrMake(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_tmptab, EliProcessInfo.yparsebuf)))
			    YYERROR;
			else
			    eliSym_SetScope(symptr, e_sym_known);
		    }
		    EliSexp_SetSym(EliProcessInfo.curglobs, $$, symptr);
		}
	|   STRING
		{
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    strptr = eliStringTable_FindOrMake(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_strtab,
			    EliParseStr(EliProcessInfo.yparsebuf));
		    if (EliErr_ErrP(EliProcessInfo.curglobs))
		    	YYERROR;
		    EliSexp_SetStr(EliProcessInfo.curglobs, $$, strptr);
		}
	;

list	:   LPAREN sexps RPAREN
		{
		    $$ = $2;
		    if (e_data_none == $2->data.type) {
			if (!(consptr = eliCons_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			    YYERROR;
			EliSexp_SetCons(EliProcessInfo.curglobs, $$, consptr);
		    }
		}
	;

sexps	:
		{
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		}
	|   sexps sexp
		{
		    $$ = $1;
		    if (e_data_none == $1->data.type) {
			if (!(consptr = eliCons_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			    YYERROR;
			EliCons_BindCar(EliProcessInfo.curglobs, consptr, $2);
			EliSexp_SetCons(EliProcessInfo.curglobs, $$, consptr);
		    }
		    else {
			consptr = EliLastCell($1->data.datum.consval);
			if (!(consptr2 = eliCons_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			    YYERROR;
			if (!(nodeptr = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
			    YYERROR;
			EliSexp_SetCons(EliProcessInfo.curglobs, nodeptr, consptr2);
			EliCons_BindCdr(EliProcessInfo.curglobs, consptr, nodeptr);
			EliCons_BindCar(EliProcessInfo.curglobs, consptr2, $2);
		    }
		}
	;

special	:   quote sexp
		{
		    if (!(consptr = eliCons_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    if (!(consptr2 = eliCons_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    EliCons_BindCar(EliProcessInfo.curglobs, consptr, $1);
		    if (!(nodeptr = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    EliSexp_SetCons(EliProcessInfo.curglobs, nodeptr, consptr2);
		    EliCons_BindCdr(EliProcessInfo.curglobs, consptr, nodeptr);
		    EliCons_BindCar(EliProcessInfo.curglobs, consptr2, $2);
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    EliSexp_SetCons(EliProcessInfo.curglobs, $$, consptr);
		}
	;

quote	:    SQUOTE
		{
		    if (!($$ = eliSexp_GetNew_trace(EliProcessInfo.curglobs, EliProcessInfo.curglobs->g_errstk)))
		    	YYERROR;
		    EliSexp_SetSym(EliProcessInfo.curglobs, $$, EliProcessInfo.curglobs->g_quoteptr);
		}
	;

%%
static char *eliy_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/eli/lib/RCS/eliy.y,v 2.4 1992/12/17 01:13:14 rr2b Stab74 $";


extern yylex();