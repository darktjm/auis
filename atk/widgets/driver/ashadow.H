/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#ifndef _AShadow_H_
#define _AShadow_H_
ATK_INTER


#include <atom.H>
#include <color.H>
#include <oflex.H>
#include <aformula.H>
class AWidgetView;
class AVarSet;

#define AShadow_Highlight 	1
#define AShadow_Default   	2
#define AShadow_Plain    	4
/*
	These three define kinds of borders.
	Highlight is the outermost and happens when the inset has
	the input focus.  Default means that the action of this
	widget will occur when Newline is typed.  Plain is the
	usual chiseled border.

    For AShadow::Create and UseBorder, the valid combinations of 
    AShadow_Highlight, AShadow_Default, and AShadow_Plain are:
	Highlight/Default/Plain
	Highlight/Default
	Highlight
	Default/Plain
	Plain
	(none)

    Invalid:  Highlight/Plain  and  Default
*/


// The elements of enum AShadowColors are the various components of 
// shadowed entities which can be given different colors.
enum AShadowColors {
    AShadowColors_Background=0,
    AShadowColors_Foreground,
    AShadowColors_UpperShadow,
    AShadowColors_LowerShadow,
    AShadowColors_LabelFG,
    AShadowColors_LabelBG,
    AShadowColors_InsensitiveLabelFG,
    AShadowColors_InsensitiveLabelBG,
    AShadowColors_InsensitiveBackground,
    AShadowColors_InsensitiveForeground,
    AShadowColors_SelectedBackground,
    AShadowColors_SelectedForeground,
    AShadowColors_Transparent,
    AShadowColors_Custom,
    AShadowColors_DefaultButtonUS,
    AShadowColors_DefaultButtonLS,
    AShadowColors_Highlight,
    AShadowColors_Colors
};



// An AShadowColorBlock is used to compute and cache the colors
// needed by the various components of a shadowed entity.
//
class AShadowColorBlock {
    color col[AShadowColors_Colors];
    // The colors actually used for each component.
    AWidgetView *target;
    // The target view for which color allocations should be done.
    AVarSet *resources;
    // The set of resources to search for alternate settings.
  public:
    AShadowColorBlock(AWidgetView *v=NULL, AVarSet *d=NULL);
    inline void SetTarget(AWidgetView *v) { target=v; }
    inline void SetResources(AVarSet *d) { resources=d; }
    color &operator[](AShadowColors shade);
	    // operator[] returns a color object for use in drawing 
	    // the 'shade' component.
};


// AShadowColor represents the color of a component in a shadowed
// entity such as a button.  Components of shadowed entities include:
// the background, the upper and lower shadows, and the label.
// 
class AShadowColor {
    const atom *resource;
  	  // Optionally a resource name to use to look up
  	  // alternate settings for this component.
    enum AShadowColors shade;
	    // The shade of this component.
    color col;
	    // The actual color to use.
  public:
    AShadowColor();
	    // Constructs an AShadowColor with no settings.
    AShadowColor(AShadowColors shade, const atom_def &name);
	    // Constructs an AShadowColor of the given shade, and using the
	    // given resource name to override any computed value.
    AShadowColor(AShadowColors shade);
	    // Constructs an AShadowColor of the given shade.
    AShadowColor(const AShadowColor &src);
	    // Constructs a copy of another AShadowColor.
    AShadowColor &operator=(const AShadowColor &src);
	    // Assigns one AShadowColor's value to another.
    inline AShadowColor &operator=(const char *src)
		{ col=src;  return *this;}
	    // Overrides any previous setting or computation and uses src
	    // as the color of this component.
    inline AShadowColor &operator=(const color &src) 
		{ col=src;  return *this;}
	    // Overrides any previous setting or computation and uses src
	    // as the color of this component.
    inline void SetResourceName(const atom_def &name) { resource=name; }
	    // Sets the name of the resource used to locate alternate settings
	    // for this component.
    inline const atom *ResourceName() { return resource; }
	    // Returns the name of the resource used to locate alternate
	    // settings for this component.
    inline void SetShade(AShadowColors t) { shade=t; }
	    // Sets the type of this component.
    inline AShadowColors Shade() { return shade; }
	    // Returns the shade of this component.
    inline color &Color() { return col; }
	    // Returns the color to use for this component.
};


