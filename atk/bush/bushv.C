/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
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

TITLE	The Bush View-object

MODULE	bushv.c

VERSION	0.0

AUTHOR	TC Peters & GW Keim
	Information Technology Center, Carnegie-Mellon University 

DESCRIPTION
	This is the suite of Methods that support the Bush View-object.

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

END-SPECIFICATION  ******************************************************/

#include <andrewos.h>
ATK_IMPL("bushv.H")
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <util.h>
#include <rect.h>
#include <attribs.h>
#include <dataobject.H>
#include <event.H>
#include <keystate.H>
#include <keymap.H>
#include <filetype.H>
#include <environ.H>
#include <fontdesc.H>
#include <proctable.H>
#include <bind.H>
#include <menulist.H>
#include <cursor.H>
#include <im.H>
#include <view.H>
#include <lpair.H>
#include <suite.H>
#include <textview.H>
#include <text.H>
#include <treev.H>	/* tree.ih is included in bush.ih */
#include <tree.H>
#include <completion.H>
#include <color.H>
#include <windowsystem.H>
#include <apts.H>
#include <bush.H>
#include <bushv.H>

#define	by_name					    0
#define	by_size					    1
#define	by_date					    2
#define	by_suffix				    3
#define	by_type					    4

#define	nodes_object				    1
#define	entries_object				    2
#define	entry_object				    4

#define	bushv_DefaultMenus			    (1<<6)
#define	bushv_EntryMenus			    (1<<7)
#define	bushv_RWEntryMenus			    (1<<8)
#define bushv_EntriesMenus			    (1<<9)

#define	DEFAULTCKPINTERVAL			    120
int CkpInterval;

static const char * const sorts[] = { "name", 
			     "size", 
			     "date", 
			     "suffix", 
			     "type", 
			      NULL };


static const char * const default_editor_choices[] = { "ez",
					      "emacs",
					      "gnu-emacs",
					      "zip",
					      "raster",
					      "table",
                                              "other",
					       NULL};

static char			    msg[MAXPATHLEN * 2];
static char			    cmd[MAXPATHLEN * 2];
static const char		   *argv[10];

static class keymap		   *kmap;
static class menulist		   *bushv_menulist = NULL;


#define BUSH			    (self->bush)
#define ControlView		    (self->control_view)
#define	EntriesView		    (self->entries_view)
#define DirTreeView		    (self->dir_tree_view)
#define EntryView		    (self->entry_view)
#define	EntryViewAppLayer	    (self->entry_view_application_layer)
#define	EntryObject		    (self->entry)
#define	EntryObjectModified	    (self->entry_object_modified)
#define	EntryObjectLastCKP	    (self->entry_object_last_checkpoint)
#define	EntryFilep		    (self->entry_filep)
#define NumPrevSelected		    (self->num_prev_selected)
#define LP			    (self->lp)
#define SortMode		    (self->sortmode)
#define Object			    (self->object)
#define Kmap			    (self->kmap)
#define Kstate			    (self->keyStatep)
#define Menulist		    (self->menulistp)
#define InitNode		    (self->initial_node)
#define CurrNode		    (self->current_node)
#define	MoveNode		    (self->move_node)
#define CurrEntry		    (self->current_entry)
#define Cursor			    (self->cursorp)
#define	Detail			    (self->detail)
#define TopLevelInset		    (self->top_level_inset)
#define Debug			    (self->debug)
#define	EditorProgram		    (self->editor_program)
#define EditorChoices		    (self->editor_choices)
#define EditorIndex		    (self->editor_index)
#define	NumEditorChoices	    (self->num_editor_choices)

#define	TREE			    ((BUSH)->Tree())
#define	RootDirPath		    ((BUSH)->RootDirPath())
#define	RootPathName		    ((BUSH)->RootPathName())
#define	TreeRoot		    ((BUSH)->TreeRoot())
#define DIR(tn)			    ((BUSH)->Dir(tn))
#define	ScanRequired(tn)	    ((BUSH)->ScanRequired(tn))
#define	DIRMODE(tn)		    (DIR(tn)->mode)
#define	DirPath(tn)		    (DIR(tn)->path)
#define	DirName(tn)		    (DIR(tn)->name)
#define	DirTimeStamp(tn)	    (DIR(tn)->time_stamp)
#define	DirEntries(tn)		    (DIR(tn)->Dir_Entries)
#define	DirEntriesCount(tn)	    (DirEntries(tn)->count)
#define	DirEntryPtr(tn)		    (DirEntries(tn)->Entry)
#define	DirEntry(tn,i)		    (DirEntryPtr(tn)[i])
#define	DirEntryMode(tn,i)	    (DirEntry(tn,i)->mode)
#define	DirEntryPos(tn,i)	    (DirEntry(tn,i)->position)
#define	DirEntryName(tn,i)	    (DirEntry(tn,i)->name)
#define	DirEntryLinkName(tn,i)	    (DirEntry(tn,i)->link_name)
#define	DirEntryType(tn,i)	    (DirEntry(tn,i)->type)
#define	DirEntryOwner(tn,i)	    (DirEntry(tn,i)->owner)
#define	DirEntryNLinks(tn,i)	    (DirEntry(tn,i)->nlinks)
#define	DirEntryTimeStamp(tn,i)	    (DirEntry(tn,i)->time_stamp)
#define	DirEntrySize(tn,i)	    (DirEntry(tn,i)->size)
#define	DirEntryPerms(tn,i)	    (DirEntry(tn,i)->permissions)

#define	Parent(tn)		    ((TREE)->ParentNode(tn))
#define	Child(tn)		    ((TREE)->ChildNode(tn))
#define	Left(tn)		    ((TREE)->LeftNode(tn))
#define	Right(tn)		    ((TREE)->RightNode(tn))

#define EntrySelected(entry)	    ((entry)->mode.selected)
#define EntryDirType(entry)	    ((entry)->type.dir)
#define EntryLinkType(entry)	    ((entry)->type.soft_link)
#define EntryPerms(entry)	    ((entry)->permissions)
#define	EntryLinkName(entry)	    ((entry)->link_name)

#define SetTreeNotificationData(self, node, code)	\
    (TREE)->notification_node = node;			\
    (TREE)->notification_code = code

#define NotifyTreeObservers(self) (TREE)->NotifyObservers(0)

#define	AllocNameSpace(s,t)	    apts::CaptureString(s,t)
#define	Announce(msg_string)	    (self)->Announce((char *)(msg_string))
#define ClearMessageLine()	    if(!message::Asking(self)) Announce("")

#define	edit_code		    1
#define	exec_code		    2
#define	print_code		    3
#define	editor_code		    4
#define	sort_code		    5
#define	pop_code		    6
#define	detail_code		    7
#define	destroy_code		    8
#define	switch_code		    9
#define	rescan_code		    10
#define	create_code		    11
#define	rename_code		    12

struct item_data {  /* item data for the ControlView buttons */
    long code;	    /* item index */
    long activate;  /* mask that determines under what state the item is active */
};

static struct item_data	
  edit_data =	 {edit_code, entries_object | entry_object},
  exec_data =	 {exec_code, entries_object | entry_object},
  print_data =	 {print_code, nodes_object | entries_object | entry_object},
  editor_data =	 {editor_code, nodes_object | entries_object | entry_object},
  sort_data =	 {sort_code, nodes_object | entries_object},
  pop_data =	 {pop_code, entries_object | entry_object},
  detail_data =	 {detail_code, nodes_object | entries_object},
  destroy_data = {destroy_code, nodes_object | entries_object | entry_object},
  switch_data =	 {switch_code, nodes_object | entries_object | entry_object},
  rescan_data =	 {rescan_code, nodes_object | entries_object | entry_object},
  create_data =	 {create_code, nodes_object | entries_object},
  rename_data =	 {rename_code, nodes_object | entries_object | entry_object};

static suite_Specification edit[] = {
    suite_ItemCaption("Edit"),
    suite_ItemDatum(&edit_data), 
    0 };
static suite_Specification exec[] = {
    suite_ItemCaption("Exec"), 
    suite_ItemDatum(&exec_data), 
    0 };
static suite_Specification print[] = {
    suite_ItemCaption("Print"), 
    suite_ItemDatum(&print_data), 
    0 };
static suite_Specification editor[] = {
    suite_ItemCaption("Editor"), 
    suite_ItemDatum(&editor_data), 
    0 };
static suite_Specification sort[] = {
    suite_ItemCaption("Sort: name"), 
    suite_ItemDatum(&sort_data), 
    0 };
static suite_Specification pop[] = {
    suite_ItemCaption("Pop"), 
    suite_ItemDatum(&pop_data), 
    0 };
static suite_Specification detail[] = {
    suite_ItemCaption("Detail: off"), 
    suite_ItemDatum(&detail_data), 
    0 };
static suite_Specification destroy[] = {
    suite_ItemCaption("Destroy"), 
    suite_ItemDatum(&destroy_data), 
    0 };
static suite_Specification Switch[] = {
    suite_ItemCaption("Switch"), 
    suite_ItemDatum(&switch_data), 
    0 };
static suite_Specification rescan[] = {
    suite_ItemCaption("ReScan"), 
    suite_ItemDatum(&rescan_data), 
    0 };
static suite_Specification create[] = {
    suite_ItemCaption("Create"), 
    suite_ItemDatum(&create_data), 
    0 };
static suite_Specification rename_it[] = {
    suite_ItemCaption("ReName"), 
    suite_ItemDatum(&rename_data), 
    0 };

ATKdefineRegistry(bushv, aptv, bushv::InitializeClass);
static void PostCursor( register class bushv    *self, register int		    type );
static long ResetSelectedState( register class bushv		 *self, register class suite		 *suite, register struct suite_item	 *item, register long		  datum );
static class view * EntriesHitHandler( register class bushv		 *self, register class suite		 *suite, register struct suite_item	 *item, register long			  object, register enum view_MouseAction  action, register long			  x , register long			  y , register long			  numClicks );
static void StartDirMove( register class bushv     *self, register tree_type_node    tn );
static int FinishDirMove( register class bushv	     *self, register tree_type_node     tn );
static class view * TreeHitHandler( register class bushv		     *self, register class treev		     *tree_view, register tree_type_node	      node, register long			      object, register enum view_MouseAction      action, register long			      x , register long			      y , register long			      numClicks );
static class view * ControlHitHandler( register class bushv		     *self, register class suite		     *suite, register struct suite_item	     *item, register long			      object, register enum view_MouseAction      action, register long			      x , register long			      y , register long			      numClicks );
static const char * FileSuffix( register const char    *file_name );
static const char * FileType( register const char	     *file_name );
long SortByName( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 );
long SortBySuffix( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 );
long SortBySize( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 );
long SortByDate( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 );
long SortByType( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 );
static void GetPreferredEditors( register class bushv  *self );
static void GetPreferredFonts( register class bushv	 *self );
static void GetPreferences( register class bushv	 *self );
static void DoEdit( class bushv  *self, char  *path , char  *name  );
static void PerformEdit( class bushv  *self );
static void PerformPrint( class bushv	 *self );
static void DoPrint( class bushv  *self, char  *path , char  *name  );
static void PerformCreate( class bushv  *self );
static void IssueError( register class bushv  *self, register const char  *what , register const char  *where, boolean  overlay );
static int DoDestroy( register class bushv  *self, register tree_type_node  tn, register struct Dir_Entry  *Entry, int  overlay );
static void PerformDestroy( class bushv  *self );
static void PerformExec( class bushv  *self );
static void DoExecute( class bushv		 *self, tree_type_node	  tn , struct Dir_Entry	 *Entry  );
static int PerformSystemAction( class bushv	 *self, const char		 *name, const char		 *argv[], char		 *msg );
static char* Format_Tags( u_short	     tag );
static char* FormatEntriesItem( register class bushv	 *self, tree_type_node	  tn, int			  i, struct Dir_Entry	 *dirEntry );
static void ResetEntriesCaptions( class bushv  *self );
static int HandleModifiedObject( register class bushv	     *self );
static void PerformExit( class bushv	 *self );
static void SwitchDirectory( class bushv	     *self );
static void SetEditor( class bushv	 *self );
char * FormatEntriesInfo( class bushv  *self, tree_type_node  tn );
static suite_sortfptr DetermineSortHandler( class bushv		 *self, tree_type_node	  tn );
static void DoAutoRescan( register class bushv		  *self );
static void PushToEntries( class bushv	       *self );
static void PushToEntry( class bushv  *self );
static int PopToNodes( class bushv	 *self );
static int PopToEntries( class bushv	 *self );
static long Passivator( class bushv	     *self, class suite	     *suite, struct suite_item  *item, unsigned	      datum );
static void PassivateControls( class bushv     *self );
static void Push( register class bushv	     *self );
static void Pop( class bushv     *self );
static void PerformPop( class bushv	 *self );
static long ToggleCaptionDetail( class bushv		 *self, class suite		 *suite, struct suite_item	 *item, unsigned		  datum );
static void SortDir( class bushv  *self, tree_type_node  tn );
static void PerformDetail( class bushv  *self );
static int SortRequested( class bushv  *self, tree_type_node  tn );
static void PerformSort( class bushv  *self );
static void PerformRescan( class bushv  *self );
static long ResetChildDirPaths( class bushv       *self, class tree	     *tree, tree_type_node      tn, long		      datum );
static void PerformRename( class bushv  *self );
static void ToggleDebug( class bushv     *self );
static void HandleChangeDir( register class bushv    *self, register char		   *dirName );
static int  bushv_WriteToFile( class bushv   *self, char		 *filename );
static int bushv_SaveFile( class bushv	 *self );
static int bushv_WriteFile( class bushv	 *self );
static int bushv_SetPrinter(class bushv	 *self);
static void  Checkpoint( long  dummyData );
static void UpdateDetailCaption( class bushv		     *self );
static void EntriesPageUp( class bushv  *self );
static void EntriesPageDown( class bushv  *self );


