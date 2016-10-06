#ifndef _AWidget_H_
#define _AWidget_H_
/* ***************************************************
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'
 *  ***************************************************

	awidget.H   -  widget data object

	The file format for a widget begins and ends with the standard 
	\begindata \enddata.  Contained is an avarset object.
*/

/** \addtogroup libawidget Object awidget
 *  A statically loadable ATK class file.  Contains AWidget.  Also contains
 *  ASlot, ASlotInt, ASlotReal, ASlotString, ASlotFile, ASlotPtr, ASlotATK
 *  ASlotFunction, ASlotDataObj, ASlotFigure, ASlotFont, ASlotBool,
 *  ASlotColor, AVarSet, AWidgetView, AFormulaViewAct, AWgtFig, AWgtFigView,
 *  ADepender, ADependerPlaceHolder, AFormula, AFormulaAct, AFormulaSlot.
 * @{ */
/* status values for NotifyObservers */
#define awidget_DATACHANGED 1

#include <aslot.H>
#include <figview.H>

class AWidgetView;

#include <avarset.H>
class AWidget : public  AVarSet {
  public:

	ASlotReal units;  // pts/100 per unit,  default 1.0
		// Widget measurements are long integer values.
		// The distance represented by a measurement of length 1
		// is given in the units slot, where it is expressed
		// as a multiple of .01 points  (1/7200 inches)
		// Example.  If the widget measures distance in points,
		//	the value of 'units' will be 100.0
		// If measurment in the widget is in terms of pixels, 
		// units should be set to the special value 0.

		// Parameters to figure methods are measured in 1/8 point.
		// If x is a value in widget units, ToFigU(x) is the
		// corresponding figure measurement.
	inline long ToFigU(long dist) {
			double factor = units;    
			return (long)(dist * ((factor)?factor:100.0) 
					* (figview_FigUPerPix/100.0)); 
	}

	ASlotInt designWidth;	// default 1900
	ASlotInt designHeight;	// default 1900
	ASlotFont designFont;	// default is font for "andy12"
	ASlotReal scaleWidth;	// default 1.0
	ASlotReal scaleHeight;	// default 1.0
	ASlotBool resizable;	// default FALSE; affects matte

	ASlotFunction computeDesignSize;
	ASlotFunction computeViewSize;

	// these next three are for caching the result of 
	// computeViewSize.  NOT written to data stream.
	ASlotFont priorViewFont;  // default NULL
	ASlotInt desiredWidth;  	// default 0
	ASlotInt desiredHeight;	// default 0

	ASlotBool sensitive;
		// True, only if this widget will respond to input.
	ASlotFunction mouseAny; 
	ASlotFunction mouseLeftDown; 
	ASlotFunction mouseLeftMove;
	ASlotFunction mouseLeftUp;
	ASlotFunction mouseRightDown;
	ASlotFunction mouseRightMove;
	ASlotFunction mouseRightUp;
	
	// printImage - (String or Function) 
	//	either a string giving postscript for the image
	//	or a function that generates and returns that string

	ASlotInt nominalPrintHeight; 
	ASlotInt nominalPrintWidth;  
		// nominal printed size of printImage
		// nominalPrint{Height,Width} can be set by the 
		// function that computes printImage
		// The image will be scaled so it fits into whatever space 
		// has actually been allocated on the page
	
	virtual ATKregistryEntry *ATKregistry();
	AWidget();

	// destructors for embedded slots will destruct their contents
	// Read and Write are handled by AVarSet

	static AWidget *NewWidget(const char *widgettype);
	static AWidgetView *NewWidgetView(AWidget *dobj, const char *widgettype=NULL);
};

#define WIDGET_DATA_FUNC_CLASS(DataClass,ViewClass) \
DEFINE_AACTION_FUNC_CLASS(Concat(DataClass,Act),DataClass)

#define WIDGET_VIEW_FUNC_CLASS(DataClass,ViewClass) \
DEFINE_AACTION_FUNC_CLASS(Concat(DataClass,Act),DataClass)

#define WIDGET_DATA_METHOD_CLASS(DataClass,ViewClass) \
DEFINE_AACTION_CLASS(Concat(DataClass,MethodAct),DataClass)

