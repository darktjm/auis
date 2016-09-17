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

/*****************************************\

  Chart Programming Guide --  Example #1

\*****************************************/

#include  <andrewos.h>
ATK_IMPL("chartx1app.H")
#include  <im.H>
#include  <frame.H>
#include  <chart.H>
#include  <chartv.H>
#include  <chartx1app.H>


#define  Chart		    (self->chart_data_object)
#define  ChartView	    (self->chart_view_object)
#define  Frame		    (self->framep)
#define  Im		    (self->imp)


ATKdefineRegistry(chartx1app, application, NULL);
static int Query( const char			      *topic );


chartx1app::chartx1app( )
      {
  class chartx1app *self=this;
  (this)->SetMajorVersion(  0 );
  (this)->SetMinorVersion(  0 );
  Chart = new chart;
  ChartView = new chartv;
  THROWONFAILURE( TRUE);
  }

boolean
chartx1app::Start( )
    {
  class chartx1app *self=this;
  boolean	status = TRUE;
  int mortgage, food, insurance, entertainment, savings, education, vacation;

  (this )->application::Start( );
  if ( Chart  &&  ChartView )
    {
    mortgage =	    Query( "Mortgage" );
    food =	    Query( "Food" );
    insurance =	    Query( "Insurance" );
    entertainment = Query( "Entertainment" );
    savings =	    Query( "Savings" );
    education =	    Query( "Education" );
    vacation =	    Query( "Vacation" );

    (Chart)->SetChartAttribute(  chart_Type( "Pie" ) );
    (Chart)->SetChartAttribute(  chart_TitleCaption( "Home Budget" ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Mortgage", 0 ),
	chart_ItemValue( mortgage ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Food", 0 ),
	chart_ItemValue( food ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Insurance", 0 ),
	chart_ItemValue( insurance ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Entertainment", 0 ),
	chart_ItemValue( entertainment ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Savings", 0 ),
	chart_ItemValue( savings ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Education", 0 ),
	chart_ItemValue( education ) );
    (Chart)->SetItemAttribute(  (Chart)->CreateItem(  "Vacation", 0 ),
	chart_ItemValue( vacation ) );
    if((Frame = new frame) == NULL) {
	fprintf(stderr,"Could not allocate enough memory.\nexiting.\n");
	exit(-1);
    }
    (Frame)->SetView(  ChartView );
    if((Im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"Could not create new window.\nexiting.\n");
	exit(-1);
    }
    (Im)->SetView(  Frame );
    (ChartView)->SetDataObject(  Chart );
    (ChartView)->WantInputFocus(  ChartView );
    return  status;
    }
    else 
    {
    printf( "ChartX1: Failed to Create objects\n" );
    return FALSE;
    }
  }


static int Query( const char			      *topic )
    {
  char			      response[255];

  printf( "Enter %s: ", topic );
  fgets( response, sizeof(response), stdin );
  return  atoi( response );
  }