suite_Specification control_spec[] = {
    suite_TitleCaption( "Bush: A FileSystem Browser" ),
    suite_TitlePlacement( suite_Top ),
    suite_TitleCaptionFontName( "AndySans10" ),
    suite_TitleBorderStyle( suite_Rectangle ),
    suite_Item(edit),
    suite_Item(editor),
    suite_Item(exec),
    suite_Item(print),
    suite_Item(pop),
    suite_Item(Switch),
    suite_Item(sort),
    suite_Item(detail),
    suite_Item(create),
    suite_Item(destroy),
    suite_Item(rescan),
    suite_Item(rename_it),
    suite_ItemCaptionFontName( "AndySans10b" ),
    suite_TitleBorderSize( 2 ),
    suite_GutterSize( 1 ),
    suite_BorderSize( 2 ),
    suite_ItemBorderSize( 2 ),
    suite_ItemOrder( suite_ColumnMajor ),
    suite_Arrangement( suite_Matrix | suite_Fixed ),
    suite_Columns( 6 ),
    suite_Rows( 2 ),
    suite_SelectionMode( suite_Exclusive ),
    suite_HitHandler( ControlHitHandler ),
    0
};

suite_Specification entries_spec[] = {
    suite_TitleCaption( "Entries" ),
    suite_TitleBorderSize( 2 ),
    suite_TitleBorderStyle( suite_Rectangle ),
    suite_BorderStyle( suite_None ),
    suite_Arrangement( suite_List | suite_Matrix | suite_Unbalanced | suite_ColumnLine ),
    suite_HorizontalGutterSize( 3 ),
    suite_ItemOrder( suite_ColumnMajor ),
    suite_ItemCaptionFontName( "AndyType12f" ),
    suite_TitleFontName( "Andy10" ),
    suite_ItemCaptionAlignment( suite_Left | suite_Center ),
    suite_ItemBorderStyle( suite_Invisible ),
    suite_ItemBorderSize( 1 ),
    suite_VerticalGutterSize( 0 ), 
    suite_SelectionMode( suite_Inclusive ),
    suite_SortHandler( SortByName ),
    suite_Scroll( suite_Left ),
    suite_HitHandler( EntriesHitHandler ),
    suite_CursorFontName( "aptcsr20" ),
    suite_Cursor( 'c' ),
    0
};

treev_Specification tree_spec[] = {
    treev_NodeConnectorStyle( treev_Fold | treev_DogLeg ), 
    treev_NodeFiligree( treev_DropShadow ),
    treev_NodeHighlightStyle( treev_Invert ),
    treev_Scroll( treev_Left ),
    treev_BackgroundShade( 50 ),
    treev_Cursor( 'z' ),
    treev_CursorFontName( "aptcsr20" ),
    treev_HitHandler( TreeHitHandler ),
    0
};


static void
PostCursor( register class bushv    *self, register int		    type )
    {
  struct rectangle	   r;

  (self)->GetVisualBounds(&r);
  (Cursor)->SetStandard(type);
  (self)->PostCursor(&r,Cursor);
}

/*  Called by static void EntriesHitHandler().
    Resets the Dir_Entry mode flag, associated with each suite_item, to 
    correspond with the items that are currently highlighted in the suite 
    (entries_object). We do this after the check for Pop conditions because we 
    need to determine the number of items previously highlighted for that check.
*/

static long
ResetSelectedState( register class bushv		 *self, register class suite		 *suite, register struct suite_item	 *item, register long		  datum )
{
  struct Dir_Entry	*dirEntry = NULL;
  long int		 status = 0;

  if(!suite || !item) return(status);
  dirEntry = (struct Dir_Entry*)
    (suite)->ItemAttribute(item,suite_itemdatum);
  if(!(suite)->ItemExposed(item) || 
     !(suite)->ItemHighlighted(item) || 
     !(suite)->ItemActivated(item)) 
	    EntrySelected(dirEntry) = 0;
  else EntrySelected(dirEntry) = TRUE;
  return(status);
}

static class view *
EntriesHitHandler( register class bushv		 *self, register class suite		 *suite, register struct suite_item	 *item, register long			  object, register enum view_MouseAction  action, register long			  x , register long			  y , register long			  numClicks )
            {
  register struct Dir_Entry	*dirEntry = NULL;
  register int			 numSelected = 0, count = 0, i = 0;

  IN(EntriesHitHandler);
  ClearMessageLine();
  if((object == suite_ItemObject) && item && 
	((action == view_LeftUp) || (action == view_RightUp))) {
    if(action == view_LeftUp) {
	dirEntry = (struct Dir_Entry *) (EntriesView)->ItemAttribute(item,suite_itemdatum);
	if(EntrySelected(dirEntry) && (NumPrevSelected == 1)) {
	  PostCursor(self,Cursor_Wait);
	  CurrEntry = dirEntry;
	  Push(self);
	  (self)->RetractCursor(Cursor);
	}
    }
    (suite)->Apply((suite_applyfptr) ResetSelectedState, (long int) self, 0);
    if(DirEntries(CurrNode)) {
      count = DirEntriesCount(CurrNode);
      for(i = 0 ; i < count ; i++)
        if(DirEntryMode(CurrNode,i).selected) 
	  numSelected++;
    }
    NumPrevSelected = numSelected;
    if(Object == entries_object) { /* may have Pop'ed back to nodes */
      if((numSelected > 1) || (numSelected == 0)) 
	sprintf(msg,"%d entries currently selected",numSelected);
      else sprintf(msg,"1 entry selected");
	Announce(msg);
    }
  }
  OUT(EntriesHitHandler);
  return(0);
}

static void
StartDirMove( register class bushv     *self, register tree_type_node    tn )
    {
  IN(StartDirMove);
  if(tn) {
    PostCursor(self,Cursor_Gunsight);
    sprintf(msg,"Moving directory '%s'...",DirName(tn));
    Announce(msg);
  }
  else Announce("No selected directory to move.");
  OUT(StartDirMove);
}

static int
FinishDirMove( register class bushv	     *self, register tree_type_node     tn )
    {
  register int		     status = 0;
  char			     finalLocation[MAXPATHLEN];

  IN(FinishDirMove);
  PostCursor(self,Cursor_Wait);
  if(!MoveNode) status = -1;
  else if(!tn) {
    sprintf(msg,"No target selected. Move operation cancelled.");
    Announce(msg);
    status = -1;
  }
  else if(tn == MoveNode) {
    sprintf(msg,"Cancelled.");
    Announce(msg);
    status = -1;
  }
  else if((TREE)->NodeAncestor(MoveNode,tn)) {
    sprintf(msg,"You cannot move a directory to one of its children.");
    Announce(msg);
    status = -1;
  }
  else if(tn == Parent(MoveNode)) {
    sprintf(msg,"'%s' already lives under '%s'",
	     DirName(MoveNode),DirName(tn));
    Announce(msg);
    status = -1;
  }
  else {
    sprintf(msg,"Moving '%s' under '%s'",DirName(MoveNode),
	DirName(tn));
    Announce(msg);
    sprintf(finalLocation,"%s/%s",DirPath(tn),
	     DirName(MoveNode));
    if((status = rename(DirPath(MoveNode),finalLocation))) {
      IssueError(self,"Moving",DirName(MoveNode),TRUE);
      sprintf(msg,"Move failed.");
      Announce(msg);
    }
    else {
      if((TREE)->NodeLevel(CurrNode) >= (TREE)->NodeLevel(MoveNode)) 
	  CurrNode = NULL;
      SetTreeNotificationData(self,MoveNode,tree_NodeDestroyed);
      NotifyTreeObservers(self);
      (TREE)->DestroyNode(MoveNode);
      sprintf(msg,"Move succeeded.");
      Announce(msg);
    }
  }
  (self)->RetractCursor(Cursor);
  OUT(FinishDirMove);
  return(status);
}

static class view *
TreeHitHandler( register class bushv *self, register class treev *tree_view, tree_type_node node, long object, register enum view_MouseAction action, register long x, register long y, register long	numClicks )
{
    register tree_type_node	     old_CurrNode = NULL;
    register tree_type_node	     peer = NULL;
    struct stat			     stats;

    IN(TreeHitHandler);
    if(object == treev_NodeObject) {
	if(node) (tree_view)->HighlightNode(node);
	if(action == view_RightDown) {
	    MoveNode = node;
	    StartDirMove(self,node);
	    return 0;
	}
	else if(action == view_RightUp) {
	    if(FinishDirMove(self,node)) {
		(tree_view)->HighlightNode(CurrNode);
		MoveNode = NULL;
		return 0;
	    }
	    MoveNode = NULL;
	    DIRMODE(node).do_rescan = 1;
	}
	else if(action != view_LeftUp || !node) return 0;
	ClearMessageLine();
	PostCursor(self,Cursor_Wait);
	if(stat(DirPath(node),&stats)) {
	    DIRMODE(node).stat_failed = 1;
	    IssueError(self,"Scanning",DirName(node),TRUE);
	    (self)->RetractCursor(Cursor);
	    return 0;
	}
	old_CurrNode = CurrNode;
	CurrNode = node;
	if(Parent(CurrNode))
	    peer = Child(Parent(CurrNode));
	else
	    peer = CurrNode;
	if(!DIRMODE(CurrNode).selected || (action == view_RightUp)) {
	    if(old_CurrNode) {
		DIRMODE(old_CurrNode).selected = 0;
		old_CurrNode = NULL;
	    }
	    while(peer) {
		if(Child(peer)) {
		    SetTreeNotificationData(self, peer, tree_NodeChildrenDestroyed);
		    NotifyTreeObservers(self);
		    (TREE)->DestroyNodeChildren(peer);
		    break;
		}
		peer = Right(peer);
	    }
	    DIRMODE(CurrNode).selected = 1;
	    if(ScanRequired(CurrNode)) {
		sprintf(msg, "Scanning '%s' ...", DirName(CurrNode));
		Announce(msg);
		im::ForceUpdate();
		(BUSH)->ScanDir(CurrNode);
		ClearMessageLine();
	    }
	    (BUSH)->BuildSubDirs(CurrNode);
	    SetTreeNotificationData(self, CurrNode, tree_NodeChildrenCreated);
	    NotifyTreeObservers(self);
	    (ControlView)->ChangeSuiteAttribute( suite_TitleCaption( DirPath(CurrNode) ) );
	}
	else Push(self);
	(self)->RetractCursor(Cursor);
    }
    OUT(TreeHitHandler);
    return(0);
}

