#ifndef _AVarSet_H_
#define _AVarSet_H_
ATK_INTER
/* ***************************************************
 *	   Copyright Carnegie Mellon, 1995 - All Rights Reserved
 *        For full copyright information see:'andrew/doc/COPYRITE'
 *  ***************************************************
 */

/*
	avarset.H   -  AVarSet data object

	The file format for an AVarSet begins and ends with 
	the standard \begindata \enddata.  Intervening lines are 
	in extended resource file format:  
		The colon may be followed by a type name 
		in parentheses.  The typename must correspond 
		to a suffix for ASlot.
	The first line may be of the form:
		!Class:resourcelassname
	where resourceclassname.wgt is a file name on the
	CLASSPATH, or a CLASSPATH element /widgets/
	There will later be a line of the form
		+local
	this is followed by data for the specific class derived from
	avarset.

    Fetching initial resources
	Resources can be initially set from a file in 
	CLASPATH-element/{,widgets/}<ClassName>.wgt
	This is done by calling
		SetResourceClass(atom)
		FetchInitialResources()
	or simply
		FetchInitialResources(atom)
	FetchInitialResources also inherits values from parents
	in the view tree.

	The Read() method will call FetchInitialResources() if
	it has not been called or there is a "resourceclass" entry
	as the first resource in the datastream.  Its value is the
	resourceclassname.  If there is no leading "resourceclass" 
	line and SetResourceClass() has not been called, the name
	is taken from the class name of the object.
	If a FetchInitialResources for another class has been called
	and the datastream sets another resourceclass name, 
	FetchInitialResources will be called again for the new name;
	any vars from the first FetchInitialResources will remain, 
	but can be overridden.

 */


/* status values for NotifyObservers */
#define AVarSet_DATACHANGED 1

#include <mflex.H>
#include <atom.H>
#include <aslot.H>
#include <aformula.H>

struct proctable_Entry;
class aaction;
class figure;
class AVarSet;
DEFINE_MFLEX_CLASS(ASlot_Flex, ASlot *, 0);
DEFINE_OFLEX_CLASS(ASlot_PlaceHolderFlex, ADependerPlaceHolder,0);

typedef boolean AVarSet_esfptr(AVarSet *vars, ASlot *slot, void *rock);
	
#include <dataobject.H>
class AVarSet : public  dataobject 
{
  private:
    const atom *resourceclass;
	// `resourceclass' is used to decide what file to load to get the
	// initial resources. 
	// SetResourceClass sets the value to either a supplied name
	// or the default value, which is the name of the derived class. 
	// The initial value from the constructor is `WidgetSelector`.

    boolean fetchedresources;
	// false until FetchInitialResources has been called

    ASlot_Flex addedslots;	// ptrs to slots that are not members
    ASlot_Flex localslots; 	// ptrs to slots that are members
    ASlot_PlaceHolderFlex refedslots; // non existent slots referenced by formulas.
    ASlot *CheckAndReport(ASlot_NameParam name, 
		ATKregistryEntry *ent, 
		const char *readableclassname);
	// finds or creates a slot of name `name' and type `ent'
	// `readableclassname' is `ent's classname without 
	// the leading "ASlot".  If the name refers to a local slot, 
	// its type will be unchanged and may differ from `ent'.

  public:
    virtual ATKregistryEntry *ATKregistry();
    AVarSet();
    virtual ~AVarSet();

    long Read(FILE *file, long id);
    long Write(FILE *file, long id, int level);

    virtual void *ParseLocalDataLine(char *line, void *state);
		// a derived class should override this method
		// It will be called for each line written by 
		// WriteLocalData()
		// Initially 'state' will be NULL
		// On subsequent calls it will have whatever value was 
		// returned by the call for the previous line.
		// When there are no more lines, the method will be
		// called with NULL for the 'line' value.

    virtual void WriteLocalData(FILE *file);
		// a derived class should override this method
		// and write lines for its private data
		// no line should begin with '\'
		// all lines should be printable ASCII and under 79 bytes

    inline virtual void SetResourceClass(const atom *name)
		{resourceclass = name;}
    inline virtual const atom *GetResourceClass() 
		{return resourceclass;}

