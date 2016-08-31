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

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart Data-object

MODULE	chart.c

VERSION	0.0

AUTHOR	TC Peters
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that suport the Chart Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  02/23/88	Created (TCP)
  05/04/89	Add Line chart (TCP)
  05/23/89	Use apt_LeftArea, etc (TCP)
  05/31/89	Add classID parameter in FinalizeObject (TCP)
		Fix DestroyItem (free dumping)
  06/02/89	Set Item Position by default (TCP)
  09/01/89	Upgrade to V1.0

END-SPECIFICATION  ************************************************************/

#include  <andrewos.h>
ATK_IMPL("chart.H")
#include  <rect.h>
#include  <dataobject.H>
#include  <apt.H>
#include  <apts.H>
#include  <chart.H>

#define  ChartTitle		     (self->chart_title)
#define  ChartTitleDataObjectName    (self->chart_title_data_object_name)
#define  ChartTitleViewObjectName    (self->chart_title_view_object_name)
#define  ChartFileName		     (self->chart_file_name)
#define  ChartType		     (self->chart_moniker)
#define  ChartMonikers		     (self->chart_monikers)

#define  ClientAnchor		     (self->client_anchor)
#define  ClientDatum		     (self->client_datum)
#define  ExceptionCode		     (self->exception_code)
#define  ExceptionItem		     (self->exception_item)

#define  ItemAnchor		     (self->item_anchor)
#define  ItemCount		     (self->item_count)
#define  ItemValueRangeLow	     (self->item_value_range_low)
#define  ItemValueRangeInterval	     (self->item_value_range_interval)
#define  ItemValueRangeHigh	     (self->item_value_range_high)
#define  ItemValueGreatest	     (self->item_value_greatest)
#define  ItemValueLeast		     (self->item_value_least)
#define  ItemValueSpan		     (self->item_value_span)
#define	 ITEMFONTNAME		     (self->item_font_name)

#define  ItemName(x)		     ((x)->name)
#define  ItemDatum(x)		     ((x)->datum)
#define  ItemValue(x)		     ((x)->value)
#define  ItemPosition(x)	     ((x)->position)
#define  NextItem(x)		     ((x)->next)


boolean chart_debug = 0;


ATKdefineRegistry(chart, apt, NULL);
static long SetChartAttribute( register class chart		    *self, register long			     attribute , register long			     value );
static long SetItemAttribute( register class chart       *self, register struct chart_item  *item, register long		       attribute , register long		       value );
static char * Extract_Field_Value( register class chart		      *self, register const char			     **fields, register const char			      *name );
static void Reader( register class chart	    	      *self );
static void Parse_Name_Field( register class chart		      *self, register const char			      *string );
static void Parse_Type_Field( register class chart		      *self, register const char			      *string );
static void Parse_Item_Field( register class chart		      *self, register const char			      *string );
static char * ValueString( register class chart		      *self, register struct chart_item	      *item );
static void Writer( register class chart		      *self );
static void SetItemValue( register class chart		      *self, register struct chart_item	      *item, register long			       value );
static int Sort_By_Ascending_Value( register struct chart_item	     **a , register struct chart_item	     **b );
static int Sort_By_Descending_Value( register struct chart_item	     **a , register struct chart_item	     **b );
static int Sort_By_Ascending_Label( register struct chart_item	     **a , register struct chart_item	     **b );
static int Sort_By_Descending_Label( register struct chart_item	     **a , register struct chart_item	     **b );
static int Sort_By_Ascending_Position( register struct chart_item	     **a , register struct chart_item	     **b );
static int Sort_By_Descending_Position( register struct chart_item	     **a , register struct chart_item	     **b );


class chart *
chart::Create( chart_Specification		      *specification, char  *anchor )
        {
  register class chart	     *self;

  IN(chart_Create);
  self = new chart;
  ClientAnchor = (class dataobject *) anchor;
  while ( specification  &&  specification->attribute )
    {
    ::SetChartAttribute( self, specification->attribute, specification->value );
    specification++;
    }

  OUT(chart_Create);
  return self;
  }