static class view *
ControlHitHandler( register class bushv		     *self, register class suite		     *suite, register struct suite_item	     *item, register long			      object, register enum view_MouseAction      action, register long			      x , register long			      y , register long			      numClicks )
            {

  IN(ControlHitHandler);
  ClearMessageLine();
  if((action == view_LeftUp) || (action == view_RightUp)) {
    if(item && (object == suite_ItemObject)) {
      struct item_data	*itemData = NULL;

      if((itemData = (struct item_data*) (suite)->ItemAttribute( item, suite_itemdatum)))
        switch(itemData->code) {
	    case(edit_code):	PerformEdit(self);	break;
	    case(exec_code):	PerformExec(self);	break;
            case(print_code):	PerformPrint(self);	break;
            case(sort_code):	PerformSort(self);	break;
            case(destroy_code):	PerformDestroy(self);   break;
            case(rescan_code):	PerformRescan(self);    break;
            case(create_code):	PerformCreate(self);    break;
            case(pop_code):	PerformPop(self);	break;
            case(detail_code):	PerformDetail(self);    break;
            case(rename_code):	PerformRename(self);    break;
            case(editor_code):	SetEditor(self);	break;
            case(switch_code):	SwitchDirectory(self);  break;
	}
      PassivateControls(self);
    }
    else if(object == suite_TitleObject) {
      (suite)->HighlightTitle();
      PostCursor(self,Cursor_Wait);
      HandleChangeDir(self,DirPath(CurrNode));
#ifndef hp9000s800 /* The SIGALRM causes trouble on the hp800. */
      sleep(1);
#endif /* hp9000s800 */
      (self)->RetractCursor(Cursor);
      sprintf(msg,"Working directory changed to '%s'",
	       DirPath(CurrNode));
      Announce(msg);
      (suite)->NormalizeTitle();
    }
#if 0
    else if(object == suite_NoObject) {
      PassivateControls(self);
      if((suite)->CurrentItem()) 
        (suite)->PassivateItem((suite)->CurrentItem());
    }	
#endif
  }
  OUT(ControlHitHandler);
  return(0);
}

static const char *
FileSuffix( register const char    *file_name )
  {
  register const char   *suffix;

  if((suffix = strrchr(file_name,'.'))) suffix++;
  else suffix = file_name + strlen(file_name);
  return(suffix);
}

static const char *
FileType( register char *file_name )
  {
  static const char * const suffixes[] = {"BAK","CKP",0};
  const char * const *suffix_ptr;
  register char	    *suffix;

  suffix = strrchr(file_name,'.');
  if(!suffix) return(file_name + strlen(file_name));
  suffix++;
  suffix_ptr = suffixes;
  while(*suffix_ptr) {
    if(!strcmp(suffix,*suffix_ptr)) {
      *(suffix-1) = '\0';
      return(FileSuffix(file_name));
    }
    suffix_ptr++;
  }
  return(suffix);
}

long
SortByName( register class bushv *self, register class suite *suite, register struct suite_item *e1, register struct suite_item *e2 )
      {
  register struct Dir_Entry	*a = NULL, *b = NULL;
  register long			 status = 0;

  if(!e1 || !e2) return(0);
  a = (struct Dir_Entry*)(suite)->ItemAttribute(e1,suite_itemdatum);
  b = (struct Dir_Entry*)(suite)->ItemAttribute(e2,suite_itemdatum);
  if(a && b) { 
    status = strcmp(a->name,b->name);
    if(status < 0) return(-1);
    else if(status > 0) return(1);
  }
  return(0);
}

long int
SortBySuffix( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 )
      {
  register struct Dir_Entry	*a = NULL, *b = NULL;
  register long			 rc;

  if(!e1|| !e2) return(0);
  a = (struct Dir_Entry*)(suite)->ItemAttribute(e1,suite_itemdatum);
  b = (struct Dir_Entry*)(suite)->ItemAttribute(e2,suite_itemdatum);
  if(a && b) {
    if(!(rc = strcmp(FileSuffix(a->name),FileSuffix(b->name)))) {
      rc = strcmp(a->name,b->name);
      if(rc > 0) return(1);
      else if(rc < 0) return(-1);
    }
    else {
      if(rc > 0) return(1);
      else if(rc < 0) return(-1);
    }
  }
  return(0);
}

long int
SortBySize( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 )
      {
  register struct Dir_Entry	*a = NULL, *b = NULL;

  if(!e1 || !e2) return(0);
  a = (struct Dir_Entry*)(suite)->ItemAttribute(e1,suite_itemdatum);
  b = (struct Dir_Entry*)(suite)->ItemAttribute(e2,suite_itemdatum);
  if(a && b) {
    if(a->size < b->size) return(1);
    else if(a->size > b->size) return(-1);
  }
  return(0);
}

long int
SortByDate( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 )
      {
  register struct Dir_Entry	*a = NULL, *b = NULL;

  if(!e1 || !e2) return(0);
  a = (struct Dir_Entry*)(suite)->ItemAttribute(e1,suite_itemdatum);
  b = (struct Dir_Entry*)(suite)->ItemAttribute(e2,suite_itemdatum);
  if(a && b) {
    if(a->time_stamp < b->time_stamp) return(1);
    if(a->time_stamp > b->time_stamp) return(-1);
  }
  return(0);
}

long int
SortByType( register class bushv		 *self, register class suite		 *suite, register struct suite_item     *e1, register struct suite_item     *e2 )
      {
  register struct Dir_Entry	*a = NULL, *b = NULL;
  char				 n1[MAXPATHLEN+1], n2[MAXPATHLEN+1];
  register long			 rc;

  if(!e1 || !e2) return(0);
  a = (struct Dir_Entry*)(suite)->ItemAttribute(e1,suite_itemdatum);
  b = (struct Dir_Entry*)(suite)->ItemAttribute(e2,suite_itemdatum);
  if(a && b) {
    strcpy(n1,a->name);
    strcpy(n2,b->name);
    if(!(rc = strcmp(FileType(n1),FileType(n2)))) {
      rc = strcmp(a->name,b->name);
      if(rc < 0) return(-1);
      else if(rc > 0) return(1);
    }
    else {
      if(rc < 0) return(-1);
      else if(rc > 0) return(1);
    }
  }
  return(0);
}

static struct bind_Description bushvBindings[] = {

    {"bushv-exit", "\030\003", 0, "Quit~99", 0, bushv_DefaultMenus, (proctable_fptr) PerformExit, "Exit.  If any file is modified ask for confirmation."},

    {"bushv-save-file", "\030\023", 0, "Save~20", 0, bushv_RWEntryMenus, (proctable_fptr) bushv_SaveFile, "Saves buffer into its current file."},

    {"bushv-write-file", "\030\027", 0, "File~10,Save As~1", 0, bushv_EntryMenus, (proctable_fptr) bushv_WriteFile, "Prompt for a file to save."},

    {"bushv-set-printer", NULL, 0, "File~10,Set Printer~20", 0, bushv_DefaultMenus, (proctable_fptr) bushv_SetPrinter, "Set the default printer."},

    {"bushv-entries-page-up", "\033G", 0, "Entries~10,Page Up~20", 0, bushv_EntriesMenus, (proctable_fptr) EntriesPageUp, "Scroll entries list up."},

    {"bushv-entries-page-down", "\033E", 0, "Entries~10,Page Down~10", 0, bushv_EntriesMenus, (proctable_fptr) EntriesPageDown, "Scroll entries list down."},
  0
};

boolean
bushv::InitializeClass( )
  {
  struct ATKregistryEntry  *classInfo = NULL;
  struct proctable_Entry *pe;

  ::kmap = new keymap;
  ::bushv_menulist = new menulist;
  classInfo = ATK::LoadClass("bushv");
  bind::BindList(bushvBindings, ::kmap, ::bushv_menulist, classInfo);
  proctable::DefineProc("bushv-DEBUG", (proctable_fptr) ToggleDebug, 
    &bushv_ATKregistry_ , NULL, "Toggle Bush debug flag.");
  proctable::DefineProc("bushv-pop", (proctable_fptr) PerformPop, 
    &bushv_ATKregistry_ , NULL, "Pop up a level.");
  proctable::DefineProc("bushv-switch", (proctable_fptr) SwitchDirectory, 
    &bushv_ATKregistry_ , NULL, "Switch to a new directory.");
  proctable::DefineProc("bushv-rescan", (proctable_fptr) PerformRescan, 
    &bushv_ATKregistry_ , NULL, "Rescan the current directory.");
  proctable::DefineProc("bushv-destroy", (proctable_fptr) PerformDestroy, 
    &bushv_ATKregistry_ , NULL, "Destroy the current directory/files.");

  pe = proctable::Lookup("bushv-entries-page-up");
  (::kmap)->BindToKey( "\033v", pe, 0);

  pe = proctable::Lookup("bushv-entries-page-down");
  (::kmap)->BindToKey( "\026", pe, 0);
  return(TRUE);
}

static void
GetPreferredEditors( register class bushv  *self )
  {
  const char *ctmp;
  register char *tmp = NULL, *colon = NULL;
  char *myCopy = NULL;
  register int i = 0;

  IN(GetPreferredEditors);
  environ::SetProgramName("bush");
  if(((ctmp = environ::GetProfile("editors")) ||
      (ctmp = environ::GetProfile("editor"))) && *ctmp) {
    AllocNameSpace(ctmp,&myCopy);
    if((colon = strchr(tmp = myCopy,':'))) {
      while((colon = strchr(tmp,':')) && (i < MAXEDITORS)) {
        *colon = '\0';
	if(tmp && (*tmp != '\0')) {
	  EditorChoices[i] = NULL;
	  AllocNameSpace(tmp,&EditorChoices[i++]);
	}
	tmp = colon + 1;
      }
      if(tmp && (*tmp != '\0') && (i < MAXEDITORS)) {
        EditorChoices[i] = NULL;
	AllocNameSpace(tmp,&EditorChoices[i]);
	EditorChoices[i+1] = NULL;
      }
      else if(i >= MAXEDITORS)
	  fprintf(stderr,"bush: too many editors in preference.\n");
    }
    else if(tmp && (*tmp != '\0')) {
      EditorChoices[i] = NULL;
      AllocNameSpace(myCopy,&EditorChoices[i]);
      EditorChoices[i+1] = NULL;
    }
    i++;
    AllocNameSpace("other",&EditorChoices[i]);
    EditorChoices[i+1] = NULL;
    NumEditorChoices = i+1;
    if(myCopy) free(myCopy);
  }
  else if(!ctmp || (i == 0)) {
    for( i = 0 ; default_editor_choices[i] && (i < MAXEDITORS); i++ ) {
      EditorChoices[i] = NULL;
      AllocNameSpace(default_editor_choices[i],&EditorChoices[i]);
    }
    EditorChoices[i] = NULL;
    NumEditorChoices = i;
  }
  EditorIndex = 0;
  strcpy(EditorProgram,EditorChoices[0]);
  if(EditorProgram && *EditorProgram != '\0') {
    struct suite_item	*editorItem = NULL;
    char		 editorCaption[64];

    if((editorItem = (ControlView)->ItemOfDatum((long) &editor_data))) {
      sprintf(editorCaption,"Editor: %s",EditorProgram);
      (ControlView)->SetItemAttribute( editorItem, suite_ItemCaption(editorCaption));
    }
  }
  OUT(GetPreferredEditors);
}

static void
GetPreferredFonts( register class bushv	 *self )
  {
  const char *control_font;
  const char *tree_node_font;
  const char *listing_font;

  IN(GetPreferredFont);
  control_font = environ::GetProfile("controlfont");
  tree_node_font = environ::GetProfile("treenodefont");
  listing_font = environ::GetProfile("listingfont");
  if(control_font && *control_font) {
      (ControlView)->SetSuiteAttribute( suite_TitleFontName(control_font));
      (ControlView)->SetSuiteAttribute( suite_ItemCaptionFontName(control_font));
  }
  if(listing_font && *listing_font) {
      (EntriesView)->SetSuiteAttribute( suite_TitleFontName(listing_font));
      (EntriesView)->SetSuiteAttribute( suite_ItemCaptionFontName(listing_font));
  }
  if(tree_node_font && *tree_node_font) {
      (DirTreeView)->SetTreeAttribute( treev_NodeFontName(tree_node_font));
  }
  OUT(GetPreferredFont);
}