    virtual int FetchInitialResources(const atom *name=NULL);
	    // If 'name' is given, it is used to set the resourceclassname.
	    // FetchInitialResources reads like named resource files and
	    //	adds their resource to the Slots of this AVarSet.
	    // If the name already exists in the dataobject,
	    //	replace its value and complain about any type conflict.
	    // Eventually will also fetch resources from 
	    //	ancestors and the command line.
	    // returns number of errors detected

    virtual int FetchResources(FILE *fp);
	    // does the actual working of fetching resources 
	    // 	and creating slots
	    // returns number of errors detected
    
    int FetchResourcesFromPath(const char *filenm);
	    // Fetches the resources for the given widget name.
    
    virtual ASlot &Baptize(ASlot &v, 
			   ASlot_NameParam name=(const atom *)NULL,
			   boolean fromresource=FALSE);
	    // v is a member of this object
	    // Baptize gives v a name and adds it to LocalSlots
	    // (xxx this should be done with a table ala X)
            // returns *v so init with = can be chained on
            // fromresource is TRUE iff the value should be considered
            // as if coming from a resource file.

    virtual ASlot *LocateSlot(ASlot_NameParam name);  
	    // returns ptr to slot for name
	    // does not create slot
    virtual ASlot *AddSlot(ASlot_NameParam name, 
			const char *typenm);  
	    // returns ptr to slot for name and type  
	    //  (typenm should not have its leading "ASlot"
	    // 	creates slot if needed
    virtual void RemoveSlot(ASlot_NameParam name);
	    // removed the named slot if it exists in the added slots

    virtual void EnumerateSlots(AVarSet_esfptr *f, void *rock);
	    // calls f once for each slot in the varset
	    // stops iteration if a call returns FALSE
	    // call is f(this-avarset, slot, rock)

    virtual void CopySlots(AVarSet &dest, boolean fromresources=FALSE);
	    // Copy all the values from this AVarSet to dest.
	    // If fromresources is TRUE todatastream will be cleared and fromresources
	    // set on each slot copied over.
    
    // define Set functions     
    // ->Set will add a slot if none of that name exists
	    // returns the object, so further Sets can be chained on
#define AVSSET(argtype, reg, typenm)  \
 		AVarSet &Set(ASlot_NameParam name, argtype v)
    // NO SPACES AFTER COMMAS HERE
        AVSSET(fontdesc *,fontslotreg,Font);
        AVSSET(abool,boolslotreg,Bool);
	AVSSET(long,intslotreg,Int);
	AVSSET(int,intslotreg,Int);
	AVSSET(double,realslotreg,Real);
	AVSSET(const char *,stringslotreg,String);
	AVSSET(void *,ptrslotreg,Ptr);
	AVSSET(ATK *,atkslotreg,ATK);
	AVSSET(aaction *,functionslotreg,Function);
	AVSSET(proctable_Entry *,functionslotreg,Function);
	AVSSET(dataobject *,dataobjslotreg,DataObj);
	AVSSET(figure *,figureslotreg,Figure);
#undef AVSSET

	AVarSet &Set(ASlot_NameParam name, ASlot *v);
	virtual inline short Get(struct atom*name, struct atom**rtype, long *val) {
	    return dataobject::Get(name,rtype,val);
	}
    ASlot *Get(ASlot_NameParam name);
	    // gets the slot with the given name
	    // does linear lookup in both localslots and addedslots (xxx)
	    // if not found, returns NULL.
    long Get(ASlot_NameParam name, long def);
	    // Get for an integer with a default value.
    inline long Get(ASlot_NameParam name, int def)
		{ return Get(name, (long)def); }
    double Get(ASlot_NameParam name, double def);
	    // Get for a real with a default value.
    const char *Get(ASlot_NameParam name, const char *def);
    // Get for a C string with a default value.
    const void *Get(ASlot_NameParam name, const void *def);
    const color &Get(ASlot_NameParam name, const color &def);
    // Get for a color, with a default color.
    fontdesc *GetFont(ASlot_NameParam name, fontdesc *def);
    // Get a font.

