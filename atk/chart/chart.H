ATK_INTER
#ifndef _chart_H_
#define _chart_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libchart libchart.so
 *  A dynamically loadable ATK class file.  Contains chart, chartv,
 *  chartapp, chartobj, chartdot, chartpie, chartx1app, charthst,
 *  chartstk, chartcsn, chartlin.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Chart Data-object

MODULE	chart.ch

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
  02/23/89	Created (TCP)
  05/23/89	Add EnclosureModified notification-code (TCP)
  09/01/89	Upgrade to V1.0

END-SPECIFICATION  ************************************************************/


#define  chart_VERSION    1

#include <apt.H>

typedef struct chart_specification  chart_Specification;
struct  chart_specification
  {
  char	attribute;
  long	value;
  };

typedef struct chart_item	     *chart_type_item;
struct  chart_item
  {
  struct chart_item		     *next;
  char				     *name;
  long				      datum;
  long				      position;
  long				      value;
  char				      shade; /* 0 => None, 1 => White ... 255 = Black */
  };

typedef struct chart_monikers	     *chart_type_monikers;
struct  chart_monikers
  {
  const char			     *chart_moniker;
  const char			     *chart_module_name;
  };

typedef long (*chart_applyfptr)(long ca, class chart *self, struct chart_item *i, long datum);
typedef int (*chart_sortfptr) (struct chart_item  **a, struct chart_item  **b);

class chart : public  apt
  {
public:
	virtual ATKregistryEntry *ATKregistry();




   long Read( FILE *file, long id )				 ;
   long Write( FILE *file, long id, int level )		 ;
   const char * ViewName()						 ;



  /***  Methods Dealing with Chart as a Whole  **/

  virtual long SetChartAttribute( long attribute, long value )			 ;
  virtual long ChartAttribute( long attribute_code )			 ;

  virtual void Apply( chart_applyfptr proc, long anchor, long datum );
  virtual void Reset( long mode );
  virtual void Sort( long mode, chart_sortfptr handler );
  virtual const struct chart_monikers * Monikers()						 ;
  virtual const char * ModuleName( const char *moniker )				 ;
  virtual void SetDebug( boolean state );

  /***  Methods Dealing with Individual Items  **/

  virtual long SetItemAttribute( struct chart_item *item, long attribute, long value )
							 ;
  virtual long ItemAttribute( struct chart_item *item, long attribute_code )
							 ;
  virtual struct chart_item * CreateItem( const char *item_name, long datum )		 ;
  virtual void DestroyItem( struct chart_item *item );
  virtual struct chart_item * ItemOfName( const char *item_name )				 ;



  inline struct chart_item * ItemAnchor()			      { return (this->item_anchor); }
  inline long ItemCount()			      { return (this->item_count); }

  inline long ItemValueGreatest()		      { return (this->item_value_greatest); }
  inline long ItemValueLeast()		      { return (this->item_value_least); }
  inline long ItemValueRangeHigh()		      { return (this->item_value_range_high); }
  inline long ItemValueRangeInterval()	      { return (this->item_value_range_interval); }
  inline long ItemValueRangeLow()		      { return (this->item_value_range_low); }
  inline long ItemValueSpan()		      { return (this->item_value_span); }
  inline const char * ItemFontName()	      { return (this->item_font_name); }

  inline struct chart_item * NextItem( struct chart_item *item)		      { return ((item)->next); }



  
	chart();
  
	virtual ~chart();
  static class chart * Create( chart_Specification *chart_Spec, char *anchor )		 ;



  char				     *chart_moniker, *chart_title,
				     *chart_title_data_object_name, *chart_title_view_object_name;
  class dataobject		     *client_anchor;
  long				      client_datum, item_count;
  struct chart_item		     *item_anchor;
  long				      id;
  char				     *chart_file_name, *item_font_name;
  long				      item_value_greatest, item_value_least,
				      item_value_range_low, item_value_range_high,
				      item_value_range_interval, item_value_span;
  struct chart_monikers		     *chart_monikers;
  long				      exception_code;
  struct chart_item		     *exception_item;
  };

/***  Exception Codes  ***/
#define  chart_NonExistentItem			1
#define  chart_UnknownChartAttribute		2
#define  chart_UnknownItemAttribute		3

/***  Notification Codes  ***/
#define  chart_ItemCreated			1
#define  chart_ItemDestroyed			2
#define  chart_ItemValueChanged			3
#define  chart_ItemNameChanged			4
#define  chart_ItemCaptionChanged		5
#define  chart_ItemsSorted			6
#define  chart_EnclosureModified		7


/***  Chart Attribute Values  ***/

/***  Chart Sort Fields  ***/
#define  chart_ByValue			    (1<<0)
#define  chart_ByLabel			    (1<<1)
#define  chart_ByPosition		    (1<<2)

/***  Chart Sort Order	***/
#define  chart_Ascend			    (1<<8)
#define  chart_Descend			    (1<<9)

/***  Attribute Macros  ***/

#define  chart_Datum(x)			    chart_datum,	    (long) x
#define  chart_FileName(x)		    chart_filename,	    (long) x
#define  chart_ItemDatum(x)		    chart_itemdatum,	    (long) x
#define  chart_ItemName(x)		    chart_itemname,	    (long) x
#define  chart_ItemPosition(x)		    chart_itemposition,	    (long) x
#define  chart_ItemValue(x)		    chart_itemvalue,	    (long) x
#define  chart_TitleCaption(x)		    chart_titlecaption,	    (long) x
#define  chart_TitleDataObjectName(x)	    chart_titledataobjectname,	(long) x
#define  chart_TitleViewObjectName(x)	    chart_titleviewobjectname,	(long) x
#define  chart_Type(x)			    chart_type,		    (long) x

/***  Attribute Codes  ***/

#define  chart_datum	    		    5
#define  chart_filename			    6

#define  chart_itemdatum		    12
#define  chart_itemname			    14
#define  chart_itemposition		    15
#define  chart_itemvalue		    16

#define  chart_titlecaption		    33
#define  chart_titledataobjectname	    35
#define  chart_titleviewobjectname	    38

#define  chart_type			    40
/** @} */
#endif /* _chart_H_ */