static void
GetPreferences( register class bushv	 *self )
  {
  IN(GetPreferences);
  GetPreferredEditors(self);
  GetPreferredFonts(self);
  CkpInterval = environ::GetProfileInt("CheckpointInterval",DEFAULTCKPINTERVAL);
  OUT(GetPreferences);
}

bushv::bushv( )
    {
	ATKinit;

  class bushv *self=this;
  Kmap = ::kmap;
  if(!(Kstate = keystate::Create(this,Kmap))) {
    fprintf(stderr,"bushv: could not create keystate.\n");
    exit(1);
  }
  Cursor = cursor::Create(this);
  (Cursor)->SetStandard(Cursor_Wait);
  BUSH = NULL;
  CurrNode = InitNode = MoveNode = NULL;
  CurrEntry = NULL;
  EntryView = EntryViewAppLayer = NULL;
  EntryObject = NULL;
  EntryObjectModified = EntryObjectLastCKP = 0;
  EntryFilep = NULL;
  ControlView = suite::Create(control_spec, (long) this);
  EntriesView = suite::Create(entries_spec, (long) this);
  DirTreeView = treev::Create(tree_spec, (class view *) this);
  LP = new lpair;
  (LP)->VTFixed((class view*)ControlView,(class view*)DirTreeView,95,TRUE);
  (LP)->LinkTree((class view*)this);
  Object = nodes_object;
  SortMode = 0;
  Detail = 0;
  NumPrevSelected = 0;
  Debug = 1;
  Menulist = (::bushv_menulist)->DuplicateML(this);
  GetPreferences(this);
  (this)->SetOptions(aptv_SuppressControl | aptv_SuppressBorder);
  TopLevelInset = TRUE;
  OUT(bushv_InitializeObject);
  THROWONFAILURE((TRUE));
}   

bushv::~bushv( )
    {
	ATKinit;
   class bushv *self=this;
  IN(bushv_FinalizeObject);
  if(LP) (LP)->Destroy();
  if(ControlView) (ControlView)->Destroy();
  if(EntriesView) (EntriesView)->Destroy();
  if(DirTreeView) (DirTreeView)->Destroy();
  if(EntryView) (EntryView)->Destroy();
  if(EntryObject) (EntryObject)->Destroy();
  if(Menulist) delete Menulist;
  OUT(bushv_FinalizeObject);
}

class bushv *
bushv::Create( register char  object )
    {
	ATKinit;

  class bushv *self = NULL;

  if((self = new bushv))
      Object = object;
  OUT(bushv_Create);
  return(self);
}

void
bushv::PostMenus( class menulist	 *menulist )
    {
  int mask = 0;
  class bushv *self=this;
  IN(bushv_PostMenus);
  (Menulist)->ClearChain();
  mask = (Menulist)->GetMask() | bushv_DefaultMenus;
  switch(Object) {
      case nodes_object:
	  mask &= (~bushv_EntriesMenus & ~bushv_EntryMenus & ~bushv_RWEntryMenus);
	  break;
      case entry_object:
	  mask |= bushv_EntryMenus;
	  mask &= ~bushv_EntriesMenus;
	  break;
      case entries_object:
	  mask |= bushv_EntriesMenus;
	  mask &= (~bushv_EntryMenus & ~bushv_RWEntryMenus);
	  break;
  }
  (Menulist)->SetMask(mask);
  if(menulist) 
    (Menulist)->ChainAfterML(menulist, 0);
  (this)->aptv::PostMenus(Menulist);
  OUT(bushv_PostMenus);
}

class view *
bushv::Hit( enum view_MouseAction  action, long  x , long  y , long  numberOfClicks )
      {
  class bushv *self=this;
  IN(bushv_Hit);
  return((LP)->Hit(action,x,y,numberOfClicks));
}

void
bushv::FullUpdate( enum view_UpdateType  Type, long  left , long  top , long  width , long  height )
      {
  struct rectangle r;
  char RootPathIfInset[MAXPATHLEN];
  char NewTitle[MAXPATHLEN];
  class bushv *self=this;

  IN(bushv_FullUpdate);
  if(Type == view_LastPartialRedraw || Type == view_FullRedraw) {
    (this)->aptv::FullUpdate(Type,left,top,width,height);
    if(!RootPathName) {
	im::GetDirectory(RootPathIfInset);
	(TREE)->DestroyNode(TreeRoot);
      (BUSH)->InitTree(RootPathIfInset);
      CurrNode = InitNode = TreeRoot;
      (BUSH)->BuildSubDirs(TreeRoot);
      HandleChangeDir(this,RootPathName);
    }
    (this)->GetVisualBounds(&r);
    (LP)->InsertView((class view*)this,&r);
    (this)->SetTransferMode(graphic_BLACK);
    (LP)->FullUpdate(Type,0,0,r.width,r.height);
    if(CurrNode || (CurrNode = (DirTreeView)->CurrentNode())) {
      sprintf(NewTitle,"%s%s%s",DirPath(CurrNode),
	       CurrEntry ? "/" : "", CurrEntry ? CurrEntry->name : "");
      (ControlView)->ChangeSuiteAttribute(
				  suite_TitleCaption(NewTitle));
    }
    else 
	(ControlView)->ChangeSuiteAttribute(suite_TitleCaption("No Current Directory"));
    PassivateControls(this);
    ClearMessageLine();
  }
 // Must NOT request the input focus in a screen update routine! if(Object == nodes_object)    (this)->WantInputFocus(DirTreeView);
  OUT(bushv_FullUpdate);
}

static void
DoEdit( class bushv  *self, char  *path , char  *name  )
    {
  char full_path[MAXPATHLEN * 2];

  IN(DoEdit);
  sprintf(full_path,"%s/%s",path,name);
  sprintf(msg,"%s '%s/%s'",EditorProgram,path,name);
  argv[0] = EditorProgram;
  argv[1] = full_path;
  argv[2] = NULL;
  PerformSystemAction(self,EditorProgram,argv,msg);
  OUT(DoEdit);
}

static void
PerformEdit( class bushv  *self )
  {
  register int i = 0;

  IN(PerformEdit);
  PostCursor(self,Cursor_Wait);
  switch(Object) {
    case nodes_object: 
      break;
    case entries_object:
      if(CurrNode && DirEntries(CurrNode) &&
	  (DirEntriesCount(CurrNode) >= 0))
        for(i=0;i<DirEntriesCount(CurrNode);i++)
	  if(DirEntryMode(CurrNode,i).selected &&
	     !DirEntryType(CurrNode,i).dir)
	    DoEdit(self,DirPath(CurrNode), 
		   DirEntryName(CurrNode,i));
      break;
    case entry_object:
      DoEdit(self,DirPath(CurrNode),CurrEntry->name);
      break;
  }
  (self)->RetractCursor(Cursor);
  OUT(PerformEdit);
}

static void
PerformPrint( class bushv	 *self )
  {
  register int	 i = 0;
  register FILE	*file = NULL;

  IN(PerformPrint);
  PostCursor(self,Cursor_Wait);
  switch(Object) {
    case nodes_object:
#ifdef RCH_ENV
	// XXX Fix this!
	Announce("You must visit a file to print.");
#else
      if((file = fopen("/tmp/bush_print.PS","w"))) {
        (DirTreeView)->Print(file,"PostScript","PostScript",1);
	fclose(file);
	system("lpr /tmp/bush_print.PS");
	sprintf(msg,"Printed '%s'",RootPathName);
	Announce(msg);
      }
      else Announce("Error Printing");
#endif
      break;
    case entries_object:
#ifdef RCH_ENV
	// XXX Fix this!
	Announce("You must visit a file to print.");
#else
      for(i=0;i < DirEntriesCount(CurrNode);i++)
        if(DirEntryMode(CurrNode,i).selected && 
	    !DirEntryType(CurrNode,i).dir)
	  DoPrint(self,DirPath(CurrNode),
		  DirEntryName(CurrNode,i));
#endif
      break;
    case entry_object:
#ifdef RCH_ENV
      char *fname = (char *)ControlView->SuiteAttribute(suite_titlecaption);
      if (fname == NULL) fname = "(unknown)";	// can this happen?
      char *p = strrchr(fname, '/');
      if (p)
	  fname = p+1;
      ATK::LoadClass("rchprint");
      struct proctable_Entry *pe = proctable::Lookup("rchprint-doprint");
      if (pe) {
	  void (*doprint)(view *, view *, char *, int) = (void (*)(view *, view *, char *, int))(pe->proc);
	  doprint(self, self->entry_view, NewString(fname), 1);
      } else
	  Announce("Cannot load rchprint dialog.");
#else
      DoPrint(self,DirPath(CurrNode),CurrEntry->name);
#endif
      break;
  }
  (self)->RetractCursor(Cursor);
  OUT(PerformPrint);
}

static
void DoPrint( class bushv  *self, char  *path , char  *name  )
    {
  char full_path[MAXPATHLEN];
  int i = 0;

  IN(DoPrint);
  sprintf(full_path, "%s/%s", path, name);
  sprintf(msg, "Printing '%s'", full_path);
  argv[i++] = "ezprint";
  argv[i++] = full_path;
  argv[i] = NULL;
  PerformSystemAction(self, argv[0], argv, msg);
  OUT(DoPrint);
}

static void
PerformCreate( class bushv  *self )
  {
  register int f = 0;
  char *response = NULL;

  IN(PerformCreate);
  if(!CurrNode) return;
  switch(Object) {
    case nodes_object:
      if((self)->Query("Directory Name: ","",&response)) return;
      sprintf(cmd,"%s/%s",DirPath(CurrNode),response);
      if(mkdir(cmd,0777)) {
        IssueError(self,"Creating",response,TRUE);
	return;
      }
      else PerformRescan(self);
      break;
    case entries_object:
      if((self)->Query("File Name: ","",&response)) return;
      sprintf(cmd,"%s/%s",DirPath(CurrNode),response);
      if((f = open(cmd,O_CREAT | O_EXCL, 0100 | 0200 | 0400)) < 0) {
        IssueError(self,"Creating",response,TRUE);
	return;
      }
      else {
        close(f);
	PerformRescan(self);
      }
      break;
    case entry_object:
      break;
  }
  OUT(PerformCreate);
}

static void
IssueError( register class bushv  *self, register const char  *what , register const char  *where, boolean  overlay )
      {
  long result = 0;
  static const char * const question[] = { "Continue", NULL };

  IN(IssueError);
  if(errno > 0 && errno <= sys_nerr) 
    sprintf(msg,"ERROR %s '%s': %s", what, where, strerror(errno) );
  else if(errno != 0)
    sprintf(msg,"ERROR %s '%s': (Invalid System Error-code '%d')", what, where, errno );
  else
    sprintf(msg,"ERROR %s '%s'", what, where);
  if(overlay) message::MultipleChoiceQuestion(self,100,msg,0,&result,question,NULL);
  else Announce(msg);
  errno = 0;
  OUT(IssueError);
}

static int
DoDestroy( register class bushv  *self, register tree_type_node  tn, register struct Dir_Entry  *Entry, int  overlay )
        {
  register int status = 0;

  IN(DoDestroy);
  if(self && tn && Entry) {
    if(status = (BUSH)->DestroyEntry(tn,Entry)) 
      IssueError(self,"Destroying",Entry->name,overlay);
  }
  OUT(DoDestroy);
  return(status);
}