chart::chart( )
      {
  class chart *self=this;
#define MAX_INTEGER 400000000 /*===*/
  IN(chart_InitializeObject);
  DEBUGst(RCSID,rcsidchart);
  (this)->SetAreaTitleFontName(  "AndySans16b", apt_TopArea );
  (this)->SetAreaLegendFontName(  "Andy8", apt_BottomArea );
  ChartTitle = ChartFileName = NULL;
  ChartTitleDataObjectName = ChartTitleViewObjectName = NULL;
  ChartType = NULL;
  apts::CaptureString( "Histogram", &ChartType );
  ChartMonikers = NULL;
  ClientAnchor = NULL;
  ClientDatum = 0;
  ItemAnchor = NULL;
  ItemCount = 0;
  ItemValueRangeLow = MAX_INTEGER;
  ItemValueRangeHigh = -MAX_INTEGER;
  ItemValueSpan = 0;
  ItemValueRangeInterval = 1;
  ItemValueGreatest = ItemValueLeast = 0;
  ITEMFONTNAME= NULL;
  OUT(chart_InitializeObject);
  THROWONFAILURE( TRUE);
  }

chart::~chart( )
      {
  class chart *self=this;
  register struct chart_item	      *item = ItemAnchor, *next;

  IN(chart_FinalizeObject);
  if ( ChartTitle )		    free( ChartTitle );
  if ( ChartFileName )		    free( ChartFileName );
  if ( ChartTitleDataObjectName )   free( ChartTitleDataObjectName );
  if ( ChartTitleViewObjectName )   free( ChartTitleViewObjectName );
  while ( item )
    {
    next = item->next;
    (this)->DestroyItem(  item );
    item = next;
    }
  OUT(chart_FinalizeObject);
  }

const char *
chart::ViewName( )
    {
  IN(chart_ViewName);
  OUT(chart_ViewName);
  return "chartv";
  }

long
chart::SetChartAttribute( long attribute, long value )
{  return  ::SetChartAttribute( this, attribute, value );  }

static
long SetChartAttribute( register class chart		    *self, register long			     attribute , register long			     value )
      {
  register long			    status = ok;

  IN(SetChartAttribute);
  switch ( attribute )
    {
    case  chart_datum:
      ClientDatum = value;				break;
    case  chart_filename:
      apts::CaptureString( (char *) value, &ChartFileName );	break;
    case  chart_titlecaption:
      apts::CaptureString( (char *) value, &ChartTitle );		break;
    case  chart_titledataobjectname:
      apts::CaptureString( (char *) value, &ChartTitleDataObjectName );break;
    case  chart_titleviewobjectname:
      apts::CaptureString( (char *) value, &ChartTitleViewObjectName );break;
    case  chart_type:
      apts::CaptureString( (char *) value, &ChartType );		break;
    default:
      fprintf( stderr, "Chart: Unrecognized ChartAttribute (%ld) -- Ignored\n", attribute );
    }
  OUT(SetChartAttribute);
  return(status);
  }

long
chart::ChartAttribute( register long		       attribute )
      {
  class chart *self=this;
  register long		      value = 0;

  IN(chart_ChartAttribute);
  switch ( attribute )
    {
    case  chart_datum:
      value = (long) ClientDatum;			break;
    case  chart_filename:
      value = (long) ChartFileName;			break;
    case  chart_titlecaption:
      value = (long) ChartTitle;			break;
    case  chart_titledataobjectname:
      value = (long) ChartTitleDataObjectName;		break;
    case  chart_titleviewobjectname:
      value = (long) ChartTitleViewObjectName;		break;
    case  chart_type:
      value = (long) ChartType;				break;
    default:
      ExceptionCode = chart_UnknownChartAttribute;
      fprintf( stderr, "Chart: Unrecognized ChartAttribute (%ld) -- Ignored\n", attribute );
    }
  OUT(chart_ChartAttribute);
  return  value;
  }

long
chart::SetItemAttribute( class chart_item *item, long attribute, long value )
{  return  ::SetItemAttribute( this, item, attribute, value );  }

static
long SetItemAttribute( register class chart       *self, register struct chart_item  *item, register long		       attribute , register long		       value )
        {
  register long		      status = ExceptionCode = ok;

  IN(SetItemAttribute);
  DEBUGdt(Attribute,attribute);
  DEBUGdt(Value,value);
  if ( item )
    switch ( attribute )
      {
      case  chart_itemdatum:
	ItemDatum(item) = value;				    break;
      case  chart_itemname:
	apts::CaptureString( (char *) value, &ItemName(item) );		    break;
      case  chart_itemposition:
/*===*/  break;
      case  chart_itemvalue:
	SetItemValue( self, item, value );			    break;
/*===*/
      default:
        status = ExceptionCode = chart_UnknownItemAttribute;
	fprintf( stderr, "Chart: Unknown Item Attribute (%ld) -- Ignored\n", attribute );
      }
    else  status = ExceptionCode = chart_NonExistentItem;
  OUT(SetItemAttribute);
  return(status);
  }

