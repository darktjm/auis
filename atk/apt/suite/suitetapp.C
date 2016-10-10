/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/**********************************\
**                                **
**  suite CLIENT/TEST PROGRAM     **
**                                **
\**********************************/

#include <andrewos.h>
ATK_IMPL("suitetapp.H")
#include <frame.H>
#include <im.H>
#include <filetype.H>
#include <lpair.H>
#include <text.H>
#include <textview.H>
#include <cursor.H>
#include <suite.H>
#include <apt.H>
#include <apts.H>
#include <suitetapp.H>

 
            
	     
             
             
	    
             
            
	     
  
ATKdefineRegistry(suitetapp, application, NULL);
static class view * Change_Test( class suitetapp  *self );
static class view * First_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Last_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Number_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Next_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Prior_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Quit_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Name_Choice( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * RW_Hit_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
static class view * Text_Object_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type );
static class view * TextView_Object_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type );
static class view * Alphabet_Sort( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks );
          

#define NUM_NAMES 27
static const char * const names[NUM_NAMES] = {
  "Adams, Joe",	
  "Brown, Sally",   
  "Crandall, Harry",
  "Donovan, Joe",
  "Ellison, Mark",
  "Frankfurter, Nancy",
  "Gregson, Tom",
  "Henry, Betty",
  "Iverson, Greg",
  "Johansohn, Harry",
  "Klein, Larry",
  "Lewis, Donna", 
  "Morgan, Ken",
  "Noonan, Curt",	
  "Oppenheimer, Sam", 
  "Peterson, Robert",
  "Quiz, Don",		
  "Robertson, Jack",  
  "Samuals, Frank",
  "Thompson, Andrew",	
  "Ullanlong, Waldo", 
  "Victor, Wally",
  "Wallace, Charles",	
  "Xanadu, Paul",	    
  "Yellow, Edward",
  "Zweig, Bronson",
  NULL
};

static const char * const alphabet[] =
  { "A","B","C","D","E","F","G","H","I","J","K","L","M",
    "N","O","P","Q","R","S","T","U","V","W","X","Y","Z", NULL };

static const char * const list_items[] = {
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
    "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD",
    "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE",
    "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",
    "HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH",
    "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII",
    "JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ",
    "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK",
    "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL",
    "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM",
    "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN",
    "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
    "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP",
    "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ",
    "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR",
    "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
    "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
    "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU",
    "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV",
    "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",
    "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
    "YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY",
    "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ",
    NULL
};

static suite_Specification phones_1[] = {
  suite_TitleCaption( "Phone Names -- ONE" ),
  suite_ItemCaptionList( names ),
  suite_HitHandler( Name_Choice ),
  suite_Arrangement( suite_Matrix | suite_Fixed ),
  suite_Columns( 2 ), suite_Rows( 13 ),
  0
};