static void
PerformDestroy( class bushv  *self )
{
    static const char * const question[] = {"Confirm", "Cancel", 0};
    int i = 0;
    long result = 0;
    long count = 0;
    struct Dir_Entry *d_entry = NULL, *current_entry = CurrEntry;
    tree_type_node current_node = CurrNode, tn = NULL;
    struct suite_item **selected = NULL;

    IN(PerformDestroy);
    if(!current_node) return;
    switch(self->object) {
	case nodes_object:
	    if(tn = Parent(current_node)) {
		for(i=0;i<DirEntriesCount(tn);i++)
		    if(DirEntryType(tn,i).dir  &&
		       strcmp(DirName(current_node),DirEntryName(tn,i)) == 0) {
			d_entry = DirEntry(tn,i);
			break;
		    }
		if(d_entry) {
		    sprintf(msg,"Confirm destroying '%s'",
			    DirName(current_node));
		    if(message::MultipleChoiceQuestion(self,50,msg,1,
						       &result,question,NULL) == -1) 
			return;
		    else if((result == 0) && DirEntries(current_node) && 
			    (DirEntriesCount(current_node) > 0)){
			sprintf(msg,"'%s' has contents. Destroy Anyway?",
				DirName(current_node));
			if(message::MultipleChoiceQuestion(self,100,msg,
							   1,&result,question,NULL) == -1) return;
			if(result != 0) {
			    Announce("Cancelled");
			    return;
			}
		    }
		    if(result == 0) {
			PostCursor(self,Cursor_Wait);
			if(DoDestroy(self,Parent(current_node),
				     d_entry,TRUE)) {
			    (self)->RetractCursor(Cursor);
			    return;
			}
			CurrNode = Parent(current_node);
			(DirTreeView)->HighlightNode(CurrNode);
			(ControlView)->ChangeSuiteAttribute(
							    suite_TitleCaption(DirPath(CurrNode)));
			(self)->RetractCursor(Cursor);
			PerformRescan(self);
			sprintf(msg,"Destroyed Node '%s'",d_entry->name);
			Announce(msg);
		    }
		}
		else Announce("Node Not Found in Parent");
	    }
	    else Announce("May Not Destroy Root Directory");
	    break;
	case entries_object:
	    selected = (EntriesView)->SelectedItems(&count);
	    if(count == 0) break;
	    if(count > 1)
		if(count == 2) sprintf(msg, "Destroy Both Items ?");
		else sprintf(msg, "Destroy All %ld Items ?", count);
	    else {
		d_entry = (struct Dir_Entry*)
		  (EntriesView)->ItemAttribute(selected[0], suite_itemdatum);
		sprintf(msg, "Destroy %s '%s' ?", EntryDirType(d_entry)
			? "Node" : "", d_entry->name);
	    }
	    if(message::MultipleChoiceQuestion(self, 100, msg, 1, &result, question, NULL) == -1) return;
	    if(result == 0) {
		PostCursor(self,Cursor_Wait);
		for( i = 0 ; i < count ; i++ ) {
		    d_entry = (struct Dir_Entry*) 
		      (EntriesView)->ItemAttribute(selected[i],suite_itemdatum);
		    if(!DoDestroy(self,current_node,d_entry,TRUE)) {
			(EntriesView)->PassivateItem(selected[i]);
			EntrySelected(d_entry) = FALSE;
		    }
		}
		(self)->RetractCursor(Cursor);
	    }
	    else Announce("Canceled");
	    break;
	case entry_object:
	    if(current_entry) {
		if(EntryDirType(current_entry))
		    sprintf(msg, "Destroy Directory '%s'?", current_entry->name);
		else
		    sprintf(msg, "Destroy '%s'?", current_entry->name);
		if(message::MultipleChoiceQuestion(self, 100, msg, 1, &result, question, NULL) == -1) return;
		if(result==0) {
		    PostCursor(self, Cursor_Wait);
		    if(!DoDestroy(self, current_node, current_entry, TRUE)) {
			EntrySelected(current_entry) = FALSE;
			CurrEntry = NULL;
			PerformPop(self);
			(ControlView)->ChangeSuiteAttribute( suite_TitleCaption( DirPath(CurrNode) ) );
			(EntriesView)->PassivateItem( (EntriesView)->CurrentItem() );
		    }
		    (self)->RetractCursor(Cursor);
		}
	    }
	    else Announce("Canceled");
	    break;
    }
    OUT(PerformDestroy);
}

static void
PerformExec( class bushv  *self )
  {
  register int i = 0;

  IN(PerformExec);

  PostCursor(self,Cursor_Wait);
  switch(Object) {
    case nodes_object: break;
    case entries_object:
      if(CurrNode && DirEntries(CurrNode) && 
	  (DirEntriesCount(CurrNode) >= 0))
        for(i = 0;i < DirEntriesCount(CurrNode);i++)
	  if(DirEntryMode(CurrNode,i).selected &&
	     !DirEntryType(CurrNode,i).dir)
	      DoExecute(self,CurrNode,DirEntry(CurrNode,i));
      break;
    case entry_object:
      DoExecute(self,CurrNode,CurrEntry);
      break;
  }
  (self)->RetractCursor(Cursor);
  OUT(PerformExec);
}

static
void DoExecute( class bushv		 *self, tree_type_node	  tn , struct Dir_Entry	 *Entry  )
      {
  char			 full_path[MAXPATHLEN * 2];

  IN(DoExecute);
  sprintf(full_path,"%s/%s",DirPath(tn),Entry->name);
  sprintf(msg,"Executing '%s'",full_path);
  argv[0] = full_path;
  argv[1] = NULL;
  PerformSystemAction(self,full_path,argv,msg);
  OUT(DoExecute);
}

static int
PerformSystemAction( class bushv	 *self, const char		 *name, const char		 *argv[], char		 *msg )
        {
  Announce(msg);
  return((BUSH)->PerformSystemAction(name,argv));
}

static char*
Format_Tags( u_short	     tag )
  {
  static char	    tags[11];

  strcpy(tags,"----------");
  if((tag & S_IFMT) == S_IFREG) tags[0] = '-';
  else if((tag & S_IFMT) == S_IFDIR) tags[0] = 'd';
  else if((tag & S_IFMT) == S_IFCHR) tags[0] = 'c';
#ifdef S_IFBLK
  else if((tag & S_IFMT) == S_IFBLK) tags[0] = 'b';
#endif
#ifdef S_IFLNK
  else if((tag & S_IFMT) == S_IFLNK) tags[0] = 'l';
#endif
#ifdef S_IFIFO
  else if((tag & S_IFMT) == S_IFIFO) tags[0] = '?';
#endif /* #ifdef S_IFIFO */
#ifdef S_IFSOCK
  else if((tag & S_IFMT) == S_IFSOCK) tags[0] = '?';
#endif /* #ifdef S_IFSOCK */
  if(tag & S_IREAD) tags[1] = 'r';
  if(tag & S_IWRITE) tags[2] = 'w';
  if(tag & S_IEXEC) tags[3] = 'x';
  if(tag & (S_IREAD >>3)) tags[4] = 'r';
  if(tag & (S_IWRITE>>3)) tags[5] = 'w';
  if(tag & (S_IEXEC >>3)) tags[6] = 'x';
  if(tag & (S_IREAD >>6)) tags[7] = 'r';
  if(tag & (S_IWRITE>>6)) tags[8] = 'w';
  if(tag & (S_IEXEC >>6)) tags[9] = 'x';
  tags[10] = '\0';
  return(tags);
}

static char*
FormatEntriesItem( register class bushv *self, tree_type_node tn, int i, struct Dir_Entry *dirEntry )
{
  static char entries_item[257];
  char *entries_ptr = NULL, *time_ptr = NULL, trailer[5];

  IN(FormatEntriesItem);
  if(tn) {
      if(!DirEntryType(tn,i).soft_link && DirEntryType(tn,i).dir) 
	  strcpy(trailer,"/");
      else if(DirEntryType(tn,i).soft_link && Detail) 
	  strcpy(trailer," -> ");
      else if(DirEntryPerms(tn,i) & S_IEXEC)
	  strcpy(trailer,"*");
      else
	  trailer[0] = '\0';
      if(Detail) {
	  time_ptr = (char*) ctime((const time_t *) &DirEntryTimeStamp(tn,i));
      time_ptr[24] = '\0';
      sprintf(entries_item,"%s %2d %8s %8ld %24s %s%s%s",
	      Format_Tags(DirEntryPerms(tn,i)),
	      DirEntryNLinks(tn,i), DirEntryOwner(tn,i),
	      DirEntrySize(tn,i), time_ptr, DirEntryName(tn,i),
	      trailer, DirEntryType(tn,i).soft_link ? DirEntryLinkName(tn,i) : "");
      }
      else sprintf(entries_item, "%s%s", DirEntryName(tn,i), trailer);
      entries_ptr = entries_item;
  }
  else if(dirEntry) {
      if(!EntryLinkType(dirEntry) && EntryDirType(dirEntry))
	  strcpy(trailer,"/");
      else if(EntryLinkType(dirEntry) && Detail) 
	  strcpy(trailer," -> ");
      else if(EntryPerms(dirEntry) & S_IEXEC)
	  strcpy(trailer,"*");
      else
	  trailer[0] = '\0';
      if(Detail) {
	  time_ptr = (char*) ctime(&dirEntry->time_stamp);
	  time_ptr[24] = '\0';
	  sprintf(entries_item,"%s %2d %8s %8ld %24s %s%s%s",
		  Format_Tags(dirEntry->permissions),
		  dirEntry->nlinks,dirEntry->owner,
		  dirEntry->size,time_ptr,dirEntry->name,
		  trailer, EntryLinkType(dirEntry) ? EntryLinkName(dirEntry) : "");
      }
      else sprintf(entries_item,"%s%s",dirEntry->name,trailer);
      entries_ptr = entries_item;
  }
  OUT(FormatEntriesItem);
  return(entries_ptr);
}

static void
ResetEntriesCaptions( class bushv  *self )
{
  register int i = 0, count = 0;
  struct suite_item *item = NULL;

  IN(ResetEntriesCaptions);
  (EntriesView)->Reset(suite_ClearItems);
  if(DirEntries(CurrNode) && 
      (count = DirEntriesCount(CurrNode)) >= 0)
      for(i = 0 ; i < count ; i++) {
	  if(DirEntryMode(CurrNode, i).destroyed == FALSE &&
	     (item = (EntriesView)->CreateItem((char*) DirEntryName(CurrNode,i), (long int) DirEntry(CurrNode,i))))
	      (EntriesView)->SetItemAttribute(item, suite_ItemCaption(FormatEntriesItem(self, CurrNode, i, NULL)));
      }
  OUT(ResetEntriesCaptions);
}

static int
HandleModifiedObject( register class bushv	     *self )
  {
  int			     return_value = 0;
  long			     result = 0;
  static const char  * const answers[] = { "Save to file.",
                                           "Save As...",
					   "Don't save",
                                           "Cancel",
                                            NULL };

  IN(HandleModifiedObject);
  sprintf(msg,"File has been modified.  Save?");
  if(message::MultipleChoiceQuestion(self,100,msg,0,&result,answers,NULL) == -1)
    return_value = -1;
  else 
    switch(result) { 
      case(0):
	return_value = bushv_SaveFile(self);	    
	break;
      case(1):
	return_value = bushv_WriteFile(self);  
	break;
      case(2):
        EntryObjectModified = EntryObjectLastCKP = 0;
	break;
      case(3):
      default:		    
	return_value = -1;	    
	break;
    }
  OUT(HandleModifiedObject)
  return(return_value);
}

static void
PerformExit( class bushv	 *self )
  {
  IN(PerformExit);
  if(Object == entry_object)
    if((EntryObject)->GetModified() > EntryObjectModified)
      if(HandleModifiedObject(self) < 0)
        return;
  exit(0);
  OUT(PerformExit);
}