long
chart::ItemAttribute( register struct chart_item  *item, register long		       attribute )
        {
  class chart *self=this;
  register long		      value = 0;

  IN(chart_ItemAttribute);
  if ( item )
    switch ( attribute )
      {
      case  chart_itemdatum:
	value = (long) ItemDatum(item);		    break;
      case  chart_itemname:
	value = (long) ItemName(item);		    break;
      case  chart_itemposition:
	value = (long) ItemPosition(item);	    break;
      case  chart_itemvalue:
	value = (long) ItemValue(item);		    break;
/*===*/
      default:
        ExceptionCode = chart_UnknownItemAttribute;
	fprintf( stderr, "Chart: Unknown Item Attribute (%ld) -- Ignored\n", attribute );
      }
  OUT(chart_ItemAttribute);
  return  value;
  }

void
chart::SetDebug( boolean		        state )
      {
  IN(chart_SetDebug);
  chart_debug = state;
/*===  super_SetDebug( self, debug );*/
  OUT(chart_SetDebug);
  }

static char *
Extract_Field_Value( register class chart		      *self, register const char			     **fields, register const char			      *name )
        {
  register char			     *field = NULL, *t;
  const char *s;
  register long			      length;
  char				      mask[257];

  IN(Extract_Field_Value);
  DEBUGst(Name,name);
  sprintf( mask, "%s(", name );
  length = strlen( mask );
  while ( *fields )
    {
    DEBUGst(Fields,*fields);
    if ( strncmp( *fields, mask, length ) == 0 )
      {
      DEBUG(Matched);
      s = *fields + length;
      t = field = (char *) malloc( 257 );
      while ( *s  &&  *s != ')' )
	*t++ = *s++;
      *t = 0;
      break;
      }
      else fields++;
    }
  DEBUGst(Extracted,field);
  OUT(Extract_Field_Value);
  return  field;
  }

static
void Reader( register class chart	    	      *self )
    {
  register struct apt_field	     *field;

  IN(Reader);
  while ( field = (self )->ReadObjectField( ) )
    {
    DEBUGst(Field-Name,field->name);
    DEBUGst(Field-Content,field->content);
    if ( strcmp( "Item", field->name ) == 0 )
	Parse_Item_Field( self, field->content );
    else
    if ( strcmp( "ChartType", field->name ) == 0 )
	Parse_Type_Field( self, field->content );
    else
    if ( strcmp( "ChartTitle", field->name ) == 0 )
	Parse_Name_Field( self, field->content );
    }
  OUT(Reader);
  }

long
chart::Read( register FILE			      *file, register long			       id )
        {
  class chart *self=this;
  register long			      status; 

  IN(chart::Read);
  ItemCount = 0;
  if ( (status = (this)->ReadObject(  file, id, (apt_readfptr) Reader )) ==
	dataobject_NOREADERROR )
    {
    (this)->NotifyObservers(  1234 );
    }
  OUT(chart::Read);
  return status;
  }

static
void Parse_Name_Field( register class chart		      *self, register const char			      *string )
      {
  IN(Parse_Name_Field);
  DEBUGst(Name,string);
  (self)->SetChartAttribute(  chart_TitleCaption(string) );
  OUT(Parse_Name_Field);
  }

static
void Parse_Type_Field( register class chart		      *self, register const char			      *string )
      {
  IN(Parse_Type_Field);
  DEBUGst(Type,string);
  ChartType = strdup(string); /* tjm - FIXME: find out if necessary */
  OUT(Parse_Type_Field);
  }

static
void Parse_Item_Field( register class chart		      *self, register const char			      *string )
      {
  register char			    **fields;
  char				     *extract;
  register struct chart_item	     *item;
  long				      value;

  IN(Parse_Item_Field);
  DEBUGst(Item,string);
  if ( fields = (self)->ParseFieldContent(  string ) )
    {
    extract = Extract_Field_Value( self, (const char **)fields, "Name" );
    if ( item = (self)->CreateItem(  extract, 0 ) )
      {
      if ( extract )  free( extract );
      if ( extract = Extract_Field_Value( self, (const char **)fields, "Value" ) )
	{
	sscanf( extract, "%ld", &value );
	SetItemValue( self, item, value );
        free( extract );
	}
      if ( extract = Extract_Field_Value( self, (const char **)fields, "Position" ) )
	{
	sscanf( extract, "%ld", &value );
	ItemPosition(item) = value;
	free( extract );
	}
      }
    }
  OUT(Parse_Item_Field);
  }