#define WIDGET_VIEW_METHOD_CLASS(DataClass,ViewClass) \
DEFINE_AACTION_CLASS(Concat(ViewClass,MethodActBase), ViewClass); \
class Concat(ViewClass,MethodAct) : public Concat(ViewClass,MethodActBase) { \
  public: \
	inline Concat(ViewClass,MethodAct)(void (ViewClass::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a) : Concat(ViewClass,MethodActBase)(p, a) { \
    	} \
	  inline Concat(ViewClass,MethodAct)(void (ViewClass::*p)(const avalueflex &aux,  const avalueflex &in, avalueflex &out), const avalueflex &a, const avalueflex &t,  const avalueflex &r) : Concat(ViewClass,MethodActBase)(p, a, t, r) { \
    	} \
inline virtual void operator()(ATK *obj, const avalueflex &in, avalueflex &out) { \
ATK_CLASS(ViewClass); \
void (ViewClass::*p)(const avalueflex &in, const avalueflex &aux, avalueflex &out)=pmf; \
ViewClass *abv; \
              if(obj && obj->IsType(Concat(class_,ViewClass))) { \
                  abv=(ViewClass *)obj; \
              } else { \
                  abv=((ViewClass *)in[0].ATKObject()); \
              } \
            (abv->*p)(auxargs, in, out); \
} \
};

#define WIDGET_FUNC_AND_METHOD_CLASS(DataClass,ViewClass) \
ATK_CLASS(DataClass); \
ATK_CLASS(ViewClass); \
WIDGET_DATA_FUNC_CLASS(DataClass,ViewClass); \
WIDGET_DATA_METHOD_CLASS(DataClass,ViewClass) ; \
WIDGET_VIEW_METHOD_CLASS(DataClass,ViewClass);

#define START_VIEW_CALLBACK_METHOD(name,DataClass,ViewClass) \
AACTION_METHOD(name,ViewClass) { \
DataClass *dself=(DataClass *)GetDataObject();
#define END_CALLBACK_METHOD() }

#define START_DATA_CALLBACK_METHOD(name,DataClass,ViewClass) \
AACTION_METHOD(name,DataClass) { \
ViewClass *vself=(ViewClass *)in[0].ATKObject();


#define START_VIEW_CALLBACK_FUNC(name,DataClass,ViewClass) \
AACTION_FUNC(name,DataClass) { \
DataClass *dself=(DataClass *)obj; \
ViewClass *self=(ViewClass *)in[0].ATKObject();

#define START_DATA_CALLBACK_FUNC(name,DataClass,ViewClass) \
AACTION_FUNC(name,DataClass) { \
ViewClass *vself=(ViewClass *)in[0].ATKObject();
#define END_CALLBACK_FUNC() }

#define START_VIEW_MOUSE_METHOD(name,DataClass,ViewClass) \
AACTION_METHOD(name,ViewClass) {\
long x=in[1].Integer(); \
long y=in[2].Integer(); \
boolean within=(boolean)in[3].Integer(); \
view_MouseAction action \
= (view_MouseAction)in[4].Integer(); \
long num_clicks=in[5].Integer(); \
DataClass *dself=(DataClass *)GetDataObject();
#define END_MOUSE_METHOD() }

#define START_DATA_MOUSE_METHOD(name,DataClass,ViewClass) \
AACTION_METHOD(name,DataClass) {\
long x=in[1].Integer(); \
long y=in[2].Integer(); \
boolean within=(boolean)in[3].Integer(); \
view_MouseAction action \
= (view_MouseAction)in[4].Integer(); \
long num_clicks=in[5].Integer(); \
ViewClass *vself=(ViewClass *)GetDataObject();


#define START_VIEW_MOUSE_FUNC(name,DataClass,ViewClass) \
AACTION_FUNC(name,DataClass) { \
DataClass *dself=(DataClass *)obj; \
ViewClass *self=(ViewClass *)in[0].ATKObject(); \
long x=in[1].Integer(); \
long y=in[2].Integer(); \
boolean within=(boolean)in[3].Integer(); \
view_MouseAction action \
= (view_MouseAction)in[4].Integer(); \
long num_clicks=in[5].Integer();
#define END_MOUSE_FUNC() }

#define START_DATA_MOUSE_FUNC(name,DataClass,ViewClass)  \
AACTION_FUNC(name,DataClass) {\
ViewClass *vself=(ViewClass *)in[0].ATKObject(); \
long x=in[1].Integer(); \
long y=in[2].Integer(); \
boolean within=(boolean)in[3].Integer(); \
view_MouseAction action \
= (view_MouseAction)in[4].Integer(); \
long num_clicks=in[5].Integer();

/*    $Log: awidget.H,v $
 * Revision 1.17  1996/10/09  19:22:59  robr
 * Hacked the widget aaction interface to deal with getting either
 * a view or a dataobject as the obect argument.
 * so that the (data,view,arguments) and (view,argument) protocols
 * can both be used.
 * BUG
 *
 * Revision 1.16  1996/10/07  17:49:42  robr
 * oops, removed bogus revision...
 * BUG
 *
 * Revision 1.14  1996/09/25  17:49:21  robr
 * FIxed pasto omission of local vars for START_VIEW_MOUSE_FUNC.
 * BUG
 *
 * Revision 1.13  1996/09/25  17:05:00  robr
 * Added the genericized convenience functions used for AButtonv.
 * FEATURE
 *
 * Revision 1.12  1996/05/13  17:13:58  wjh
 *  add slots resizable and mouseAny; change slot sensitive to ASlotBool
 *
 * Revision 1.11  1996/05/07  20:35:59  wjh
 * moved log to end and truncated; \
 * added units (pt/100)
 *
 * Revision 1.10  1996/05/02  02:46:55  wjh
 * added units, priorViewFont, desiredWidth, desiredHeight; truncated log
 * . . .
 * Revision 1.0  95/09/05  16:23:22  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
 */

/** @} */
#endif /* _AWidget_H_ */
