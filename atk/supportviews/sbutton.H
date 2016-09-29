#ifndef _sbutton_H_
#define _sbutton_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
#include <rect.h>

#define DS_VERSION 10 /* datastream version */

/*
  Appearances for pushbutton:

  Style  Look
    0    plain, in line text
    1    a boxed rectangle
    2    a motif-ish 3-Dish button
    3    a single rectangle
    4	 a more motif-ish button
*/
#define sbutton_PLAIN 0
#define sbutton_BOXEDRECT 1
#define sbutton_THREEDEE 2
#define sbutton_PLAINBOX 3
#define sbutton_MOTIF 4

#define sbutton_SIZECHANGED 1
#define sbutton_TRIGGERCHANGED 2
#define sbutton_LABELCHANGED 4
#define sbutton_FONTCHANGED 8
#define sbutton_STYLECHANGED 16
#define sbutton_ACTIVATIONCHANGED 32
#define sbutton_ALLCHANGED 64
#define sbutton_SENSITIVITYCHANGED 128

#define sbutton_CHANGEBASE 1024  /* reserve 1024 values (no relation to sbutton_ALLCHANGED is implied...) for specifying other change details, numbers>1024 indicate that button #-1024 has had a change in activation status */

#define sbutton_BACKGROUND 0
#define sbutton_TOPSHADOW 1
#define sbutton_TOP 2
#define sbutton_BOTTOMSHADOW 3
#define sbutton_FOREGROUND 4
#define sbutton_LABELFG 5
#define sbutton_LABELBG 6
#define sbutton_INSENSITIVELABELFG 7
#define sbutton_INSENSITIVELABELBG 8
#define sbutton_INSENSITIVETOP 9
#define sbutton_COLORS 10

struct sbutton_prefs {
    int writeid;
    int refcount;
    int style;
    class fontdesc *font;
    const char *colors[sbutton_COLORS];
    char *name;
    int bdepth;
};

struct sbutton_info {
    struct sbutton_prefs *prefs;
    const char *label;
    long rock;
    class atom *trigger;
    boolean lit;
    boolean sensitive;
};


struct sbutton_list {
    const char *label;
    long rock;
    const char *trigger;
    boolean lit;
};

enum sbutton_sizepolicy {
    sbutton_NoneSet,
    sbutton_SizeFixed,
    sbutton_GrowRows,
    sbutton_GrowColumns,
    sbutton_ResizeFunction
};

#include <dataobject.H>

	typedef void (*sbutton_resizefptr)(class sbutton *self, long rock);
	typedef void (*sbutton_hitfptr)(class sbutton *self, long hitrock, int ind, int buttonrock);
	typedef boolean (*sbutton_efptr)(class sbutton *self, int i, struct sbutton_info *si, long rock);

