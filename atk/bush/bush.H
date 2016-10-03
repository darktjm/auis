#ifndef _bush_H_
#define _bush_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbush Object bush
 *  A dynamically loadable ATK class file.  Contains bush, bushv, bushapp.
 * @{ */
/***  SPECIFICATION -- External Facility Suite  *********************************

TITLE	The Bush Data-object

MODULE	bush.ch

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Bush Data-object.

    NB: The comment-symbol "===" indicates areas which are:
	    1 - Questionable
	    OR
	    2 - Arbitrary
	    OR
	    3 - Temporary Hacks
    Such curiosities need be resolved prior to Project Completion...


HISTORY
  08/21/85	Created (TCP)
  01/15/89	Convert to ATK (GW Keim)

END-SPECIFICATION  ************************************************************/


#include <tree.H>

struct entry_mode {
    unsigned int do_rescan:1;
    unsigned int destroyed:1;
    unsigned int selected:1;
    unsigned int detailed:1;
    unsigned int stat_failed:1;
    unsigned int scan_failed:1;
};

struct entry_kind {
    unsigned int dir:1;
    unsigned int soft_link:1;
    unsigned int filevar:1;
};

struct Dir_Entry {
    char *name;
    char *link_name;
    struct entry_kind type;
    struct entry_mode mode;
    size_t size;
    time_t time_stamp;
    char *owner;
    int nlinks;
    unsigned permissions;
    int position;
    tree_type_node parent, instance;
};

struct Dir_Entries {
    int count;
    struct Dir_Entry **Entry;
};

struct Dir_ {
    time_t time_stamp;
    char *path, *name;
    struct entry_mode mode;
    tree_type_node tn;
    struct Dir_Entries *Dir_Entries;
};

#include <apt.H>
#include <attribs.h>
class bush : public  apt {
public:
	virtual ATKregistryEntry *ATKregistry();

    

  bush();
  virtual ~bush();
	static class bush * Create(const char *init_dir)  ;


	/* dired compatibility */
	virtual void SetAttributes(struct attributes *attributes);

	virtual void InitTree(const char *root_path)  ;
	virtual int ScanDir(tree_type_node tn)  ;
	virtual void BuildSubDirs(tree_type_node tn)    ;
	virtual int DestroyEntry(tree_type_node tn,struct Dir_Entry *Entry)  ; 
	virtual boolean ScanRequired(tree_type_node tn)  ;
	virtual int RenameDir(tree_type_node tn,char *newPath, char *newName)  ;
	virtual int MoveEntry(tree_type_node tn,struct Dir_Entry *Entry,char *newName)  ;
	virtual int PerformSystemAction( const char* name, const char **argv )  ;

    
  	 long Read(FILE *file,long id)  ;
  	 long Write(FILE *file,long id,int level)  ;
	 const char * ViewName()  ;

    

	inline class tree * Tree() { return (this->tree); }
	inline struct tree_node * TreeRoot() { return (((this)->Tree())->RootNode()); }
	inline char * RootPathName() { return (this->root_pathname); }
	inline char * GivenRootPathName() { return (this->given_dir_name); }
	inline struct Dir_ * Dir(tree_type_node tn) { return ((struct Dir_ *)((this)->Tree())->NodeDatum(tn)); }
	inline struct entry_mode  DirMode(tree_type_node tn) { return ((this)->Dir(tn)->mode); }
	inline char * DirPath(tree_type_node tn) { return ((this)->Dir(tn)->path); }
	inline char * DirName(tree_type_node tn) { return ((this)->Dir(tn)->name); }
	inline time_t DirTimeStamp(tree_type_node tn) { return ((this)->Dir(tn)->time_stamp); }
	inline char * RootDirPath() { return ((this)->DirPath((this)->TreeRoot())); }
	inline struct Dir_Entries * DirEntries(tree_type_node tn) { return ((this)->Dir(tn)->Dir_Entries); }
	inline int DirEntriesCount(tree_type_node tn) { return ((this)->DirEntries(tn)->count); }
	inline struct Dir_Entry ** DirEntryPtr(tree_type_node tn) { return ((this)->DirEntries(tn)->Entry); }
	inline struct Dir_Entry * DirEntry(tree_type_node tn,int i) { return ((this)->DirEntries(tn)->Entry[i]); }
	inline struct entry_mode  DirEntryMode(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->mode); }
	inline int DirEntryPos(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->position); }
	inline char * DirEntryName(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->name); }
        inline char * DirEntryLinkName(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->link_name); }
	inline struct entry_kind DirEntryType(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->type); }
	inline char * DirEntryOwner(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->owner); }
	inline int DirEntryNLinks(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->nlinks); }
	inline time_t DirEntryTimeStamp(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->time_stamp); }
	inline size_t DirEntrySize(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->size); }
	inline unsigned int DirEntryPerms(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->permissions); }
	inline tree_type_node DirEntryParent(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->parent); }
	inline tree_type_node  DirEntryInstance(tree_type_node tn,int i) { return ((this)->DirEntry(tn,i)->instance); }
	inline struct Dir_ * DirEntryParentDir(tree_type_node tn,int i) { return ((this)->Dir((this)->DirEntryParent(tn,i))); }
	inline struct Dir_ * RootDir() { return ((this)->Dir((this)->TreeRoot())); }
	inline tree_type_node Parent(tree_type_node tn) { return (((this)->Tree())->ParentNode(tn)); }
	inline tree_type_node Child(tree_type_node tn) { return (((this)->Tree())->ChildNode(tn)); }
	inline tree_type_node Left(tree_type_node tn) { return (((this)->Tree())->LeftNode(tn)); }
	inline tree_type_node Right(tree_type_node tn) { return (((this)->Tree())->RightNode(tn)); }
	inline struct Dir_ * ParentDir(tree_type_node tn) { return ((this)->Dir((this)->Parent(tn))); }
	inline struct Dir_ * ChildDir(tree_type_node tn) { return ((this)->Dir((this)->Child(tn))); }
	inline struct Dir_ * LeftDir(tree_type_node tn) { return ((this)->Dir((this)->Left(tn))); }
	inline struct Dir_ * RightDir(tree_type_node tn) { return ((this)->Dir((this)->Right(tn))); }

    

	char		 given_dir_name[1025];
	class tree	*tree;
	char		*root_pathname;
	class vector	*uid_uname_map;
	int		 debug;
};
/** @} */
#endif /* _bush_H_ */