static void
SwitchDirectory( class bushv	     *self )
  {
  static const char * const question[] = {"Continue",NULL};
  int		     msg_status = 0;
  long		     result = 0;
  char		    *response = NULL;
  struct stat	     stats;

  IN(SwitchDirectory);
  msg_status = (self)->QueryDirectoryName("Switch To Directory: ",&response);
  if(msg_status) {
    return;
  }
  PostCursor(self,Cursor_Wait);
  if(stat(response,&stats)) {
    IssueError(self,"ReScanning",response,TRUE);
    (self)->RetractCursor(Cursor);
    return;
  }
  if((stats.st_mode & S_IFMT) != S_IFDIR) {
    sprintf(msg,"Must Change to a Directory");
    message::MultipleChoiceQuestion(self,100,msg,0,&result,question,NULL);
    sprintf(msg,"Failed to change to '%s'",response);
    Announce(msg);
    (self)->RetractCursor(Cursor);
    return;
  }
  if(Object != nodes_object) {
    PopToNodes(self);
    PostCursor(self,Cursor_Wait);
  }
  SetTreeNotificationData(self,TreeRoot,tree_NodeDestroyed);
  NotifyTreeObservers(self);
  (TREE)->DestroyNode(TreeRoot);
  (BUSH)->InitTree(response);
  CurrNode = InitNode = TreeRoot;
  (BUSH)->BuildSubDirs(CurrNode);
  SetTreeNotificationData(self,CurrNode,tree_NodeCreated);
  NotifyTreeObservers(self);
  (ControlView)->ChangeSuiteAttribute(
	suite_TitleCaption(DirPath(CurrNode)));
  if(TopLevelInset) 
    ((self)->GetIM())->SetTitle(DirName(CurrNode));
  (self)->RetractCursor(Cursor);
  HandleChangeDir(self,RootPathName);
  OUT(SwitchDirectory);
}

static void
SetEditor( class bushv	 *self )
  {
  long		 result = 0;
  char		*response = NULL;

  IN(SetEditor);
  sprintf(msg,"Set editor to: ");
  if(message::MultipleChoiceQuestion(self,100,msg,EditorIndex,
	&result,EditorChoices,NULL) == -1) return;
  if(!strcmp(EditorChoices[result],"other")) {
    if((self)->Query("Set editor to: ","",&response)) return;
    AllocNameSpace(EditorChoices[NumEditorChoices-1],
	       &EditorChoices[NumEditorChoices]);
    AllocNameSpace(response,&EditorChoices[NumEditorChoices-1]);
    EditorChoices[++NumEditorChoices] = NULL;
  }
  EditorIndex = result;
  strcpy(EditorProgram,EditorChoices[result]);
  if(EditorProgram && *EditorProgram != '\0') {
    struct suite_item	*editorItem = NULL;
    char		 editorCaption[64];

    if(editorItem = (ControlView)->ItemOfDatum((long int) &editor_data)) {
      sprintf(editorCaption,"Editor: %s",EditorProgram);
      (ControlView)->ChangeItemAttribute( editorItem, suite_ItemCaption(editorCaption));
    }
  }
  sprintf(msg,"Editor set to: '%s'",EditorProgram);
  Announce(msg);
  OUT(SetEditor);
}

void
bushv::PostKeyState( class keystate *kstate )
{
  class bushv *self = this;
  class keystate *finalKstate = Kstate;

  IN(bushv_KeyState);
  Kstate->next= NULL;
  if (kstate)
      finalKstate= Kstate->AddAfter(kstate);
  (this)->aptv::PostKeyState(finalKstate);
  OUT(bushv_KeyState);
}

char *
FormatEntriesInfo( class bushv  *self, tree_type_node  tn )
{
  static char entries_info[257];
  register long i = 0, total_bytes = 0, count = 0;

  IN(FormatEntriesInfo);
  if(tn) {
    if(DirEntries(tn)) count = DirEntriesCount(tn);
      for( i = 0; i < count; i++ ) total_bytes += DirEntrySize(tn,i);
        sprintf(entries_info,"%ld %s    %ld %s    %s %s",count,
		 "Entries",total_bytes,"Bytes","Sorted by", sorts[SortMode]);
  }
  OUT(FormatEntriesInfo);
  return(entries_info);
}

static suite_sortfptr
DetermineSortHandler( class bushv *self, tree_type_node tn )
{
  suite_sortfptr		       sorter;

  IN(DetermineSortHandler);
  switch(SortMode) {
    case by_name:    sorter =  (suite_sortfptr) SortByName;    break;
    case by_size:    sorter =  (suite_sortfptr) SortBySize;    break;
    case by_date:    sorter =  (suite_sortfptr) SortByDate;    break;
    case by_suffix:  sorter =  (suite_sortfptr) SortBySuffix;  break;
    case by_type:    sorter =  (suite_sortfptr) SortByType;    break;
  }
  OUT(DetermineSortHandler);
  return(sorter);
}

static void
DoAutoRescan( register class bushv  *self )
  {
  register struct suite_item	**selected = NULL, *item = NULL;
  register char			**names = NULL;
  int				  count = 0;
  register int			  i;

  IN(DoAutoRescan);
  if(environ::GetProfileSwitch("AutoRescan", FALSE)) {
      if(ScanRequired(CurrNode)) {
	  if(Object == entries_object) {
	      selected = (EntriesView)->SelectedItems((long int *) &count);
	      names = (char**)calloc(count, sizeof(char*));
	      for(i = 0; i < count; i++)
		  AllocNameSpace( (char *) (EntriesView)->ItemAttribute( selected[i], suite_itemcaption), &names[i] );
	  }
	  PerformRescan(self);
	  if(Object == entries_object)
	      for(i = 0; i < count; i++) {
		  if(item = (EntriesView)->ItemOfName( names[i]))
		      (EntriesView)->HighlightItem( item);
		  free(names[i]);
		  names[i] = NULL;
	      }
	  if(names) {
	      free(names);
	      names = NULL;
	  }
	  if(selected) {
	      free(selected);
	      selected = NULL;
	  }
      }
  }
  IN(DoAutoRescan);
}

static void
PushToEntries( class bushv	       *self )
  {
  suite_sortfptr       sorter;

  IN(PushToEntries);
  if(EntriesView) {
     sorter = DetermineSortHandler(self,CurrNode);
    (EntriesView)->SetSuiteAttribute(suite_SortHandler(sorter));
    Object = entries_object;
    (LP)->SetNth(1,(class view*)EntriesView);
    ResetEntriesCaptions(self);
    (EntriesView)->SetSuiteAttribute(
	    suite_TitleCaption(FormatEntriesInfo(self,CurrNode)));
    DoAutoRescan(self);
    (self)->WantInputFocus(EntriesView);
  }
  OUT(PushToEntries);
}

static void
PushToEntry( class bushv  *self )
  {
  char file_name[MAXPATHLEN];
  const char *objectName = NULL;
  long objectID = 0;
  struct attributes *attrs;

  IN(PushToEntry);
  PostCursor(self, Cursor_Wait);
  Object = entry_object;
  sprintf(file_name, "%s/%s", DirPath(CurrNode), CurrEntry->name);
  sprintf(msg, "reading '%s'", file_name);
  Announce(msg);
  if(EntryFilep = fopen(file_name, "r")) {
    if(!(objectName = filetype::Lookup(EntryFilep, file_name,
				       &objectID, &attrs)))
      objectName = "text";
    if(EntryView) {
      if((EntryViewAppLayer)->IsAncestor( self))
        (LP)->SetNth( 1, NULL);
      (EntryView)->DeleteApplicationLayer( EntryViewAppLayer);
      (EntryView)->Destroy();
      EntryView = EntryViewAppLayer = NULL;
      (EntryObject)->Destroy();
      EntryObject = NULL;
    }
    if(!(EntryObject = (class dataobject *) ATK::NewObject(objectName)) || 
       !(EntryView = (class view *)
	  ATK::NewObject((EntryObject)->ViewName()))) {
      IssueError(self, "Allocating Object", objectName, TRUE);
      Object = entries_object;
    }
    else { /*success*/
      struct attributes readWriteAttr;

      (EntryObject)->Read( EntryFilep, objectID);
      (EntryView)->SetDataObject( EntryObject);
      readWriteAttr.next = attrs;
      readWriteAttr.key = "readonly";
      if((access(file_name, W_OK) == -1) && (errno == EACCES)) {
	readWriteAttr.value.integer = TRUE; /* Read Only */
	(Menulist)->SetMask(0);
      }
     else {
	readWriteAttr.value.integer = FALSE; /* Read Write */
	(Menulist)->SetMask( bushv_RWEntryMenus);
	if(CkpInterval != 0)
	  im::EnqueueEvent((event_fptr) Checkpoint, (char *) self, event_SECtoTU(CkpInterval));
      }
      (EntryObject)->SetAttributes( &readWriteAttr);
      (LP)->SetNth( 1,
	EntryViewAppLayer = (EntryView)->GetApplicationLayer());
      (self)->WantInputFocus( EntryView);
      (ControlView)->ChangeSuiteAttribute( suite_TitleCaption(file_name));
      EntryObjectModified = EntryObjectLastCKP = (EntryObject)->GetModified();
    }
  }
  else {
    IssueError(self, "Opening Entry", CurrEntry->name,TRUE);
    Object = entries_object;
  }
  ClearMessageLine();
  (self)->RetractCursor( Cursor);
  OUT(PushToEntry);
}

static int
PopToNodes( class bushv	 *self )
  {
  int		 status = 0;

  IN(PopToNodes);
  if(DirTreeView) {
    Object = nodes_object;
    CurrEntry = NULL;
    (ControlView)->ChangeSuiteAttribute(
	suite_TitleCaption(DirPath(CurrNode)));
    (LP)->SetNth(1,(class view*)DirTreeView);
    DoAutoRescan(self);
    NumPrevSelected = 0;
    (self)->WantInputFocus(DirTreeView); 
  }
  OUT(PopToNodes);
  return(status);
}

static int
PopToEntries( class bushv *self )
{
    int status = 0;

    IN(PopToEntries);
    if(Object == entry_object)
	if((EntryObject)->GetModified() > EntryObjectModified)
	    if((status = HandleModifiedObject(self)) < 0)
		return(status);
    if(EntryFilep) {
	fclose(EntryFilep);
	EntryFilep = NULL;
    }
    if(EntriesView) {
	Object = entries_object;
	(LP)->SetNth(1, (class view *)EntriesView);
	(ControlView)->ChangeSuiteAttribute( suite_TitleCaption(DirPath(CurrNode)) );
	DoAutoRescan(self);
	(self)->WantInputFocus(EntriesView);
    }
    CurrEntry = NULL;
    OUT(PopToEntries);
    return(status);
}

static long
Passivator( class bushv	     *self, class suite	     *suite, struct suite_item  *item, unsigned	      datum )
/*============================================================
I set the item_data attribute, active, of the ControlView items to be the OR'ed sum of the Object-codes for which the particular item is active. If a button is active during both the nodes_object(1) and entries_object(2) then its data field is set to (nodes_object + entries_Object) = 1 + 2 = 3. This is how I implemented automatic button "Passivation" in bush.  It has been proposed that this feature be incorporated into suite (3/29/89).
=============================================================*/
        {
  struct item_data  *itemData = NULL; 
  long int	     result = 0;

  if(suite && item) {
    itemData = (struct item_data*) (suite)->ItemAttribute( item, suite_itemdatum);
    if(itemData && (itemData->activate & Object))
      (suite)->ActivateItem(item);
    else (suite)->PassivateItem(item);
  }
  return(result);
}

static void
PassivateControls( class bushv     *self )
  {
  IN(PassivateControls);
  (ControlView)->Apply((suite_applyfptr) Passivator, (long int) self, 0);
  (ControlView)->Reset(suite_Normalize);
  OUT(PassivateControls);
}

static void
Push( register class bushv	     *self )
  {
  register tree_type_node    tn = NULL;
  char			    *name = NULL;

  IN(Push);
  switch(Object) {
    case nodes_object:
      PushToEntries(self);
      break;
    case entries_object:
      if(EntryDirType(CurrEntry)) {
	AllocNameSpace(CurrEntry->name,&name);
	PostCursor(self,Cursor_Wait);
	Pop(self);
	tn = Child(CurrNode);
	while(tn) 
	  if(!strcmp(DirName(tn),name)) 
	    break;
	  else 
            tn = Right(tn);
	if(tn) {
	  TreeHitHandler(self,DirTreeView,tn,
		      treev_NodeObject,view_LeftUp,0,0,0);
	  (DirTreeView)->HighlightNode(tn);
	}
	else {
	  sprintf(msg,"Couldn't locate directory node '%s'.",CurrEntry->name);
	  Announce(msg);
	}
	(self)->RetractCursor(Cursor);
      }
      else PushToEntry(self);
      break;
  }
  if(name) free(name);
  PassivateControls(self);
  ClearMessageLine();
  OUT(Push);
}