// An AShadowLayer is one line in the shadows above or below the
// main area of a shadowed entity such as a button.
//
class AShadowLayer {
    AShadowColor color;
	    // The color to use for this layer of the border.
    unsigned char thickness;
	    // The thickness of this layer.
  public:
    AShadowLayer();
	    // Constructs an AShadowLayer of thickness 0 with no color set.
    AShadowLayer(AShadowColors t, const atom_def &name, unsigned char d=1);
	    // Constructs an AShadowLayer of shade t, and thickness d,
	    // using name to locate alternate settings.
    AShadowLayer(AShadowColors t, unsigned char thickness=1);
	    // Constructs an AShadowLayer of shade t, and thickness d.
    inline AShadowColor &Color() { return color; }
	    // Returns the AShadowColor to be used for this layer.
    inline void SetThickness(unsigned char d) { thickness=d; }
	    // Sets the thickness of this layer.
    inline unsigned char Thickness() { return thickness; }
	    // Returns the thickness of this layer.
    inline void SetShade(AShadowColors shade) { color.SetShade(shade); }
	    // Sets the shade of this layer.
    inline AShadowColors Shade() { return color.Shade(); }
	    // Returns the shade of this layer.
};

// An AShadowLayers is an array of layers.
DEFINE_OFLEX_CLASS(AShadowLayers,AShadowLayer,1);

class AShadow {
    AFormula *scaleFormula;
    double uxscale, uyscale;
    AShadowColor background;
	    // The color to use for the backrgound.
    AShadowLayers upper, lower;
	    // The arrays of components above and below
	    // the main area of the shadowed entity.
    AShadowColorBlock scb;
	    // The object used to compute and cache the
	    // colors computed or set from the standard resources.
    AVarSet *resources;
	    // The resources to use to locate alternate color settings.

    boolean armed;
	    // Is the 'button' currently 'pressed'?
    boolean selected;
	    // Is the 'button' selected.  
	    // (e.g., the selected item in a radio button box.)
    boolean sensitive;
	    // Is the 'button' currently sensitive to hits?
    boolean def;
	    // Is the 'button' the default?  
	    // (ie.  will it activate when type newline)
    boolean highlighted;
	    // Is the 'button' highlighted?  (i.e. it has the input focus)
    boolean fillOnArm;
	    // Should the 'button' be filled specially when armed?
    
    boolean dirty;  // T if needs redrawn

    void AllocateShadowColor(AShadowColor &dest);
	    //  A private function which does the work of setting
	    // a color from the standard computations or resources,
	    // or from a custom resource.

    boolean standard;
    // if T, a standard set of resources are used for selected,
    // sensitive, def, and fillOnArm.
    boolean indicator;
    // T iff this ashadow should show the Selected state when selected is set.

