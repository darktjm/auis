/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/*
    $Log: calc.C,v $
 * Revision 1.3  1995/11/07  20:17:10  robr
 * OS/2 port
 *
 * Revision 1.2  1993/05/29  16:34:26  rr2b
 * Removed ::s before macros and local variables.
 * Replaced self with this in the macro Value since it is only used in methods.
 * Fixed the sscanf and sprintf format to be "%lf" instead of "%F".
 * Also removed dependence on the return value from sprintf.
 * Added cast on the read and write fptrs. for Read and WriteObject.
 *
 * Revision 1.1  1993/05/29  15:33:56  rr2b
 * Initial revision
 *
 * Revision 1.3  1992/12/15  21:48:53  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.2  1992/12/14  21:04:49  rr2b
 * disclaimerization
 *
 * Revision 1.1  1992/10/05  20:07:19  susan
 * Initial revision
 *
 * Revision 1.4  1991/09/12  16:02:50  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.3  1989/08/04  17:08:38  tom
 * Accept keyboard inputs;
 * Suppress Shrink & Help Icons.
 *
 * Revision 1.2  89/05/26  18:24:20  tom
 * Use apt_LeftArea, etc.
 * 
 * Revision 1.1  89/05/10  20:58:06  tom
 * Initial revision
 * 
*/

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Calc Data-object

MODULE	calc.c

VERSION	1.0

DESCRIPTION
	This is the suite of Methods that suport the Calc Data-object,
	a trivial example of an ATK Inset.

HISTORY
  02/23/88	Created (TCP)
  05/26/89	Use apt_LeftArea, etc (TCP)

END-SPECIFICATION  ************************************************************/

#include <andrewos.h>
#include "dataobject.H"
#include "apt.H"
#include "calc.H"

#define  Value			      this->value



ATKdefineRegistry(calc, apt, NULL);
static void Reader( class calc	    	      *self );
static void Writer( class calc		      *self );


calc::calc( )
      {
  IN(calc_InitializeObject);
  DEBUGst(RCSID,rcsidcalc);
  (this)->SetAreaTitleFontName(  "AndySans22b", apt_TopArea );
  (this)->SetAreaLegendFontName(  "Andy20i", apt_BottomArea );
  Value = 0;
  OUT(calc_InitializeObject);
  THROWONFAILURE( TRUE);
  }

calc::~calc( )
      {
  IN(calc_FinalizeObject);
  DEBUGst(RCSID,rcsidcalc);
  /* Nop */
  OUT(calc_FinalizeObject);
  }

const char *
calc::ViewName( )
    {
  IN(calc_ViewName);
  OUT(calc_ViewName);
  return "calcv";
  }

void
calc::SetValue( double		        value )
      {
  IN(calc::SetValue);
  Value = value;
  (this )->SetModified( );
  OUT(calc::SetValue);
  }

static
void Reader( class calc	    	      *self )
    {
  struct apt_field	     *field;

  IN(Reader);
  while ( ( field = (self )->ReadObjectField( ) ) )
    {
    if ( strcmp( "Value", field->name ) == 0 )
       sscanf( field->content, "%lf", &self->value );
    }
  OUT(Reader);
  }

long
calc::Read( FILE			      *file, long			       id )
        {
  long			      status; 

  IN(calc::Read);
  if ( (status = (this)->ReadObject(  file, id, (apt_readfptr)Reader )) ==
	dataobject_NOREADERROR )
    {
    (this)->NotifyObservers(  calc_value_changed );
    }
  OUT(calc::Read);
  return status;
  }

static
void Writer( class calc		      *self )
    {
  struct apt_field		      field;
  char				      value[25];

  IN(Writer);
  field.name = "Value";
  sprintf( value, "%lf", self->value );
  field.content = value;
  (self)->WriteObjectField(  &field );
  OUT(Writer);
  }

long
calc::Write( FILE			      *file, long			       id, int			       level )
          {
  IN(calc_Write);
  (this)->WriteObject(	file, id, level, (apt_writefptr)Writer );
  OUT(calc_Write);
  return  id;
  }