    ATK *GetATK(ASlot_NameParam name, ATKregistryEntry *type=NULL);
	    // GetATK gets a pointer to an ATK object.  
	    // If type is given, this function is guaranteed 
	    // to return NULL or an object with the indicated type.
    aaction *Get(ASlot_NameParam name, aaction *def);
    dataobject *GetDataObj(ASlot_NameParam name, ATKregistryEntry *type=NULL);
	    // Gets a pointer to a dataobject, or NULL if the value 
	    // is not a dataobject.  type may be used to further restrict
	    // the actual type allowed.
    figure *GetFigure(ASlot_NameParam name, ATKregistryEntry *type=NULL);
	    // Gets a pointer to a figure, or NULL if the value is not a 
	    // dataobject.  type may be used to further restrict
	    // the actual type allowed.
};

/*
 *    $Log: avarset.H,v $
 * Revision 1.20  1996/11/06  20:18:58  robr
 * Support of an AVarSet::Set(name, font) method.
 * FEATURE
 *
 * Revision 1.19  1996/10/24  16:35:41  robr
 * added Get method for void *s
 * BUG
 *
 * Revision 1.18  1996/10/15  00:29:34  robr
 * Added automatic loading of the class referred to in a !Class: entry.
 * Added Set method taking an abool.
 * BUG
 *
 * Revision 1.17  1996/06/19  17:57:12  robr
 * Added support for recording references to non-existent slots so formulas
 * can depend on such references.
 *
 * Revision 1.16  1996/05/22  19:34:15  robr
 * Fixed uninitialized memory reference and added Get methods for colors
 * and fonts.
 * BUG
 *
 * Revision 1.15  1996/05/20  17:04:47  robr
 * Fixed warnings.
 *
 * Revision 1.14  1996/05/07  20:36:05  wjh
 * some reformatting; \
 * define Get for int type (works on RS/6000 and Sun, anyway)
 *
 * Revision 1.13  1996/05/02  02:47:01  wjh
 * added conversion from int to slot so L is not needed after integers
 *
 * Revision 1.12  1996/04/30  19:01:30  robr
 * Changed CopySlots to allow the copied resources to be marked as
 * 'fromresources'.
 * BUG
 *
 * Revision 1.11  1996/04/29  21:51:24  robr
 * Made FetchFromResources a member function.
 * Added CopySlots to copy resources values from one AVarSet to another.
 * FEATURE
 *
 * Revision 1.10  1996/04/23  21:32:54  robr
 * Added argument to baptize to control whether the slot
 * is treated as if set from a resource file.
 * Also arranged for resources read from a datastream to
 * be written back out to the datastream.
 * BUG
 *
 * Revision 1.9  1996/04/09  20:15:21  wjh
 * pointless copying to make two AVSSET groups identical
 * ,
 *
 * Revision 1.8  1996/04/06  13:48:17  wjh
 * fixed RemoveSlot decl
 *
 * Revision 1.7  1996/04/05  20:38:21  wjh
 * include aslot.H for ASlot_NameParam
 *
 * Revision 1.6  1996/04/05  19:38:17  wjh
 * added ASlot_NameVar and ASlot_NameParam
 *
 * Revision 1.5  1996/04/03  20:32:18  wjh
 * changed a few methods to virtual
 *
 * Revision 1.4  1996/04/02  21:13:22  wjh
 * more const atom crap
 *
 * Revision 1.3  1996/03/28  20:32:27  robr
 * Removed bogus default return value from plain Get(atom), it
 * now returns NULL if the desired resource was not found.
 * BUG
 *
 * Revision 1.2  1996/03/28  19:03:53  robr
 * Updated to use atom_defs in place of atom *s when possible
 * (Ifdefed on ATOM_DEF_HAS_CONST until the new atom.H is available
 * everywhere this code will be worked on.)
 * Added new Get methods which take defaults to select the target type..
 * Also added GetATK, GetDataObj, and GetFigure methods.
 * XXX Did NOT remove the bogus default return from Get(atom) should I? XXX
 * FEATURE
 *
 * Revision 1.1  1996/03/22  19:32:55  wjh
 * Initial revision
 *
 * Revision 1.1  1995/09/23  15:18:47  wjh
 * Initial revision
 * 
 * Revision 1.0  95/09/05  16:23:22  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
*/
#endif /* _AVarSet_H_ */