static void
Pop( class bushv     *self )
  {
  int		   status = 0;

  IN(Pop);
  switch(Object) {
    case entries_object:
      status = PopToNodes(self);    break;
    case entry_object:
      status = PopToEntries(self);  break;
  }
  if(status == 0) {
    PassivateControls(self);
    ClearMessageLine();
  }
  OUT(Pop);
}

static void
PerformPop( class bushv	 *self )
  {
  IN(PerformPop);
  PostCursor(self,Cursor_Wait);
  Pop(self);
  (self)->RetractCursor(Cursor);
  OUT(PerformPop);
}

static long
ToggleCaptionDetail( class bushv *self, class suite *suite, struct suite_item *item, unsigned datum )
{
  long int result = 0;

  IN(ToggleCaptionDetail);
  if(suite && item )
    (suite)->SetItemAttribute(item,
	suite_ItemCaption(FormatEntriesItem(self,NULL,0,(struct Dir_Entry*)
		(suite)->ItemAttribute(item,suite_itemdatum))));
  OUT(ToggleCaptionDetail);
  return(result);
}

static void
SortDir( class bushv *self, tree_type_node tn )
{
  suite_sortfptr sorter;

    IN(SortDir);
    sorter = DetermineSortHandler(self, tn);
    if(Object == entries_object)
	(EntriesView)->Sort( 0, (suite_sortfptr) sorter);
    (EntriesView)->ChangeSuiteAttribute( suite_TitleCaption(FormatEntriesInfo(self, CurrNode)));
    OUT(SortDir);
}


static void
PerformDetail( class bushv  *self )
  {
    IN(PerformDetail);
    PostCursor(self, Cursor_Wait);
    Detail = !Detail;
    if(Object == nodes_object) {
	sprintf(msg, "Global detail '%s'", Detail ? "On" : "Off" );
	Announce(msg);
    }
    else if(Object == entries_object) {
	(EntriesView)->Apply( (suite_applyfptr) ToggleCaptionDetail, (long int) self, 0);
	(self)->WantUpdate( EntriesView);
    }
    UpdateDetailCaption(self);
    (self)->RetractCursor( Cursor);
    OUT(PerformDetail);
}

void
bushv::SetDataObject( class dataobject  *dbush )
{
    class bush *bush=(class bush *)dbush;
    class bushv *self = this;
    IN(bushv_SetDataObject);
    BUSH = bush;
    (DirTreeView)->SetDataObject(TREE);
    (this)->aptv::SetDataObject(bush);
    CurrNode = InitNode = TreeRoot;
    HandleChangeDir(this,RootPathName);
    OUT(bushv_SetDataObject);
}

static int
SortRequested( class bushv  *self, tree_type_node  tn )
    {
  int sort = -1, current_mode = SortMode;
  long result = 0;

    IN(SortRequested);
    if(message::MultipleChoiceQuestion(self, 100, "Sort By: ", current_mode, &result, sorts, NULL) != -1)
	switch(result) {
	    case 0:  sort = by_name;   break;
	    case 1:  sort = by_size;   break;
	    case 2:  sort = by_date;   break;
	    case 3:  sort = by_suffix; break;
	    case 4:  sort = by_type;   break;
	    default: sort = -1;
	}
    OUT(SortRequested);
    return(sort);
}

static void
PerformSort( class bushv  *self )
  {
  long int sMode = 0;
  suite_sortfptr sorter;
  struct suite_item *sortItem = NULL;
  char sortCaption[16];

    IN(PerformSort);
    if((sMode = SortRequested(self, CurrNode)) != -1) {
	strcpy(sortCaption, "Sort: ");
	sprintf(msg, "Sorting by '%s' ...", sorts[SortMode = sMode]);
	Announce(msg);
	strcat(sortCaption, sorts[SortMode]);
	if(Object == nodes_object) {
	    sorter = DetermineSortHandler(self, CurrNode);
	    (EntriesView)->SetSuiteAttribute( suite_SortHandler(sorter));
	}
	else if(Object == entries_object) {
	    PostCursor(self, Cursor_Wait);
	    SortDir(self, CurrNode);
	    (self)->RetractCursor( Cursor);
	}
	if(sortItem = (ControlView)->ItemOfDatum( (long int) &sort_data))
	    (ControlView)->ChangeItemAttribute( sortItem, suite_ItemCaption(sortCaption));
    }
    ClearMessageLine();
    OUT(PerformSort);
}

static void
PerformRescan( class bushv  *self )
  {
  struct stat stats;

    IN(PerformRescan);
    if(!CurrNode)
	return;
    PostCursor(self, Cursor_Wait);
    if(stat(DirPath(CurrNode), &stats)) {
	DIRMODE(CurrNode).stat_failed = TRUE;
	IssueError(self, "ReScanning", DirPath(CurrNode), TRUE);
	(self)->RetractCursor( Cursor);
	return;
    }
    if(Object == nodes_object || Object == entries_object) {
	char firstVisibleName[MAXPATHLEN];
	int f;
	struct suite_item *first = (EntriesView)->FirstVisible();

	sprintf(msg, "Scanning Directory '%s' ...", DirPath(CurrNode));
	Announce(msg);

	*firstVisibleName = (char)0;
	if(first) {
	    f = (EntriesView)->ItemAttribute(first, suite_itemposition);
	    while(first && (EntriesView)->ItemActivated(first) == FALSE)
		first = (EntriesView)->ItemAtPosition(++f);
	    if(first)
		strcpy(firstVisibleName, (const char *) (EntriesView)->ItemAttribute( first, suite_itemname));
	}
	if(Child(CurrNode)) {
	    SetTreeNotificationData(self, CurrNode, tree_NodeChildrenDestroyed);
	    NotifyTreeObservers(self);
	}
	(BUSH)->ScanDir(CurrNode);
	(BUSH)->BuildSubDirs(CurrNode);
	if(Child(CurrNode)) {
	    SetTreeNotificationData(self, CurrNode, tree_NodeChildrenCreated);
	    NotifyTreeObservers(self);
	}
	if(Object == entries_object) {
	    ResetEntriesCaptions(self); /* This call clears the current set and rebuilds the new set;  After this call, FirstVisible == NULL; set it to the desired first visible item here */
	    if(*firstVisibleName && (first = (EntriesView)->ItemOfName(firstVisibleName)) != NULL)
		(EntriesView)->SetFirstVisible(first);
	    SortDir(self, CurrNode);
	}

	sprintf(msg, "Finished scanning '%s'", DirPath(CurrNode));
	Announce(msg);
    }
    else if(Object == entry_object)
	PushToEntry(self);
    (self)->RetractCursor(Cursor);
    OUT(PerformRescan);
}

static long
ResetChildDirPaths( class bushv *self, class tree *tree, tree_type_node tn, long datum )
{
  long int	     status = 0;
  char		     tmp_path[MAXPATHLEN];

  IN(ResetChildDirPaths);
  if(tn && (TREE)->NodeLevel(tn) > (TREE)->NodeLevel(CurrNode)) {
    sprintf(tmp_path,"%s/%s",(char*)datum,DirName(tn));
    AllocNameSpace(tmp_path, &DirPath(tn));
  }
  else if(tn && (CurrNode == tn)) status = 0;
  else status = -1;
  OUT(ResetChildDirPaths);
  return(status);
}


static void
PerformRename( class bushv  *self )
  {
  int msg_status = 0, count = 0;
  char *response = NULL;
  char tmp_path[MAXPATHLEN],*tmp = NULL;
  register struct suite_item **selected = NULL;
  struct Dir_Entry *dirEntry = NULL;
  register int i = 0;

  IN(PerformRename);
  if(!CurrNode) return;
  switch(Object) {
    case nodes_object:
      msg_status = (self)->Query("New Name: ",
				DirName(CurrNode),&response);
      if(msg_status || !strcmp(response,DirName(CurrNode))) return;
      sprintf(msg,"Directory '%s' renamed to '",DirName(CurrNode));
      strcpy(tmp_path,DirPath(CurrNode));
      if(tmp = strrchr(tmp_path,'/')) *tmp = '\0';
      if(!(BUSH)->RenameDir(CurrNode,tmp_path,response)) {
        (TREE)->SetNodeName(CurrNode,DirName(CurrNode));
	SetTreeNotificationData(self,CurrNode,tree_NodeNameChanged);
	NotifyTreeObservers(self);
	strcat(msg,DirName(CurrNode));
	strcat(msg,"'");
	Announce(msg);
	(ControlView)->ChangeSuiteAttribute(
				   suite_TitleCaption(DirPath(CurrNode)));
	(TREE)->Apply(CurrNode, (tree_applyfptr) ResetChildDirPaths,
		   (char *) self, strcpy(tmp_path, DirPath(CurrNode)));
      }
      else IssueError(self,"Renaming",DirName(CurrNode),TRUE);
      break;
    case entries_object:
      selected = (EntriesView)->SelectedItems((long int *) &count);		
	for( i = 0 ; (i < count) && selected[i] ; i++ ) {
	  (EntriesView)->PassivateItem(selected[i]);
	  dirEntry = (struct Dir_Entry *)
	    (EntriesView)->ItemAttribute(selected[i],suite_itemdatum);
	  sprintf(msg,"Renaming '%s' to '",dirEntry->name);
	  msg_status = (self)->Query("New Name: ",
				   dirEntry->name,&response);
	  if(msg_status || !strcmp(response,dirEntry->name)) {
	    (EntriesView)->ActivateItem(selected[i]);
	    (EntriesView)->HighlightItem(selected[i]);
	    continue;
	  }
	  if((BUSH)->MoveEntry(CurrNode,dirEntry,response) != -1) {
	    strcat(msg,response); strcat(msg,"'");
	    Announce(msg);
	    (EntriesView)->ActivateItem(selected[i]);
	    (EntriesView)->HighlightItem(selected[i]);
	    (EntriesView)->ChangeItemAttribute(selected[i],
		suite_ItemCaption(FormatEntriesItem(self,NULL,0,dirEntry)));
	  }
	  else {
	    IssueError(self,"Renaming",dirEntry->name,TRUE);
	    (EntriesView)->ActivateItem(selected[i]);
	    (EntriesView)->HighlightItem(selected[i]);
	  }
	}   
	break;
    case entry_object:
      msg_status = (self)->Query("New Name: ",CurrEntry->name,&response);
      if(msg_status || !strcmp(response,CurrEntry->name)) return;
      sprintf(msg,"'%s' renamed to '",CurrEntry->name);
      if((BUSH)->MoveEntry(CurrNode,CurrEntry,response) != -1) {
	strcat(msg,response); strcat(msg,"'");
	Announce(msg);
	sprintf(msg,"%s/%s",DirPath(CurrNode),CurrEntry->name);
	(ControlView)->ChangeSuiteAttribute(suite_TitleCaption(msg));
      }
      else IssueError(self,"Renaming",CurrEntry->name,TRUE);
      break;
  }	
  OUT(PerformRename);
}

static void
ToggleDebug( class bushv     *self )
  {
  IN(ToggleDebug);
  Debug = !Debug;
  (EntriesView)->SetDebug(TRUE);
  OUT(ToggleDebug);
}

class view *
bushv::GetApplicationLayer( )
  {
  class bushv *self=this;
  IN(bushv_GetApplicationLayer);
  TopLevelInset = FALSE;
  OUT(bushv_GetApplicationLayer);
  return((class view *)this);
}

void
bushv::ReceiveInputFocus( )
  {
  IN(bushv_ReceiveInputFocus);
  (this)->aptv::ReceiveInputFocus();
  OUT(bushv_ReceiveInputFocus);
}

static void
HandleChangeDir( register class bushv    *self, register char		   *dirName )
    {
  IN(HandleChangeDir);
  if(dirName && (*dirName != '\0'))
    im::ChangeDirectory(dirName);
  OUT(HandleChangeDir);
}