static char *
ValueString( register class chart		      *self, register struct chart_item	      *item )
      {
  static char			      value[257];
  register char			     *ptr = value;

  *value = 0;
  sprintf( value, "%ld", ItemValue(item) );
  return  ptr;
  }

static
void Writer( register class chart		      *self )
    {
  register long			      i;
  register struct chart_item	     *item = ItemAnchor;
  struct apt_field		      field;
  char				      content[100], contents[1000];

  IN(Writer);
  field.name = "ChartType";
  DEBUGst(Chart-type,ChartType);
  field.content = ChartType;
  DEBUGst(Type,field.content);
  (self)->WriteObjectField(  &field );
  field.name = "Item";
  field.content = contents;
  i = ItemCount;
  while ( i-- )
    {
    *contents = 0;
    if ( ItemName(item) )
      {sprintf( content, "Name(%s);",	    ItemName(item) );
       strcat( contents, content );}
    if ( ValueString(self,item) )
      {sprintf( content, "Value(%s);",	    ValueString(self,item) );
       strcat( contents, content );}
    if ( ItemPosition(item) )
      {sprintf( content, "Position(%ld);",   ItemPosition(item) );
       strcat( contents, content );}
    (self)->WriteObjectField(  &field );
    item = NextItem(item);
    }
  OUT(Writer);
  }

long
chart::Write( register FILE			      *file, register long			       writeID, register int			       level )
          {
  class chart *self=this;
  IN(chart::Write);
  (this)->WriteObject(  file, writeID, level, (apt_writefptr) Writer );
  OUT(chart::Write);
  return  this->dataobject::id;
  }

const struct chart_monikers *
chart::Monikers( )
    {
static const struct chart_monikers	monikers[] = /*===MUST BE DYNAMIC*/
{
{"Histogram","charthst"},
{"Pie","chartpie"},
{"Dot","chartdot"},
{"Line","chartlin"},
{"Cartesian","chartcsn"},
{"Map","chartmap"},
{"Stack","chartstk"},
{0,0}
};

  IN(chart_Monikers);
/*===*/
  OUT(chart_Monikers);
return  monikers;
  }

const char *
chart::ModuleName( register const char			      *moniker )
      {
  register const char			     *module_name = NULL;
  register const struct chart_monikers     *monikers;

  IN(chart_ModuleName);
  DEBUGst(Moniker,moniker);
  if ( monikers = (this )->Monikers( ) )
    {
    while ( monikers->chart_moniker )
      {
      DEBUGst(Candidate-moniker,monikers->chart_moniker);
      if ( strcmp( moniker, monikers->chart_moniker ) == 0 )
	{
	module_name = monikers->chart_module_name;
	break;
	}
      monikers++;
      }
    }
    else
    {
/*===*/
    }
  DEBUGst(Module-name,module_name);
  OUT(chart_ModuleName);
  return  module_name;
  }

struct chart_item *
chart::CreateItem( register const char			      *name, register long			       datum )
        {
  class chart *self=this;
  register struct chart_item	     *item, *next = ItemAnchor;

  IN(chart_CreateItem);
  if ( item = (struct chart_item *) calloc( 1, sizeof(struct chart_item) ) )
    {
    ItemPosition(item) = ++ItemCount;
    apts::CaptureString( name, &ItemName(item) );
    item->datum = datum;
    if ( ItemAnchor )
      {
      while ( next->next )
        next = next->next;
      next->next = item;
      }
      else  ItemAnchor = item;
    }
  OUT(chart_CreateItem);
  return  item;
  }

struct chart_item *
chart::ItemOfName( register const char			      *name )
      {
  class chart *self=this;
  register struct chart_item	     *item = NULL, *next = ItemAnchor;

  IN(chart_ItemOfName);
  while ( next )
    if ( ItemName(next)  &&  *ItemName(next)  &&
	 strcmp( ItemName(next), name ) == 0 )
      {
      item = next;
      break;
      }
      else  next = next->next;
  OUT(chart_ItemOfName);
  return  item;
  }

void
chart::DestroyItem( register struct chart_item	      *item )
      {
  class chart *self=this;
  register struct chart_item	     *prior;

  IN(chart_DestroyItem);
  if ( item )
    {
    ItemCount--;
    if ( ItemName(item) )   free( ItemName(item) );
    if ( item == ItemAnchor )
      ItemAnchor = item->next;
      else
      {
      prior = ItemAnchor;
      while ( prior )
	{
	if ( prior->next == item )
	  {
	  prior->next = item->next;
	  break;
	  }
	prior = NextItem(prior);
	}
      }
    free( item );
    }
  OUT(chart_DestroyItem);
  }