    inline void CheckScale() {
        if(scaleFormula) scaleFormula->Validate();
    }
  public:
    inline AShadow(AWidgetView *v=NULL, AVarSet *d=NULL, AFormula *sFormula=NULL) : scaleFormula(sFormula), uxscale(1.0), uyscale(1.0), background(AShadowColors_Background), scb(v, d), resources(d), armed(FALSE), selected(FALSE), sensitive(TRUE), def(TRUE), highlighted(TRUE), fillOnArm(TRUE), dirty(TRUE), standard(TRUE), indicator(TRUE) {
    }
    static AShadow *Create(AWidgetView *v=NULL, AVarSet *d=NULL, 
		unsigned char flags
		= AShadow_Highlight | AShadow_Default | AShadow_Plain);
	    // creates an AShadow object using v and d
	    // and then calls UseBorder(flags) on it.
            // if the borderShape slot in d is 0, returns NULL
    inline void SetScaleFormula(AFormula *sf) {
        scaleFormula=sf;
        if(sf) sf->Invalidate();
    }
    AFormula *ScaleFormula() {
        return scaleFormula;
    }
    void SetFillOnArm(boolean a);
    boolean FillOnArm();
    void SetArmed(boolean a);
    inline boolean Armed() { return armed; }
    void SetIndicator(boolean a);
    inline boolean Indicator() { return indicator; } 
    void SetSelected(boolean a);
    boolean Selected();
    void SetHighlighted(boolean a);
    inline boolean Highlighted() { return highlighted; }
    void SetDefault(boolean a);
    boolean Default();
    void SetSensitive(boolean s);
    boolean Sensitive();
    inline void SetStandard(boolean a) {
	standard=a;
    }
    boolean Standard() {
	return standard;
    }
    inline void SetTarget(AWidgetView *v) { scb.SetTarget(v); }
    inline void SetResources(AVarSet *d) 
		{ resources=d;  scb.SetResources(d); }
	    // Constructs an AShadow object for the view v, 
	    // getting resources from the AVarSet d.
    inline color &UpperColor(size_t i) {
	    // Returns the color to be used for the ith upper layer.
	AShadowColor &ref=upper[i].Color();
	AllocateShadowColor(ref);
	return ref.Color();
    }
    inline color &LowerColor(size_t i) {
    // Returns the color to be used for the ith lower layer.
	AShadowColor &ref=lower[i].Color();
	AllocateShadowColor(ref);
	return ref.Color();
    }
    inline unsigned char UpperThickness(size_t i) 
     		{ return upper[i].Thickness(); }
	    // Returns the thickness of the ith upper layer.
    inline unsigned char LowerThickness(size_t i)
		{ return lower[i].Thickness(); }
	    // Returns the thickness of the ith lower layer.
    inline unsigned char UpperShade(size_t i)
		{ return upper[i].Shade(); }
	    // Returns the shade of the ith upper layer.
    inline unsigned char LowerShade(size_t i)
		{ return lower[i].Shade(); }
	    // Returns the shade of the ith lower layer.
    inline size_t UpperLayers() 
		{ return upper.GetN(); }
	    // Returns the number of upper layers.
    inline size_t LowerLayers() 
		{ return lower.GetN(); }
	    // Returns the number of lower layers.
   inline AShadowLayer *AppendUpperLayer()
		{ return upper.Append(); }
	   // Adds a new upper layer above any previous layers.
   inline AShadowLayer *AppendLowerLayer() 
		{ return lower.Append(); }
	   // Adds a new lower layer below any previous layers.
   inline AShadow &addu(AShadowColors t, const atom_def &name, 
			unsigned char thickness=1) {
       *AppendUpperLayer()=AShadowLayer(t, name, thickness);
       return *this;
   }
   inline AShadow &addu(AShadowColors t, unsigned char thickness=1) {
       *AppendUpperLayer()=AShadowLayer(t, thickness);
       return *this;
   }
   inline AShadow &addl(AShadowColors t, const atom_def &name, 
			unsigned char thickness=1) {
       *AppendLowerLayer()=AShadowLayer(t, name, thickness);
       return *this;
   }
   inline AShadow &addl(AShadowColors t, unsigned char thickness=1) {
       *AppendLowerLayer()=AShadowLayer(t, thickness);
       return *this;
   }
   inline AShadowLayer *InsertUpperLayer(size_t i) 
		{ return upper.Insert(i); }
	   // Inserts a new upper layer at the ith position.
   inline AShadowLayer *InsertLowerLayer(size_t i)
		{ return lower.Insert(i); }
	   // Inserts a new lower layer at the ith position.
   inline void RemoveUpperLayer(size_t i) 
		{ upper.Remove(i); }
	   // Removes the ith upper layer.
   inline void RemoveLowerLayer(size_t i) 
		{ lower.Remove(i); }
	   // Removes the ith lower layer.
   inline AShadowColorBlock &ShadowBlock() { return scb; }
	   // Returns the AShadowColorBlock used to compute the
	   // desired colors.
   void UseBorder(unsigned char flags
		= AShadow_Highlight | AShadow_Default | AShadow_Plain);
	   // Sets up the default shadows, with thickness 2, and the default
	   // colors.  flags is the or of 0 or more of:
	   // 	AShadow_Highlight, AShadow_Default, AShadow_Plain
	   // indicating which kinds of layers are be included in the border.
	   // Some combinations are mapped to the closest supported
	   // combination.  The list above gives the supported combinations.
   
   void RemoveBorder();
	   // Removes any defined border layers.
#if 0
   // These are obsoleted by the need for separate X/Y scaling.
   unsigned int TotalUpperBorder();
	   // Returns the total thickness of the upper layers.
   unsigned int TotalLowerBorder();
	   // Returns the total thickness of the lower layers.
#endif
   void TotalUpperBorderWidthHeight(long &x, long &y);
   void TotalLowerBorderWidthHeight(long &x, long &y);
   
