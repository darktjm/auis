/* File idltext.C created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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

   idltext, an object for editing SOM IDL code. */

static char ibmid[] = "(c) Copyright IBM Corp.  1988-1995.  All rights reserved.";
static char rcsHeader[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/srctext/RCS/idltext.C,v 2.0 1995/01/27 19:38:38 rr2b Stab74 $";

#include <andrewos.h>
#include <toolcnt.h>

#include "srctext.H"
#include "idltext.H"

static Dict *words[TABLESIZE];


ATKdefineRegistry(idltext, cpptext, idltext::InitializeClass);


boolean idltext::InitializeClass()
{
    static Dict idlkeywords[]={
	{"any",0,KEYWRD},
	{"attribute",0,KEYWRD},
	{"boolean",0,KEYWRD},
	{"case",0,KEYWRD},
	{"char",0,KEYWRD},
	{"class",CLASS_BIT,KEYWRD},
	{"const",0,KEYWRD},
	{"context",0,KEYWRD},
	{"default",0,KEYWRD},
	{"double",0,KEYWRD},
	{"enum",0,KEYWRD},
	{"exception",0,KEYWRD},
	{"FALSE",0,KEYWRD},
	{"float",0,KEYWRD},
	{"implementation",0,KEYWRD},
	{"in",0,KEYWRD},
	{"inout",0,KEYWRD},
	{"interface",CLASS_BIT,KEYWRD},
	{"long",0,KEYWRD},
	{"module",0,KEYWRD},
	{"octet",0,KEYWRD},
	{"oneway",0,KEYWRD},
	{"out",0,KEYWRD},
	{"raises",0,KEYWRD},
	{"readonly",0,KEYWRD},
	{"sequence",0,KEYWRD},
	{"short",0,KEYWRD},
	{"string",0,KEYWRD},
	{"struct",CLASS_BIT,KEYWRD},
	{"switch",0,KEYWRD},
	{"TRUE",0,KEYWRD},
	{"TypeCode",0,KEYWRD},
	{"typedef",0,KEYWRD},
	{"unsigned",0,KEYWRD},
	{"union",0,KEYWRD},
	{"void",0,KEYWRD},
	{NULL,0,0}
    };
    srctext::BuildTable("idltext",::words,idlkeywords);
    return TRUE;
}

idltext::idltext()
{
    ATKinit;
    this->srctext::words= (Dict **)::words;
    ToolCount("EditViews-idltext",NULL);
    THROWONFAILURE(TRUE);
}