static
void SetItemValue( register class chart		      *self, register struct chart_item	      *item, register long			       value )
        {
  IN(SetItemValue);
  ItemValue(item) = value;
  if ( ItemValue(item) < ItemValueLeast )
    {
    ItemValueLeast = ItemValue(item);
    ItemValueSpan = ItemValueGreatest - ItemValueLeast;
    if ( ItemValueLeast < ItemValueRangeLow )
      ItemValueRangeLow = ItemValueLeast;
    }
    else
    if ( ItemValue(item) > ItemValueGreatest )
      {
      ItemValueGreatest = ItemValue(item);
      ItemValueSpan = ItemValueGreatest - ItemValueLeast;
      if ( ItemValueGreatest > ItemValueRangeHigh )
        ItemValueRangeHigh = ItemValueGreatest;
     }
  OUT(SetItemValue);
  }

void
chart::Reset( register long			       mode )
      {
  IN(chart_Reset);
/*===*/
  OUT(chart_Reset);
  }

void
chart::Apply( chart_applyfptr proc, register long			       anchor, register long			      datum )
          {
  class chart *self=this;
  register struct chart_item	      *item = (struct chart_item *) ItemAnchor;

  IN(chart_Apply);
  while ( item )
    {
    (proc)( anchor, this, item, datum );
    item = NextItem(item);
    }
  OUT(chart_Apply);
  }

static int
Sort_By_Ascending_Value( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    {
    if ( ItemValue(*a) < ItemValue(*b) )  return -1;
    if ( ItemValue(*a) > ItemValue(*b) )  return  1;
    }
  return 0;
  }

static int
Sort_By_Descending_Value( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    {
    if ( ItemValue(*a) < ItemValue(*b) )  return  1;
    if ( ItemValue(*a) > ItemValue(*b) )  return -1;
    }
  return 0;
  }

static int
Sort_By_Ascending_Label( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    return  strcmp( ItemName(*a), ItemName(*b) );
  return 0;
  }

static int
Sort_By_Descending_Label( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    return  strcmp( ItemName(*b), ItemName(*a) );
  return 0;
  }

static int
Sort_By_Ascending_Position( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    {
    if( ItemPosition(*a) < ItemPosition(*b) )  return -1;
    if( ItemPosition(*a) > ItemPosition(*b) )  return  1;
    }
  return 0;
  }

static int
Sort_By_Descending_Position( register struct chart_item	     **a , register struct chart_item	     **b )
    {
  if ( a && b )
    {
    if( ItemPosition(*a) < ItemPosition(*b) )  return  1;
    if( ItemPosition(*a) > ItemPosition(*b) )  return -1;
    }
  return 0;
  }

void
chart::Sort( register long			       mode, register chart_sortfptr handler )
        {
  class chart *self=this;
  int			(*sorter)(struct chart_item **, struct chart_item **) = NULL; 
  register long			     i = 0;
  register chart_type_item	     *vector;
  register chart_type_item	      item = ItemAnchor;

  IN(chart_Sort);
  if ( vector = (chart_type_item *) malloc( ItemCount * sizeof(chart_type_item) ) )
    {
    while ( item )
      {
      vector[i++] = item;
      item = NextItem(item);
      }
    if ( mode & chart_ByValue )
      {
      if ( mode & chart_Descend )
        sorter = Sort_By_Descending_Value;
	else
        sorter = Sort_By_Ascending_Value;
      }
    else
    if ( mode & chart_ByLabel )
      {
      if ( mode & chart_Descend )
        sorter = Sort_By_Descending_Label;
	else
        sorter = Sort_By_Ascending_Label;
      }
    else
    if ( mode & chart_ByPosition )
      {
      if ( mode & chart_Descend )
        sorter = Sort_By_Descending_Position;
	else
        sorter = Sort_By_Ascending_Position;
      }
    if ( sorter )
      { DEBUGdt(Qsort,ItemCount);
      qsort( vector, ItemCount, sizeof(chart_type_item), QSORTFUNC(sorter) );
      DEBUG(Qsort Done);
      ItemAnchor = vector[0];
      for ( i = 1; i < ItemCount; i++ )
	{
	DEBUGst(ItemName,ItemName(vector[i-1]));
	NextItem(vector[i-1]) = vector[i];
	}
      NextItem(vector[i-1]) = NULL;
      free ( vector );
      }
    }
    else
    {
/*===*/
    }
  OUT(chart_Sort);
  }