class sbutton: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	
	static boolean InitializeClass();

    
	sbutton();
    
	virtual ~sbutton();
    static class sbutton * CreateSButton(struct sbutton_prefs *prefs)  ;
    static class sbutton * CreateFilledSButton(struct sbutton_prefs *prefs, const struct sbutton_list *blist)  ;
    static struct sbutton_prefs * GetNewPrefs(const char *name)  ;
    static void InitPrefs(struct sbutton_prefs *prefs, const char *name);
    static void FreePrefs(struct sbutton_prefs *prefs);
    static void WritePrefs(FILE *fp, struct sbutton_prefs *prefs);
    static struct sbutton_prefs * DuplicatePrefs(struct sbutton_prefs *prefs, const char *newname)  ;
    
  
     const char * ViewName()  ;
     long Read (FILE *fp, long id)  ;
     long Write (FILE *fp, long id, int level)  ;
     void NotifyObservers(long value);
    
  

    virtual void ActivateButton(int ind);
    virtual void DeActivateButton(int ind);

    virtual void Actuate(int ind);
    virtual void SetRock(int ind, long rock);
    virtual void SetLayout(int rows, int cols, enum sbutton_sizepolicy);
    virtual void SetLabel(int ind,const char *txt);
    virtual void SetLit(int ind, boolean litp);
    virtual void SetPrefs(int ind, struct sbutton_prefs *prefs);
    virtual int Enumerate(sbutton_efptr func,long rock)  ;
    virtual void SetTrigger(int ind, const char *name);
    virtual void Delete(int ind);
    virtual void Swap(int i1, int i2);
    virtual void SetChangeFlag(long on);
    virtual long GetChangeType()  ;
    virtual boolean EnsureSize(int ind)  ;
    virtual void SetSensitivity(int ind, boolean sensitive);
    
  
    static inline char *&GetName(struct sbutton_prefs *prefs) { return ((prefs)->name); }
    static inline int &GetStyle(struct sbutton_prefs *prefs) { return ((prefs)->style); }
    static inline struct fontdesc *&GetFont(struct sbutton_prefs *prefs) { return ((prefs)->font); }
    static inline const char *&GetColor(struct sbutton_prefs *prefs, int color) { return ((prefs)->colors[color]); }
    static inline const char *&GetLabelFG(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_LABELFG]); }
    static inline const char *&GetLabelBG(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_LABELBG]); }
    static inline const char *&GetInsensitiveLabelFG(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_INSENSITIVELABELFG]); }
    static inline const char *&GetInsensitiveLabelBG(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_INSENSITIVELABELBG]); }
    static inline const char *&GetBackground(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_BACKGROUND]); }
    static inline const char *&GetForeground(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_FOREGROUND]); }
    static inline const char *&GetTop(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_TOP]); }
    static inline const char *&GetInsensitiveTop(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_INSENSITIVETOP]); }
    static inline const char *&GetTopShadow(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_TOPSHADOW]); }
    static inline const char *&GetBottomShadow(struct sbutton_prefs *prefs) { return ((prefs)->colors[sbutton_BOTTOMSHADOW]); }
    static inline int &GetDepth(struct sbutton_prefs *prefs) { return ((prefs)->bdepth); }
    static inline int &GetRefCount(struct sbutton_prefs *prefs) { return ((prefs)->refcount); }
    
  
  
    inline long GetRock(int ind) { return (((ind)<(this)->count && (ind)>=0)?(this)->buttons[ind].rock:0); }
    inline boolean GetLit(int ind) { return (((ind)<(this)->count && (ind)>=0)?(this)->buttons[ind].lit:FALSE); }
    inline const char * GetLabel(int ind) { return (((ind)<(this)->count && (ind)>=0)?(this)->buttons[ind].label:NULL); }
    inline struct sbutton_prefs * GetPrefs(int ind) { return (((ind)<(this)->count && (ind)>=0)?(this)->buttons[ind].prefs:NULL); }
    inline class atom * GetTrigger(int ind) { return (((ind)<(this)->count && (ind)>=0)?(this)->buttons[ind].trigger:NULL); }

    inline boolean &GetSingleActivationFlag() { return ((this)->singleactivation); }
    inline struct sbutton_info * &GetButtons() { return ((this)->buttons); }
    inline int &GetRows() { return ((this)->rows); }
    inline int &GetCols() { return ((this)->cols); }
    inline  sbutton_sizepolicy &GetSizePolicy() { return ((this)->sizepolicy); }
    inline long &GetResizeRock() { return ((this)->resizerock); }
    inline sbutton_resizefptr &GetResizeFunc() { return ((this)->resizefunc); }
    inline sbutton_hitfptr &GetHitFunc() { return ((this)->hitfunc); }
    inline long &GetHitFuncRock() { return ((this)->hitfuncrock); }
    inline int &GetCount() { return ((this)->count); }
    inline struct sbutton_prefs *&GetDefaultPrefs() { return ((this)->prefs); }
    inline struct sbutton_prefs *&GetMattePrefs() { return ((this)->matteprefs); }
    inline int   &GetMaxCount() { return ((this)->maxcount); }
  
    /* Resizefunc is called as resizefunc(self, resizerock) when the dimensions of the button box are insufficient for its content. NOTE: this has nothing to do with its physical dimensions on screen, only with how many rows and columns the buttons are broken up into. */
    sbutton_resizefptr resizefunc;
    long resizerock;
    /* Hitfunc is called as hitfunc(self, hitfuncrock, buttonindex, buttonrock) called when a mouse button is released over an activated button. Button is the position of the button's information in the buttons array. */
    sbutton_hitfptr hitfunc;
    long hitfuncrock;
    
    int maxcount;
    int count;
    enum sbutton_sizepolicy sizepolicy;
    int rows,cols;
    struct sbutton_info *buttons; /* pointer to an array of count sbutton_info structures or null if no buttons have been created */
    int triggerchange; /* the index of the last button whose trigger changed */
    long change; /* a bitwise or of the change #defines */
    int activated; /* which button is currently active */
    boolean singleactivation; /* true if only one button may be active at a time */
    struct sbutton_prefs *prefs; /* the default set of preferences to use */
    struct sbutton_prefs *matteprefs; /* the prefs to use for the matte */
};
/** @} */
#endif /* _sbutton_H_ */