   void DrawBorder(AWidgetView *v, struct rectangle *r, 
			boolean ignoredirty=TRUE);
	   // checks resource->borderShape and draws the right border
   void DrawRectBorder(AWidgetView *v, struct rectangle *r);
	   // Draws a rectangular border within the rectangle r, 
	   // in the view v, according to the layers defined in this object.
   void DrawDiamondBorder(AWidgetView *v, struct rectangle *r);
	   // Draws a diamond shaped border within the rectangle r,
	   // according to the layers defined in this object.

   void FillRect(AWidgetView *v, struct rectangle *r);
	   // Fills the interior of the button bounded by r with the
	   // background color.
   void FillDiamond(AWidgetView *v, struct rectangle *r);
	   // Fills the interior of the diamond shaped button bounded
	   // by r with the background color.
   
   void ExteriorRect(struct rectangle *interior, struct rectangle *exterior);
	   // Computes the rectangle containing the border drawn around 
	   // the interior rectangle.
   void InteriorRect(struct rectangle *exterior, struct rectangle *interior);
	   // Computes the interior area left after drawing the border 
	   // within the exterior rectangle.
   
   void InterpretString(const char *src);
	   // Initializes the layers of this object from the textual 
	   // description in src. 
	/* The format of a  description is: []==optional
   [u|l] ([thickness] [ShadeName|Abbreviation|(Custom CustomResourceName)])
	u or l indicates whether subsequent entries apply to a
 	new upper or lower layer
	thickness indicates how deep the layer will be, if omitted 
	the default thickness or the thickness set by the Thickness 
	resource will be used.  (special Thickness resources are 
	consulted for highlighted (hightlightThickness),
	and shadows (shadowThickness, and defaultShadowThickness))
	ShadeName is one of (case insensitive):
		Background,    Foreground,    UpperShadow,    LowerShadow,    
		LabelFG,  LabelBG,  InsensitiveLabelFG,    InsensitiveLabelBG,
		InsensitiveBackground, InsensitiveForeground, 
		SelectedForeground,   SelectedBackground,   Transparent,
		DefaultButtonUS, DefaultButtonLS,  Highlight
	Abbreviations of the shade names are (case sensitive):
 		B, F, U, L, LF, LB, ILF, ILB, IB, IF, SF, SB, T, DU, DL, H
	In the case where the shade name "Custom" or its abbreviation is used 
	the next word will be the name of a resource of type Color.
	Otherwise the standard computations or resource lookups will be used.
	*/
   AShadowColors ShadeForState(AShadowColors s);
	   // Returns the AShadowColor shade to be used for the given
	   // AShadowColor given the state of the ashadow object.
	   // (i.e. whether it's the default, if it's armed, etc...)
   
   inline color &Foreground() { return scb[AShadowColors_Foreground]; }
   inline color &Background() {
	    // Returns the color to be used for the background of the entity.
	AllocateShadowColor(background);
	return background.Color();
   }
   inline void SetBackgroundResourceName(const atom_def &name) {
	    // Sets the resource name to be used to locate alternate
	    // settings for the background color.
	background.SetResourceName(name);
   }
   inline color &UpperShadow() { return scb[AShadowColors_UpperShadow]; }
   inline color &LowerShadow() { return scb[AShadowColors_LowerShadow]; }
   inline color &LabelFG() { return scb[AShadowColors_LabelFG]; }
   inline color &LabelBG() { return scb[AShadowColors_LabelBG]; }
   inline color &InsensitiveLabelFG() 
		{ return scb[AShadowColors_InsensitiveLabelFG]; }
   inline color &InsensitiveLabelBG()
		{ return scb[AShadowColors_InsensitiveLabelBG]; }
   inline color &InsensitiveBackground()
		{ return scb[AShadowColors_InsensitiveBackground]; }
   inline color &InsensitiveForeground()
		{ return scb[AShadowColors_InsensitiveForeground]; }
   inline color &SelectedBackground()
		{ return scb[AShadowColors_SelectedBackground]; }
   inline color &SelectedForeground()
		{ return scb[AShadowColors_SelectedForeground]; }
   inline color &DefaultButtonUS()
		{ return scb[AShadowColors_DefaultButtonUS]; }
   inline color &DefaultButtonLS()
		{ return scb[AShadowColors_DefaultButtonLS]; }
   inline void SetScale(double x, double y) {
       uxscale=x;
       uyscale=y;
   }
   inline double ScaleX() {
       return uxscale;
   }
   inline double ScaleY() {
       return uyscale;
   }
};
#endif /* _AShadow_H_ */