static int 
bushv_WriteToFile( class bushv   *self, char		 *filename )
    {
  char		 realName[MAXPATHLEN],tempFilename[MAXPATHLEN];
  char		*originalFilename = NULL, *endString, *basename;
  int		 closeCode, errorCode, originalMode, fd, counter = 1;
  FILE		*outFile;
  struct stat	 statBuf;

  IN(bushv_WriteToFile);
  errorCode = 0;
  filetype::CanonicalizeFilename(realName,filename,sizeof(realName) - 1);
  filename = realName;
  if((access(filename,W_OK) < 0) && (errno == EACCES))
    return(-1);
  if(stat(filename,&statBuf) >= 0)
    originalMode = statBuf.st_mode & (~S_IFMT);
  else originalMode = 0666;
#ifndef USESHORTFILENAMES
  strcpy(tempFilename,filename);
  strcat(tempFilename,".NEW");
  endString = tempFilename + strlen(tempFilename);
  while(access(tempFilename,F_OK) >= 0) /* While the file exists. */
    sprintf(endString,".%d",counter++);
#else /* USESHORTFILENAMES */
  strcpy(tempFilename,filename);
  basename = strrchr(tempFilename,'/');
  if(!basename) basename = tempFilename;
  else basename++;
  if(strlen(basename) > 8) basename[8] = '\0';
  strcat(tempFilename,".NEW");
  endString = tempFilename + strlen(tempFilename);
  while(access(tempFilename,F_OK) >= 0 && counter < 10)
    sprintf(endString,".%d", counter++);
  if(counter == 10) return(-1);
#endif /* USESHORTFILENAMES */
  originalFilename = filename;
  filename = tempFilename;
  if((fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT,originalMode)) < 0
      || (outFile = fdopen(fd,"w")) == NULL)
    return(-1);
  (EntryObject)->Write(outFile,im::GetWriteID(),0);
  fflush(outFile);
  if(ferror(outFile)) {
    fclose(outFile);
    errorCode = 0;
    closeCode = -1;
  }
  else {
#ifdef AFS_ENV
    if((closeCode = vclose(fileno(outFile))) < 0) /* stdio can trash errno. */
      errorCode = errno; /* Protect it from the fclose below. */
    else if(originalFilename != NULL)
      if((closeCode = rename(filename, originalFilename)) < 0)
        errorCode = errno;
#else /* AFS_ENV */
    if((closeCode = close(fileno(outFile))) < 0) /* stdio can trash errno. */
      errorCode = errno; /* Protect it from the fclose below. */
    else if(originalFilename != NULL)
      if((closeCode = rename(filename, originalFilename)) < 0)
        errorCode = errno;
#endif /* AFS_ENV */
    fclose(outFile); /* Free stdio resources. */
    if(closeCode >= 0) { /* Reset readonly mode. */
      struct attributes attributes;

      attributes.next = NULL;
      attributes.key = "readonly";
      if(access(filename,W_OK) == -1 && errno == EACCES)
        attributes.value.integer = TRUE;
      else attributes.value.integer = FALSE;
      (EntryObject)->SetAttributes(&attributes);
      EntryObjectModified = (EntryObject)->GetModified();
    }
  }
  sprintf(tempFilename,"%s.CKP",filename);
  if(access(tempFilename,F_OK) >= 0)
    unlink(tempFilename);
  errno = errorCode;
  OUT(bushv_WriteToFile);
  return(closeCode);
}

#define DIR_MSG \
   "Write aborted: specified output file is a directory."
    
static int
bushv_SaveFile( class bushv	 *self )
  {
  int		 result = 0, return_value = 0;
  char		 message[sizeof("Wrote file ''.") + sizeof("Could not save file") + MAXPATHLEN];
  char		 fName[MAXPATHLEN];
  char		*filename;
  struct stat    statbuf;

  IN(bushv_SaveFile);
  sprintf(fName,"%s/%s",DirPath(CurrNode), CurrEntry->name);
  filename = fName;
  if(stat(filename, &statbuf) == 0 && (statbuf.st_mode & S_IFDIR)) {
    Announce(DIR_MSG);
    return(-1);
  }
  PostCursor(self,Cursor_Wait);
  result = bushv_WriteToFile(self,filename);
  if(result >= 0) {
    sprintf(message, "Wrote file '%.*s'", MAXPATHLEN, filename);
    Announce(message);
  }
  else {
    switch (errno) {
      case EACCES:
        Announce("Could not save file; permission denied.");
        break;
#ifdef ETIMEDOUT
      case ETIMEDOUT:
        Announce("Could not save file; a server is down.");
        break;
#endif /* ETIMEDOUT */
#ifdef EFAULT
      case EFAULT:
        Announce("Could not save file; a server is down.");
        break;
#endif /* EFAULT */
#ifdef EDQUOT
      case EDQUOT:
        Announce("Could not save file; you are over your quota.");
        break;
#endif /* EDQUOT */
       case ENOSPC:
        Announce("Could not save file; no space left on partition.");
        break;
#ifdef EIO
       case EIO:
        Announce("Could not save file; an I/O error occurred on the disk.");
        break;
#endif /* EIO */
       case EISDIR:
        Announce("File not found; could not create. Attempt to write to a directory.");
        break;
       default:
        sprintf(message, "Could not save file: %s.",strerror(errno));
        Announce(message);
    }
    return_value = -1;
  }
  (self)->RetractCursor(Cursor);
  OUT(bushv_SaveFile);
  return(return_value);
}

static int
bushv_WriteFile( class bushv	 *self )
  {
  char		 filename[MAXPATHLEN];
  char		 message[sizeof("Wrote file ''.") + sizeof("Could not save file") + MAXPATHLEN];
  int		 result = 0, return_value = 0;

  IN(bushv_WriteFile);
  sprintf(filename,"%s/%s",DirPath(CurrNode),CurrEntry->name);
  if(completion::GetFilename(self, "Write to file: ", filename, filename,
			     sizeof(filename), FALSE, FALSE) == -1)
    return(-1);
  PostCursor(self,Cursor_Wait);
  result = bushv_WriteToFile(self,filename);
  if(result >= 0) {
    sprintf(message, "Wrote file '%.*s'", MAXPATHLEN, filename);
    Announce(message);
  }
  else {
    switch (errno) {
      case EACCES:
        Announce("Could not save file; permission denied.");
        break;
#ifdef ETIMEDOUT
      case ETIMEDOUT:
        Announce("Could not save file; a server is down.");
        break;
#endif /* ETIMEDOUT */
#ifdef EFAULT
      case EFAULT:
        Announce("Could not save file; a server is down.");
        break;
#endif /* EFAULT */
#ifdef EDQUOT
      case EDQUOT:
        Announce("Could not save file; you are over your quota.");
        break;
#endif /* EDQUOT */
      case ENOSPC:
        Announce("Could not save file; no space left on partition.");
        break;
#ifdef EIO
      case EIO:
        Announce("Could not save file; an I/O error occurred on the disk.");
        break;
#endif /* EIO */
      case EISDIR:
        Announce("File not found; could not create. Attempt to write to a directory.");
        break;
      default:
        sprintf(message, "Could not save file: %s.",strerror(errno));
        Announce(message);
    }
    return_value = -1;
  }
  (self)->RetractCursor(Cursor);
  OUT(bushv_WriteFile);
  return(return_value);
}

static int
bushv_SetPrinter(class bushv	 *self)
  {
  char		 answer[256];
  const char    *currentPrinter, *defaultPrinter;
  char		 prompt[sizeof("Current printer is . Set printer to []: ") + 128];
  int		 return_value = 0;

  currentPrinter = environ::GetPrinter();
  defaultPrinter = environ::GetProfile("print.printer");
  if (!defaultPrinter) defaultPrinter = environ::GetProfile("print.spooldir");
  if((currentPrinter != NULL) && (defaultPrinter != NULL))
    sprintf(prompt,"Current printer is %.64s. Set printer to [%.64s]: ",    currentPrinter,defaultPrinter);
  else if(defaultPrinter != NULL)
    sprintf(prompt,"Set printer to [%.64s]: ",defaultPrinter);
  else strcpy(prompt,"Set printer to: ");
  if(message::AskForString(self,0,prompt,NULL,answer,sizeof(answer)) == -1)
    return(-1);
  if(*answer != '\0') {
    environ::PutPrinter(answer);
    defaultPrinter = answer;
  }
  else environ::DeletePrinter();
  if(defaultPrinter != NULL) {
    sprintf(prompt,"Printer set to %.64s.",defaultPrinter);
    Announce(prompt);
  }
  else {
    Announce("Printer not set.");
    return_value = -1;
  }
  return(return_value);
}

static void 
Checkpoint( long  dummyData )
  {
  class bushv	*self = (class bushv*)dummyData;
  char		 CkpFileName[MAXPATHLEN];
  int		 closeCode;

  if(Object == entry_object) {
    if((EntryObject)->GetModified() > EntryObjectLastCKP) {
      PostCursor(self,Cursor_Wait);
      Announce("Checkpointing ...");
      im::ForceUpdate();
      sprintf(CkpFileName,"%s/%s.CKP",DirPath(CurrNode),CurrEntry->name);
      closeCode = bushv_WriteToFile(self,CkpFileName);
      Announce(closeCode ? "Checkpoint Failed." : "Checkpointed.");
      (self)->RetractCursor(Cursor);
      EntryObjectLastCKP = (EntryObject)->GetModified();
    }
    im::EnqueueEvent((event_fptr) Checkpoint, (char *)self, event_SECtoTU(CkpInterval));
  }
}

static void
UpdateDetailCaption( class bushv		     *self )
  {
  struct suite_item	    *detailItem = NULL;
  char			     newCaption[16];

  if(detailItem = (ControlView)->ItemOfDatum((long int) &detail_data)) {
    strcpy(newCaption, "Detail: ");
    if(Detail) strcat(newCaption, "on");
    else strcat(newCaption, "off");
    (ControlView)->ChangeItemAttribute( detailItem, suite_ItemCaption(newCaption));
  }
}

static void
EntriesPageUp( class bushv  *self )
    {
    if(Object == entries_object) {
	struct suite_item *first, *last, *newFirst;

	if((first = (EntriesView)->FirstVisible()) != NULL && (last = (EntriesView)->LastVisible()) != NULL && first != last) {
	    int f = (EntriesView)->ItemAttribute( first, suite_itemposition);
	    int l = (EntriesView)->ItemAttribute( last, suite_itemposition);
	    int total = (EntriesView)->ItemCount();
	    int span = l - f, n;

	    n = (f - span > 0 ? f - span : 1);
	    if((newFirst = (EntriesView)->ItemAtPosition( n)) != NULL && newFirst != first) {
		(EntriesView)->SetFirstVisible( newFirst);
		(self)->WantUpdate( EntriesView);
	    }
	    else if(newFirst == first) Announce("Top of list");
	}
    }
}

static void
EntriesPageDown( class bushv  *self )
    {
    if(Object == entries_object) {
	struct suite_item *first, *last, *newFirst;

	if((first = (EntriesView)->FirstVisible()) != NULL && (last = (EntriesView)->LastVisible()) != NULL && first != last) {
	    int f = (EntriesView)->ItemAttribute( first, suite_itemposition);
	    int l = (EntriesView)->ItemAttribute( last, suite_itemposition);
	    int total = (EntriesView)->ItemCount();

	    if(l != total) {
		int span = l - f, n;

		if(l + span > total)
		    n = total - span;
		else
		    n = (f + span < total ? f + span : total - 1);
		if((newFirst = (EntriesView)->ItemAtPosition( n)) != NULL && newFirst != first) {
		    (EntriesView)->SetFirstVisible( newFirst);
		    (self)->WantUpdate( EntriesView);
		}
	    }
	    else Announce("Last page");
	}
    }
}

void
bushv::LinkTree(class view  *parent)
        {
    class bushv *self=this;
    (this)->aptv::LinkTree( parent);
    if((this)->GetIM()) {
	(LP)->LinkTree( (class view *) this);
   }
}