static suite_Specification phones_2[] = {
  suite_TitleCaption( "Phone Names -- TWO" ),
  suite_TitleFontName( "andy14i" ),
  suite_ItemCaptionList( names ),
  suite_ItemOrder( suite_ColumnMajor ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_3[] = {
  suite_TitleCaption( "Phone Names -- THREE" ),
  suite_TitleBorderStyle( suite_Invisible ),
  suite_TitleFontName( "andysans10bi" ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemHighlightStyle( suite_Bold | suite_Italic ),
  suite_ItemOrder( suite_RowMajor ),
  suite_ItemBorderStyle( suite_Invisible ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_4[] = {
  suite_TitleCaption( "Phone Names -- FOUR" ),
  suite_TitleFontName( "andy22" ),
  suite_TitleBorderStyle( suite_Line ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andy16" ),
  suite_ItemHighlightStyle( suite_Border ),
  suite_ItemOrder( suite_ColumnMajor ),
  suite_ItemBorderStyle( suite_Invisible ),
  suite_Scroll( suite_Left ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_5[] = {
  suite_TitleCaption( "Phone Names -- FIVE" ),
  suite_TitleFontName( "andysans10bi" ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemHighlightStyle( suite_Bold | suite_Italic ),
  suite_ItemOrder( suite_RowMajor ),
  suite_ItemBorderSize( 3 ),
  suite_BorderSize( 5 ),
/*===  suite_Arrangement( suite_RowLine ),===*/
  suite_SelectionMode( suite_Inclusive ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_6[] = {
  suite_TitleCaption( "Phone Names -- SIX" ),
  suite_TitleFontName( "andysans10bi" ),
  suite_TitleBorderStyle( suite_Rectangle ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans10b" ),
  suite_ItemHighlightStyle( suite_Invert ),
  suite_ItemOrder( suite_RowMajor ),
  suite_HitHandler( Name_Choice ),
  suite_Scroll( suite_Right ),
  0
};

static suite_Specification phones_7[] = {
  suite_TitleCaption( "Phone Names -- SEVEN" ),
  suite_TitleFontName( "andysans10bi" ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemHighlightStyle( suite_Bold | suite_Italic ),
  suite_ItemOrder( suite_RowMajor ),
  suite_ItemBorderStyle( suite_Rectangle ),
  suite_ItemCaptionAlignment( suite_Right | suite_Middle),
  suite_SelectionMode( suite_Inclusive ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_8[] = {
  suite_TitleCaption( "Phone Names -- EIGHT" ),
  suite_TitleFontName( "andysans12b" ),
  suite_TitlePlacement( suite_Bottom ),
  suite_TitleBorderSize( 4 ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemHighlightStyle( suite_Bold | suite_Italic ),
  suite_ItemCaptionAlignment( suite_Center | suite_Bottom ),
  suite_ItemBorderStyle( suite_Invisible ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_9[] = {
  suite_TitleCaption( "Phone Names -- NINE" ),
  suite_TitleFontName( "andysans10bi" ),
  suite_TitlePlacement( suite_Left ),
  suite_TitleBorderSize( 7 ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemCaptionAlignment( suite_Left | suite_Middle),
  suite_ItemHighlightStyle( suite_Bold | suite_Italic ),
  suite_ItemBorderStyle( suite_Invisible ),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification phones_10[] = {
  suite_TitleCaption( "Phone Names -- TEN" ),
  suite_TitleFontName( "andysans10b" ),
  suite_TitlePlacement( suite_Top ),
  suite_TitleBorderStyle( suite_Line ),
  suite_ItemCaptionList( names ),
  suite_ItemCaptionFontName( "andysans8" ),
  suite_ItemCaptionAlignment( suite_Left | suite_Top),
  suite_ItemHighlightStyle( suite_Bold ),
  suite_Arrangement( suite_Matrix | suite_Fixed ),
  suite_Columns( 2 ),
  suite_Rows( 10 ),
  suite_ItemBorderStyle( suite_Invisible ),
  suite_Scroll( suite_Left ),
  suite_HitHandler( Name_Choice ),
  suite_ForegroundColor( "red" ),
  suite_BackgroundColor( "blue" ),
  suite_ActiveItemForegroundColor( "tan" ),
  suite_ActiveItemBackgroundColor( "green" ),
  0
};

static suite_Specification list_11[] = {
  suite_TitleCaption( "Alphabet Soup -- Eleven" ),
  suite_TitleFontName( "andysans10b" ),
  suite_TitlePlacement( suite_Top ),
  suite_ItemCaptionList( list_items ),
  suite_ItemCaptionFontName( "andytype10" ),
  suite_ItemBorderSize( 3 ),
  suite_Arrangement( suite_List | suite_Unbalanced | suite_ColumnLine),
  suite_ItemOrder( suite_ColumnMajor ),
  suite_Scroll( suite_Right),
  suite_HitHandler( Name_Choice ),
  0
};

static suite_Specification rw_item_one_1[] = {
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemTitleCaption( "Name:" ),
  suite_ItemTitlePlacement( suite_Left ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  suite_ItemData( "rw_item_one_1" ),
  0
};

static suite_Specification rw_item_one_2[] = {
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemTitleCaption( "Address:" ),
  suite_ItemTitlePlacement( suite_Left ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  suite_ItemData( "rw_item_one_2" ),
  0
};

static suite_Specification read_write_one[] = {
  suite_TitleCaption( "Read/Write Alpha" ),
  suite_Item( rw_item_one_1 ),
  suite_Item( rw_item_one_2 ),
  0
};

static suite_Specification rw_item_two_1[] = {
  suite_ItemTitleCaption( "ID:" ),
  suite_ItemData( "rw_item_two_1" ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  0
};

static suite_Specification rw_item_two_2[] = {
  suite_ItemTitleCaption( "Serial:" ),
  suite_ItemData( "rw_item_two_2" ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  0
};

static suite_Specification rw_item_two_3[] = {
  suite_ItemTitleCaption( "Rank:" ),
  suite_ItemData( "rw_item_two_3" ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  0
};

static suite_Specification rw_item_two_4[] = {
  suite_ItemTitleCaption( "Weight:" ),
  suite_ItemData( "rw_item_two_4" ),
  suite_ItemHitHandler( RW_Hit_Handler ),
  0
};

static suite_Specification read_write_two[] = {
  suite_TitleCaption( "Read/Write Beta" ),
  suite_Item( rw_item_two_1 ),
  suite_Item( rw_item_two_2 ),
  suite_Item( rw_item_two_3 ),
  suite_Item( rw_item_two_4 ),
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemTitleFontName( "andysans14b" ),
  suite_ItemTitlePlacement( suite_Top ),
  suite_ItemCaptionFontName( "andy12" ),
  suite_Arrangement(suite_Matrix | suite_Fixed ),
  suite_Columns( 2 ), suite_Rows( 2 ),
  0
};

static suite_Specification icons_1[] = {
  suite_ItemCaptionFontName( "zipicn20" ),
  suite_ItemCaptionList( alphabet ),
  suite_ItemWidth( 50 ),
  suite_ItemHeight( 50 ),
  suite_Cursor( 'T' ),
  0
};



static suite_Specification alphabet_0[] = {
  suite_TitleCaption( "Alphabet --- ZERO" ),
  suite_ItemCaptionList( alphabet ),
  suite_HitHandler( Alphabet_Sort ),
  0
};

static suite_Specification alphabet_1[] = {
  suite_TitleCaption( "Alphabet --- ONE" ),
  suite_ItemCaptionList( alphabet ),
  suite_VerticalGutterSize( 3 ),
  suite_Arrangement( suite_RowLine ),
  0
};

static suite_Specification alphabet_2[] = {
  suite_TitleCaption( "Alphabet --- TWO" ),
  suite_ItemCaptionList( alphabet ),
  suite_HorizontalGutterSize( 3 ),
  suite_ItemOrder( suite_ColumnMajor ),
  suite_Arrangement( suite_ColumnLine | suite_Unbalanced ),
  0
};

static suite_Specification alphabet_3[] = {
  suite_TitleCaption( "Alphabet --- THREE" ),
  suite_ItemCaptionList( alphabet ),
  suite_VerticalGutterSize( 3 ),
  suite_HorizontalGutterSize( 3 ),
  suite_Arrangement( suite_RowLine | suite_ColumnLine | suite_Unbalanced ),
  0
};

static suite_Specification alphabet_4[] = {
  suite_TitleCaption( "Alphabet --- FOUR" ),
  suite_ItemCaptionList( alphabet ),
  suite_Arrangement( suite_Matrix | suite_Unbalanced ),
  suite_ItemWidth( 50 ),
  suite_ItemHeight( 50 ),
  0
};

static suite_Specification catalog_1_a[] = {
  suite_ItemData( 1 ),
  0
};

static suite_Specification catalog_1_b[] = {
  suite_ItemData( 2 ),
  0
};

static suite_Specification catalog_1[] = {
  suite_TitleCaption( "Catalog --- ONE" ),
  suite_ItemDataObjectName( "text" ),
  suite_ItemDataObjectHandler( Text_Object_Handler ),
  suite_ItemViewObjectName( "textview" ),
  suite_ItemViewObjectHandler( TextView_Object_Handler ),
  suite_ItemHighlightStyle( suite_None ),
  suite_CursorType( Cursor_Arrow ),
  suite_Item( catalog_1_a ),
  suite_Item( catalog_1_b ),
  0
};

static suite_Specification *test_suites[] = {
  phones_1,
  phones_2,
  phones_3,
  phones_4,
  phones_5,
  phones_6,
  phones_7,
  phones_8,
  phones_9,
  phones_10,
  list_11,
  read_write_one,
  read_write_two,
  icons_1,
  alphabet_0,
  alphabet_1,
  alphabet_2,
  alphabet_3,
  alphabet_4,
  catalog_1,
  0
};

static long current_test = 0, last_test = 20;
static class suite *current_suite;

static suite_Specification test_suite[] = {
  suite_TitleCaption( "Test Suite" ),
  0
};

#define	first	1
#define	next	2
#define	prior	3
#define	last	4
#define	number	5
#define	quit	6

static suite_Specification button_first[] = {
  suite_ItemCaption( "First" ),
  suite_ItemData( first ),
  suite_ItemHitHandler( First_Test ),
  0
};

static suite_Specification button_next[] = {
  suite_ItemCaption( "Next" ),
  suite_ItemData( next ),
  suite_ItemHitHandler( Next_Test ),
  0
};

static suite_Specification button_prior[] = {
  suite_ItemCaption( "Prior" ),
  suite_ItemData( prior ),
  suite_ItemHitHandler( Prior_Test ),
  0
};

static suite_Specification button_last[] = {
  suite_ItemCaption( "Last" ),
  suite_ItemData( last ),
  suite_ItemHitHandler( Last_Test ),
  0
};

static suite_Specification button_number[] = {
  suite_ItemAccessMode( suite_ReadWrite ),
  suite_ItemTitleCaption( "Number:" ),
  suite_ItemTitlePlacement( suite_Left ),
  suite_ItemData( number ),
  suite_ItemHitHandler( Number_Test ),
  0
};

static suite_Specification button_quit[] = {
  suite_ItemCaption( "Quit" ),
  suite_ItemData( quit ),
  suite_ItemHitHandler( Quit_Test ),
  0
};

static suite_Specification button_suite[] = {
  suite_TitleCaption( "Test Case Controller" ),
  suite_TitleCaptionFontName("andysans14b"),
  suite_TitlePlacement( suite_Top ),
  suite_Item( button_first ),
  suite_Item( button_next ),
  suite_Item( button_prior ),
  suite_Item( button_last ),
  suite_Item( button_number ),
  suite_Item( button_quit ),
  suite_Arrangement( suite_Matrix | suite_Fixed),
  suite_Columns( 3 ), suite_Rows( 2 ),
  0
};

static class lpair *pair, *triple;
static class text *text_data;
static class textview *text_view;
static class view *button_view, *test_view;

static int suiteta_debug = 0;
#define debug suiteta_debug



boolean
suitetapp::ParseArgs( int  argc, const char  **argv )
      {
  IN(suiteta_ParseArgs);
  (this)->application::ParseArgs( argc, argv);
  debug = 0;
  while (*++argv) { DEBUGst(ARGV, *argv);
    if (**argv == '-') {
      if (strcmp(*argv, "-d") == 0)
        debug = 1;
    }
  }
  OUT(suiteta_ParseArgs);
  return(TRUE);
}

boolean
suitetapp::Start( )
  {
  FILE *file;
  long id;
  class im *im;
  class frame *framep;
  static const char instructions[] = "Use the Buttons to step through the tests.";

  if(debug)
      (this)->SetFork(FALSE);

  button_view = (class view *) suite::Create(button_suite, (long)this);
  ((class suite*)button_view)->SetDebug(TRUE);
  test_view   = (class view *) suite::Create(test_suite, (long)this);
  ((class suite*)test_view)->SetDebug(TRUE);
  text_data = new text;
  (text_data)->InsertCharacters( 0, instructions, sizeof(instructions) - 1);
  if ((file = fopen("suiteta.c", "r"))) {
      filetype::Lookup(file, (char *) 0, &id, 0);
      (text_data)->Read( file, id);
      fclose(file);
  }
  text_view = new textview;
  (text_view)->SetDataObject( text_data);
  text_view = (class textview *) (text_view)->GetApplicationLayer();
  pair = new lpair;
  (pair)->HSplit( text_view, test_view, 50, 1);
  triple = new lpair;
  (triple)->VSplit( pair, button_view, 20, 1);
  framep = new frame;
  if((im = im::Create(0)) == 0) {
      fprintf(stderr,"Could not create new window on this display.\n");
      fprintf(stderr,"exiting.\n");
      exit(-1);
  }
  (framep)->SetView(triple);
  (im)->SetView(  framep );
  return 1;
  }

static class view *
Change_Test( class suitetapp  *self )
  {
  class suite *prior_suite = current_suite;

  apt_StartTimer;
  current_suite = suite::Create(test_suites[current_test], (long)self);
  if (current_suite) {
      if(debug) apt_PrintTimer(Creation);
    (pair)->SetNth( 1, current_suite);
    (current_suite)->SetDebug( debug);
  }
  if (prior_suite)
    (prior_suite)->Destroy();
  return(0);
}

static class view *
First_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if (action == view::LeftUp) {
    current_test = 0;
    Change_Test(self);
    (suite)->NormalizeItem( item);
  }
  return(0);
}

static class view *
Last_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {

  if (action == view::LeftUp) {
    current_test = last_test;
    Change_Test(self);
    (suite)->NormalizeItem( item);
  }
  return(0);
}

static class view *
Number_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  long test;

  if(action == view::LeftDown)
    (suite)->ChangeItemAttribute( item, suite_ItemCaption(""));
  else {
      if((suite)->ItemAttribute( item, suite_itemcaption)) {
	  test = atoi( (char *)(suite)->ItemAttribute(  item, suite_itemcaption ) );
	  if(test >= 0 || test <= last_test) {
	      current_test = test;
	      Change_Test(self);
	  }
      }
      (suite)->NormalizeItem( item);
  }
  return(0);
}

static class view *
Next_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if(action == view::LeftUp) {
      if(current_test < last_test && test_suites[current_test+1])
	  current_test++;
      else
	  current_test = 0;
      Change_Test(self);
      (suite)->NormalizeItem( item);
  }
  return(0);
}

static class view *
Prior_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if(action == view::LeftUp) {
    if(current_test > 0)
	current_test--;
    else
	current_test = last_test - 1;
    Change_Test(self);
    (suite)->NormalizeItem( item);
  }
  return(0);
}

static class view *
Quit_Test( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  if(action == view::LeftUp) {
    (suite)->NormalizeItem( item);
    exit(0);
    return(0);
  }
  return 0;
}

static class view *
Name_Choice( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  printf("Title Caption '%s' Item Caption '%s'\n",
	    (char *)(suite)->SuiteAttribute( suite_titlecaption),
	    (char *)(suite)->ItemAttribute( item, suite_itemcaption));
  return(0);
}

static class view *
RW_Hit_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks )
            {
  printf("Title Caption '%s'  Item Caption '%s'\n",
	    (char *)(suite)->SuiteAttribute( suite_titlecaption),
	    (char *)(suite)->ItemAttribute( item, suite_itemcaption));
  return(0);
}

static class view *
Text_Object_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type )
        {
  static const char words[]= "Hello.  I am a Text Inset; just tickle me to make yourself a believer.  I am not showing myself with a Scrollbar, for I feel that would impinge upon your view of this textual material.  Further, I am seen only as plain text, because that is all I was born with (if my creator had elected to read me from a file, or do the necessary text-operations, I could have Bold, and all the formatted styles.)";
  ((class text *) (suite)->ItemDataObject( item))->InsertCharacters( 0, words, sizeof(words) - 1);
  return(0);
}

static class view *
TextView_Object_Handler( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type )
        {
  ((class textview *) (suite)->ItemViewObject( item))->SetDataObject( (suite)->ItemDataObject( item));
  return(0);
}

static class view *
Alphabet_Sort( class suitetapp  *self, class suite  *suite, struct suite_item  *item, long  type, enum view::MouseAction  action, long  x , long  y , long  clicks ) 
            {
  static long sort = suite_Ascend;
  static const char *sorted, *forward = "Sorted Ascending",
  *backward = "Sorted Descending";

  if(action == view::RightUp) {
      if(sort == suite_Ascend) {
	  sort = suite_Descend;
	  sorted = backward;
      }
      else {
	  sort = suite_Ascend;
	  sorted = forward;
      }
      (suite)->Sort( suite_Alphabetic | sort, 0);
      (suite)->ChangeSuiteAttribute( suite_TitleCaption(sorted));
  }
  return(0);
}
