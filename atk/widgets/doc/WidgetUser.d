\begindata{text,6689888}
\textdsversion{12}
\template{default}
\define{itemize
}
\define{enumerate
menu:[Region~4,Enumerate~30]
attr:[LeftMargin LeftMargin Inch 32768]}
\define{excerptedcaption

attr:[LeftMargin LeftMargin Inch 32768]
attr:[RightMargin RightMargin Inch 32768]
attr:[FontFace Bold Int Set]
attr:[FontFace Italic Int Set]}
\define{literal
menu:[Region~4,Literal~62]
attr:[Flags PassThru Int Set]}
\define{black
menu:[Color,Black]
attr:['color' 'Black']}
\define{fixedtext
menu:[Region~4,Fixedtext]
attr:[Justification LeftJustified Point 0]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]
attr:[FontSize ConstantFontSize Point 10]}
\define{fixedindent
menu:[Region~4,Fixedindent]
attr:[LeftMargin LeftMargin Cm 83230]
attr:[Justification LeftJustified Point 0]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]
attr:[FontSize ConstantFontSize Point 10]}
\define{paramname
menu:[Font~1,Paramname]
attr:[FontFace Italic Int Set]}
\define{helptopic
menu:[Region,Helptopic]
attr:[FontFace Italic Int Set]
attr:[FontFace Bold Int Clear]}
\define{function
}
\define{keyword
}


Design


all documentation of awidget and widgets

aslot and avarset are separate






\bold{\italic{Andrew}}

\bold{User Interface System}



\majorheading{\bold{Widget User's Guide}}



\flushright{author: Fred Hansen

January, 1996}



what widgets supported:



how to use each widget


how to establish attributes paths


simple attribute changes



using widgets


there are three levels of usage:


User: as part of using an application

Author: as part of building an application

Developer: in building a new widget


An author can build new widgets specifically for a document.  \




button

	shape: rectangle, diamond, circle, triangle, no border

	contents: empty, text, figure drawing, inset

		or an icon image (X bitmap or Xpm pixmap)

	visibility: visible, invisible

	behavior: click, on/off, radio, checkbox, none, editable

shirt (has buttonholes for putting in buttons)

	multiplicity: one, n columns, m rows, scattered

	two or three adjacent buttons for \


		RadioButtons and SpinBox



all attributes may be adjusted


there are some standard buttons that choose attributes

multiplicity (default one) and contents (default text) may be adjusted


Label: rectangle, no action

PushButton: rectangle, click, text \


ToggleButton: rectangle, text, on/off

TextResponse: rectangle, text, editable

	also offers the option of a menu of values

DrawnButton: rectangle, click, figure drawing

CheckBox: small square, on/off

RadioButton: diamond, radio

Indicator: circle, no action, application changes contents

ArrowButton: rectangle, icon, click


SpinBox:  various ways to switch among a multitude of values

	click on arrows or popup a menu

List - choose from list of strings;  perhaps with scrollbar

Scalar

	Slider, TextNum, Thumb, Bar, Dial, Gauge

XYGraph

PieChart

	\



For more details, see WidgetAuthor.d




\section{WidgetPath}


The WidgetPath is a list of directories in which to look for the 
definitions of widgets.  It can be defined in the ~/preferences file or 
with a variable in the environment.  (The name is caseless in the 
preferences file, but must be all caps as an environment variable.)  The 
definition for a widget of type wwww is a file named wwww.wgt (extension 
".wgt") in one of those directories.


Several domains of widget definition can be discerned: document, user, 
group, site, and system.  All but the first of these correspond to 
directories that may be on the WidgetPath.  File permissions define which 
domain a widget is in;  normally, users can modify widget definitions of 
widgets in the user and group domains.


The document domain exists because an author can define a widget solely for 
use within a given document:  its definition and multiple usages are within 
the same document.


xxx how is this done?


A widget definition can be in a hidden area of text.  More usually, a 
widget definition is just a constructed widget which is later referenced 
from elsewhere in the document.  If a widget is cut or copied, the 
datastream gets its id and the window id.  If pasted into the same 
document, the widget id is used to make a reference;  if pasted in another 
document, the window id is used and the attributes are fetched through a 
selection mechanism.  (Maybe before fetching, the widget is sought in the 
local dictionary;  but how do we make names unique in there?)  (What 
happens if original window goes away?)



How does the above differ from having a created widget be based on a null 
widget?  With the latter approach, the cut/copy value is just a set of 
attributes.  But then when we paste multiple instances how do we reutilize 
trhe original definition?  \



Do we reuse definitions in any meaningful way?  One design would have only 
the modified attributes stored locally and all others accessed from parents.



This is the debate between caching all attributes with each widget instance 
or doing dynamic lookup each time an attrribute is referenced.


Three approaches:


static:  all attributes of a widget are present for each instance

	can use call-backs to link values

formulaic:  some attribute values can be references to

	other attributes, or functions that compute the value

dynamic: every attribute reference is done by a lookup


\{AMULET may do things by have the slot lists share tails.\}




Proposal:  The cut buffer gets widget class and whatever attributes would 
be written to the data stream.  (This is exactly what is written to file.) 
 Pasting works fine.


This does not solve the problem with ad hoc widgets.


An ad hoc widget is identified by having a distinct widgetclass name.  When 
the widgetclass name is set in the widget, this signals that this is an ad 
hoc widget for this document.  The former class name is preserved 
(somewhere) to use to create an 'include' line in the widget definition.


The first time an ad hoc widget is written, its definition must be written, 
too.


There ought to be a way to store an ad hoc widget into the user's widget 
file space.



\section{Attribute inheritance}


command line

data stream

parents

files

	WidgetPath


as coded in December 95, ALL files matching the name are fetched from the 
path.  This allows overriding attributes.  Perhaps it would be better to do 
this in resource files.


Need some way to save a prototype of each widget.  AMULET actually does 
widget creation from existing objects.



\section{Scenarios}


Users, authors, and developers can perform various tasks.  For each, the 
sections below describe how the person proceeds and where the results are 
stored.


\subsection{Scenario: Create or modify a widget definition}


The result of creating a widget is one or more files stored in a directory 
on, or to be included in, WidgetPath.  \



Widget creation is initiated by a widget editor invoked for files with 
extension .aw.  The primary window is a list of attributes.  A second 
window (or pane in the primary window)  shows an image, initially the 
ScreenImage attribute.  Usually a third window will house the "figure" 
toolset.


Widgets can also be created as a .C file, they must derive from awidget or 
some subclass which is also part of the statically loaded system (runapp). 
 In this case, they will be installed as dynamically loadable files in the 
ANDREWDIR path.


\subsection{Scenario: Tailor a widget or class}


Widgets can be tailored two ways:  specify attributes in ~/preferences or 
create a file in a domain subsidiary to the widget's definition.   The 
preferences editor needs to be extended to deal with widget attributes 
using the same tools as the widget editor.


\subsection{Scenario: Insert a widget}


'ESC-TAB widget' brings up the widget selector.


`ESC-TAB widgetname`  brings up a widget of that name.


copy-paste gets a widget.


There can be a chooser window (ala ADEW) in which a single click inserts a 
widget copy in the cut buffer.


It might be nice to have a mode where clicking on a widget would clone it.


\subsection{Scenario: Adapt a widget}


In "author" mode, a click on a widget brings up its attributes in the 
toolset window.



Files to read for resources:

	source system defaults

	widget specific defaults

	site immutable

	site overrideable

	personal

	project

	resources are also read in from the data stream for an object


resources priority order:

	1st: runtime assignments

	2nd:  cmd line

	3rd: datastream

	4th: inherit from tree

	5th: resource files for widget

search order

	the resource mechanism looks in numerous places for a resource value, \


	taking the first it finds.  the sequence of searching is

	1) site immutable resource file.  resources specified in this file cannot \


	be overriden by any of the other mechanisms.  this file should be quite \


	small or non-existent.

	\


	2) command line.  the usual x syntax for specifying resources on the \


	command line is used.

	\


	3) data stream.  when an author creates a widget, some attributes can be \


	tailored to its use.  these are stored in the data stream so they are \


	fetched for readers.  \\\{we need some rule so that users can override 
these \


	values.  perhaps we can say that explicitly addressing an object by name \


	(or fully qualified name) will override its datastream value.\\\}

	\


	4) a parent in the data object tree.  the author of a document may \


	associate overriding resource values with a substrate.  all children that \


	match the specification would then get this value.

	\


	5) XUSERFILESEARCHPATH.  Each file and directory on this path is examined \


	for a resource meeting the request.  If the path element is a directory, \


	the file matchingthe widget class is examined.  Typical entries on the 
path \


	might be:

		personal application resource file

		personal global resource file

		project application resource file

		project global resource file

		site resource file  \


		widget specific resource file  ($ANDREWDIR/widgets/)

		source system resource file   ($ANDREWDIR/lib/global.resources)


environment: XUSERSEARCHPATH



Widgets


Button \



There are several widgets that are buttons underneath;  there appearance 
and behavior will differ.  \



Label or Button  rectangle twice font height

		netscape has an outer rectangle


		(darker greys used in constant buttons)

			top

				dark gray line   6f's  or 65's

				three bkgnd grey lines  c0's  or b2's

				two light grey lines   e7's  or e1's

				25 lines of bkgnd gray

				two lines of dark grey

				3 lines of bkgnd grey	\


				1 line of light grey

	when armed, dark and light grey interchange and \


	middle becomes a medium dark shade  (a3's)

\begindata{figure,6765704}
$origin 0 0
$printscale 1.000000 1.000000
#none 0
\begindata{figogrp,538579928}
\figattr{
}
$ 1 0 0 524 256
$endatt
\enddata{figogrp,538579928}
\begindata{figorect,6769424}
attrs: 8 1 10 beige 12 0 andy 0 8 0 5 0
$ 10 13
$ 502 208
$# 4 19 58 6 6
$# 8 2025 1669 -6 13
$endatt
\enddata{figorect,6769424}
\begindata{figoplin,538844072}
attrs: 8 1 10 cyan 12 0 andy 0 8 0 5 0
$$ 1 6 0 0
$ 0 255
$ 28 -26
$ 28 -238
$ 500 -238
$ 529 -255
$ 0 -255
$# 4 0 2042 0 0
$# 5 112 1838 0 0
$# 6 112 136 0 0
$# 7 1956 136 0 0
$# 8 2069 0 0 0
$# 9 0 0 0 0
$endatt
\enddata{figoplin,538844072}
\begindata{figoplin,538851528}
attrs: 8 1 10 blue 12 0 andy 0 8 0 5 0
$$ 1 6 0 0
$ 529 0
$ -29 17
$ -29 229
$ -501 229
$ -529 255
$ 0 255
$# 4 2069 0 0 0
$# 5 1956 136 0 0
$# 6 1956 1838 0 0
$# 7 112 1838 0 0
$# 8 0 2042 0 0
$# 9 2069 2042 0 0
$endatt
\enddata{figoplin,538851528}
\begindata{figotext,538888472}
\figattr{
shade:8
color:black
linewidth:1
rrectcorner:10
fontsize:10
fontfamily:andysans
textpos:0
arrowsize:8
arrowpos:0
arrow:5
linestyle:0
}
$ 262 -1692691506
$ 328 112
Help=000
$# 1 1024 1013 -208 -96
$# 3 1024 1013 208 104
$endatt
\enddata{figotext,538888472}
#end
\enddata{figure,6765704}
\view{figview,6765704,0,87,45}

CheckBox  square at lineheight  2,10,2

\begindata{figure,6774952}
$origin 0 0
$printscale 1.000000 1.000000
#none 0
\begindata{figogrp,6670712}
\figattr{
}
$ 1 0 0 428 281
$endatt
\enddata{figogrp,6670712}
\begindata{figorect,6900056}
attrs: 8 1 10 beige 12 0 andy 0 8 0 5 0
$ -7 -5
$ 421 280
$# 4 -6 16 -6 -7
$# 8 2041 2048 -12 -6
$endatt
\enddata{figorect,6900056}
\begindata{figoplin,6830752}
attrs: 8 1 10 cyan 10 0 andysans 0 8 0 5 0
$$ 1 6 0 0
$ 30 190
$ 0 -114
$ 112 -114
$ 102 -105
$ 10 -105
$ 10 -10
$# 4 145 1388 0 0
$# 5 145 555 0 0
$# 6 680 555 0 0
$# 7 632 624 0 0
$# 8 194 624 0 0
$# 9 194 1319 0 0
$endatt
\enddata{figoplin,6830752}
\begindata{figoplin,6882928}
attrs: 8 1 10 blue 10 0 andysans 0 8 0 5 0
$$ 1 6 0 0
$ 142 76
$ 0 114
$ -112 114
$ -102 104
$ -10 104
$ -10 9
$# 4 680 555 0 0
$# 5 680 1388 0 0
$# 6 145 1388 0 0
$# 7 194 1319 0 0
$# 8 632 1319 0 0
$# 9 632 624 0 0
$endatt
\enddata{figoplin,6882928}
\begindata{figorect,6795240}
attrs: 8 1 10 beige 10 0 andysans 0 8 0 5 0
$ 40 85
$ 92 95
$# 4 194 624 0 0
$# 8 632 1319 0 0
$endatt
\enddata{figorect,6795240}
\begindata{figotext,6674152}
\figattr{
shade:1
linewidth:1
rrectcorner:10
fontsize:10
fontstyle:0
fontfamily:andysans
textpos:0
arrowsize:8
arrowpos:0
arrow:5
linestyle:0
}
$ 287 688081922
$ 144 46
Tag=000
$# 1 1320 995 -104 -61
$# 3 1320 888 128 88
$endatt
\enddata{figotext,6674152}
#end
\enddata{figure,6774952}
\view{figview,6774952,1,72,49}

RadioButton   diamond at lineheight


\begindata{figure,6780824}
$origin 0 0
$printscale 1.000000 1.000000
#none 0
\begindata{figogrp,6682032}
\figattr{
}
$ 1 0 0 166 160
$endatt
\enddata{figogrp,6682032}
\begindata{figorect,6895368}
attrs: 8 1 10 beige 12 0 andy 0 8 0 5 0
$ 0 0
$ 171 159
$# 4 3 4 0 0
$# 8 2112 2041 0 0
$endatt
\enddata{figorect,6895368}
\begindata{figoplin,6892144}
attrs: 8 1 10 beige 12 0 andy 0 8 0 5 0
$$ 1 4 0 0
$ 86 27
$ 57 55
$ 0 110
$ -58 55
$# 4 1063 351 0 0
$# 5 1772 1053 0 0
$# 6 1063 1755 0 0
$# 7 354 1053 0 0
$endatt
\enddata{figoplin,6892144}
\begindata{figoplin,6895904}
attrs: 8 1 10 cyan 12 0 andy 0 8 0 5 0
$$ 1 6 0 0
$ 9 82
$ 77 -73
$ 153 0
$ 134 0
$ 77 -55
$ 19 0
$# 4 118 1053 0 0
$# 5 1063 117 0 0
$# 6 2008 1053 0 0
$# 7 1772 1053 0 0
$# 8 1063 351 0 0
$# 9 354 1053 0 0
$endatt
\enddata{figoplin,6895904}
\begindata{figoplin,6897144}
attrs: 8 1 10 blue 12 0 andy 0 8 0 5 0
$$ 1 6 0 0
$ 9 82
$ 77 73
$ 153 0
$ 134 0
$ 77 55
$ 19 0
$# 4 118 1053 0 0
$# 5 1063 1989 0 0
$# 6 2008 1053 0 0
$# 7 1772 1053 0 0
$# 8 1063 1755 0 0
$# 9 354 1053 0 0
$endatt
\enddata{figoplin,6897144}
#end
\enddata{figure,6780824}
\view{figview,6780824,2,31,30}

Indicator   circle at lineheight

\begindata{figure,6786272}
$origin 0 0
$printscale 1.000000 1.000000
#none 0
\begindata{figogrp,7184696}
\figattr{
}
$ 1 0 0 128 115
$endatt
\enddata{figogrp,7184696}
\begindata{figoell,7254384}
attrs: 8 1 10 beige 12 0 andy 0 8 0 5 0
$ 127 0
$ -126 114
$# 1 17 1031 0 0
$# 3 1027 17 0 0
$# 5 2037 1031 0 0
$# 7 1027 2045 0 0
$endatt
\enddata{figoell,7254384}
\begindata{figospli,7255048}
attrs: 8 1 10 cyan 12 0 andy 0 8 0 5 0
$$ 1 28 0 0
$ 24 93
$ -5 5
$ -8 1
$ -16 -9
$ -23 -26
$ -23 -42
$ -20 -56
$ -10 -72
$ 7 -85
$ 24 -92
$ 51 -92
$ 69 -87
$ 79 -81
$ 85 -75
$ 77 -70
$ 70 -75
$ 59 -80
$ 48 -83
$ 34 -83
$ 18 -78
$ 7 -70
$ -1 -63
$ -8 -53
$ -12 -39
$ -10 -19
$ -3 -10
$ 2 -3
$ 0 0
$# 0 24 32 0 0
$# 3 1753 1761 0 0
$endatt
\enddata{figospli,7255048}
\begindata{figospli,7257616}
attrs: 8 1 10 blue 12 0 andy 0 8 0 5 0
$$ 1 26 0 0
$ 108 18
$ -8 6
$ -2 12
$ 0 16
$ 4 24
$ 7 35
$ 7 45
$ 3 58
$ -6 72
$ -31 85
$ -55 86
$ -71 80
$ -78 76
$ -83 72
$ -88 80
$ -83 83
$ -77 87
$ -63 93
$ -40 95
$ -18 90
$ 1 78
$ 13 59
$ 18 44
$ 15 24
$ 10 12
$ 5 6
$# 0 325 327 0 0
$# 3 2031 2022 0 0
$endatt
\enddata{figospli,7257616}
#end
\enddata{figure,6786272}
\view{figview,6786272,3,22,20}




\flushright{\smaller{ATK version 8.0

January, 1996}}


\chapter{class AWidget Documentation}


\bold{	Derived from:  \italic{avarset,} \italic{dataobject,} 
\italic{observable, traced, ATK}

	Files: \typewriter{awidget.H, awidget.C}

	Can be instantiated.  Can be a base class.}


\smaller{\flushright{Documenter:  W.J.Hansen}}


An AWidget object provides the slots accessed by AWidgetView to display a 
widget.  \




\section{Members of class \underline{AWidget}}


Displaying image on screen


	ASlotInt desiredHeight;  // nominal screen height of the image

	ASlotInt desiredWidth;  // nominal screen width of the image

	ASlotFigure screenImage;  // (default to empty)

		// Figure to be displayed for the widget

	ASlotFunction fullUpdate;  // (default does nothing)

		// Called at full update time before fetching screenImage.

	ASlotFunction update;  // (default does nothing)

		// Called at update time before checking screenImage.


Handling mouse hits


	ASlotBoolean passMouse;  // (default FALSE)

		// If True, mouse hits are passed to viewer.


	// If passMouse is False, the appropriate one of the following

	// is fetch and executed if defined.   (default does nothing)

	ASlotFunction mouseLeftDown; \


	ASlotFunction mouseLeftMove;

	ASlotFunction mouseLeftUp;

	ASlotFunction mouseRightDown;

	ASlotFunction mouseRightMove;

	ASlotFunction mouseRightUp;


Printing


	// printImage - (String or Function) \


	//	either a string giving postscript for the image

	//	or a function that generates and returns that string

	ASlotInt nominalPrintHeight;  // nominal printed height of printImage

	ASlotInt nominalPrintWidth;  // nominal printed width of printImage

		// nominalPrint\{Height,Width\} can be set by the function that

		// computes printImage

		// The image will be scaled so it fits into whatever space has

		// actually been allocated on the page



\section{Overview of  AWidgetView}


An \italic{\bold{AwidgetView}} has as its dataobject an object of class 
\italic{\bold{AWidget}} or a class derived from \italic{\bold{AWidget}}. 
 For each view action the AWidgetView must take, it fetches slot values 
from its dataobject.  These slots are documented in awidget.doc.


View methods of class AWidgetView can be overriden in a derived class.  \




\section{Overrides}


AWidget overrides no methods of avarset.



check Rob's doc and embedder to get embedding right

implement Update, FullUpdate, Hit

implement proctable entries and methods


go through view methods and decide what to implement



add to 'figtoolview' a tool option "Baptize", which prompts for a name to 
be given to the selected object

when an object is selected, its name should be shown in the command line.

add a method to figure which finds the figobj corresponding to a name


-----


\chapter{Widget Resource Files}


\flushright{September, 1995}


In Andrew 7.3, widgets can be implemented by creating a resource file and 
supplying slot values that will be used by the generic driver, awidgetview. 
 This document describes the resource file and the slots  that are accessed 
by awidgetview.  \



Most slots are optional;  the only absolutely required slot is 
`screenImage,` although providing only this one will make a rather 
uninteresting widget.  \



Slot types are subclasses of ASlot.  The type name given here is prefixed 
with "ASlot" to produce the class name of the class implementing the type.



__________________________________________________________________

\section{Resource file format}


Eventually, resources will be specified with the usual X mechanisms. Always 
and initially, however, widget is described in a dedicated  resource file 
with a lower-case string which is the widget name followed by the extension 
".aw".  For instance, a simple button is described in "button.aw".  The .aw 
files and other associated widget description files are stored in 
directories named on a pathlist given by the CLASSPATH value and removing 
the suffix "atk" (if any) from each entry and adding "awidgets".


xxx  revise to use all entries on path in reverse order



The resource file is a sequence of lines which have one of three forms:


\paragraph{i) comments}

\leftindent{The first non-white-space characters on the line is an 
exclamation point.  The rest of the line may be an printable characters.}


\paragraph{ii) include}

\leftindent{The line has the form

	#include filename

where the '#' is the first character on the line.   The named file is 
located using the AWIDGETPATH value   and its resources are read into the 
widget.  (A full   pathname is NOT allowed in place of the filename because 
  this poses a security threat.)}


\paragraph{iii) resource line}

\leftindent{A resource line has the form

	slotname : value-represented-as-a-string

In general resource files, the colon may be preceded by a string of class 
or inset names separated by commas and colons.  This will eventually be 
allowed for describing particular widget instances, but the widget 
description resource file should   always have only a slotname.}


Upper- and lower-case letters are allowed in slotnames;  all but the first 
 character may also be digits.  Slotnames begin with a lowercase letter, 
 but otherwise words that are part of slotnames are capitalized.


The document below specifies slotnames expected by awidgetview.  A widget 
can utilize other named slots for purposes of its own;  however, to avoid 
future conflicts, the names for those slots should begin with the widget 
name. Example:  buttonCountPushes


There is one special slotname that appears in resource files: 
\bold{typelist.} The resource given by this name specifies the type for any 
resource specified in the file, but especially for private slotnames not 
known to  awidgetview.  The format of a typelist string value is:


	type slotname slotname slotname ... ; type slotname ...


That is, the word following the colon or a semicolon is a type name (that 
is, when the prefix "ASlot" is added it is the name of a class derived from 
ASlot.)  Following the type are a list of the slotnames in succeeding 
 lines which have the given type.


Resource lines are not restricted to a single physical line.  A  backslash 
followed by a newline is ignored when reading the resource  file.  (This 
differs from X resource files.)



__________________________________________________________________

\section{Slots that can be supplied to an awidgetview}


\bold{desiredHeight} - (Int) (defaults to 91)

\bold{desiredWidth} - (Int) (defaults to 91)

\leftindent{Nominal screen size of the image.  Actual dimensions are 
determined by parent view.}


\bold{printImage} - (String or Function) (default to screenImage)

\leftindent{If not supplied, the awidgetview prints the screen image.   If 
supplied as a string, that string is used as the PostScript,  but if 
supplied as a function, the function returns a string to be used as the 
PostScript.  The function can set  nominalPrintHeight and 
nominalPrintWidth. 	The function argument is the awidgetview.  \


}
\bold{nominalPrintHeight} - (Int) \


\bold{nominalPrintWidth} - (Int) (both default to actual screen size)	\


\leftindent{Nominal printed height and width of printImage.  The image will 
be scaled so it fits into whatever space has actually been allocated on the 
page.}


\bold{screenImage} - (Figure) (default to empty)

\leftindent{This figure is displayed as the widget contents.  Thereafter, 
if the image contents are changed, the image is as well.  At Update or 
FullUpdate time (and after calling fullUpdate, if supplied), the 
screenImage slot is checked and if it now refers to a new image, that image 
replaces the former screen image.

}
\bold{fullUpdate} - (Function) (default does nothing)

\leftindent{If supplied, this function is called whenever the screen image 
is  about to be completely replaced.  It may change the value of the 
screenImage slot.  Arguments supplied are the awidgetview, an integer 
giving the update type as an enum view_UpdateType, and four integers giving 
the left, top, width, and height of the update rectangle, in the child's 
coordinate space.}


\bold{update} - (Function) (default does nothing)

\leftindent{If this function is suppied, it is called when Update is 
called; that is, just prior to pausing to wait for user input.  The 
function may change the value of the screenImage slot, though it should 
usually just modify the figure which is the value of screenImage (by means 
yet to be implemented. The arguments supplied to the function are the 
awidgetview and the screenImage value used for the preceeding Update or 
FullUpdate.}


\bold{passMouse} - (Boolean) (default FALSE)

\leftindent{When a mouse action happens and this variable has been set 
TRUE, the mouse action is passed along to the figure driving screenImage. 
This variable must be set for textentry and other widgets that  need mouse 
interaction.  After the figure has handled the mouse, the event is sent to 
one of the mouseXxxxXxxx functions, if it is defined.

}
\bold{mouseLeftDown}, \bold{mouseLeftMove,} \bold{mouseLeftUp, 
\bold{mouseRightDown}, }\


\bold{mouseRightMove, mouseRightUp} - (Function) (default does nothing)

\leftindent{These functions, if supplied, are called when the appropriate 
mouse event occurs.  The arguments are the awidgetview, the x and y 
position relative to the top left corner of the widget, and a boolean value 
which is TRUE if the mouse hit is within the widget's screen space.}


\leftindent{Guarantees:  If both Down and Up are supplied, the Down will be 
called before the Up, and there will eventually be an Up for each Down.}


__________________________________________________________________

\section{Functions available to widget implementation functions}


These functions are made available as both methods and proctable functions, 
the latter are named awidgetview_xxx, though in .xxxinit files they are 
referred to with a dash instead of the underline.


\bold{Get}(char *slotName)  \{Method on awidgetview values\}

\bold{awidgetview_Get}(awidgetview *source, char *slotName)  \{proctable\}

\leftindent{Fetches the named slot from the given awidgetview. In C++code, 
the type is determined by context, though a cast may be necessary to get 
the expected type. In Ness code, types are tested by all operations, so if 
 the value is of the wrong type, an error occurs. \


\{\{\{Need a type test in Ness.)\}\} \


\{\{If the required type could be known at run time, and the ASlot holds a 
string, a converter could be called.\}\} }\



\bold{Set}(char *slotName, string)  \{Method on awidgetview values\}

\bold{Set}(char *slotName, ASlot* value)  \{Method on awidgetview values\}

\bold{awidgetview_Set}(awidgetview *target, char *slotName, string or 
ASlot* value)

\leftindent{Sets the named slot in the awidgetview to the given value.  If 
the value is an ASlot, the slot is set to that value's type.  If the value 
is a string and the slot has a type, the string is  converted to that type. 
 Otherwise, the string is stored as a  String.}


\bold{EnqueueUpdate}()  \{Method on awidgetview values\}

\bold{awidgetview_EnqueueUpdate}(awidgetview *updatee)  \{proctable\}

\leftindent{Either of these functions can be called by a hit function or 
other user interaction function.  Later, just before pausing for user 
input, the driver will call the update function for the argument widget and 
any other widget for which EnqueueUpdate has been called.  Update is called 
only once, no matter how many times EnqueueUpdate has been called since the 
last Update cycle.}


\bold{EnqueueFullUpdate}()  \{Method on awidgetview values\}

\bold{awidgetview_EnqueueFullUpdate}(awidgetview *updatee)  \{proctable\}

\leftindent{Either of these functions can be called by a hit function or 
other user interaction function.  Later, just before pausing for user 
input, instead of calling Update, the driver will call FullUpdate. 
FullUpdate is called only once, no matter how many times EnqueueFullUpdate 
has been called since the last Update cycle.}


\bold{WantNewSize}()  \{Method on awidgetview values\}

\bold{awidgetview_WantNewSize}(awidgetview *updatee)  \{proctable\}

\leftindent{When a widget wants to change its size, it changes 
DesiredHeight and DesiredWidth, and then calls this function.  \{\{\{Should 
we use an automatic update scheme instead?\}\}\}}


\bold{im::ForceUpdate}()  \{static member of im\}

\bold{im_ForceUpdate}()  \{Ness function\}

\leftindent{These functions cause an update cycle and cause any screen 
changes to be updated on the screen.}


\bold{Beep}(int volume)  \{method on an im\}

\bold{Beep}(int volume)  \{Ness function\}

\leftindent{Cause the work station to beep.  The beep may be at less than 
full strength by specifying a volume of less than 100.  The volume is 
treated as a percent of full volume.   (Warning:  A volume of zero will 
cause no beep.)

}
figobj *\bold{GetFigObj}(atom *figoname)  \{Method on figure values\}

figobj *\bold{awidgetview_GetFigObj}(figure *fig, atom *figoname) 
 \{proctable\}

figobj *AddFigObj(figobj *anchor, int offset, atom *type);

Adds a figobj to the figure at a position relative to anchor.  The 'offset' 
is an index of the positions between figobj's, with zero being the position 
immediately after 'anchor'.  Positive values skip that number forward and 
negative values skip backward.  Negative one is the position immediately 
preceding anchor.


\bold{SetFigAttr}(atom *figoname,  atom *figattr, T value)  \{Method on 
awidgetview values

\bold{awidgetview_SetFigAttrI}(awidgetview *updatee, atom *figoname,  atom 
*figattr, int V)  \{proctable\}

\bold{awidgetview_SetFigAttrS}(awidgetview *updatee, atom *figoname,  atom 
*figattr, string V)  \{proctable\}

\bold{awidgetview_SetFigAttrO}(awidgetview *updatee, atom *figoname,  atom 
*figattr, object V)  \{proctable\}




figobj.H

      struct point *pts  [int numpts]

      long x, y;

figattr.H

      long shade; \


      long linewidth;

      char *color;

      long rrectcorner;

      long fontsize;

      long fontstyle;

      char *fontfamily;

      long textpos;

      long arrowpos;

      long arrowsize;

      long arrow;

      long linestyle;

figorect.H

      long w, h;  /* may be negative */

figoins.H

      class dataobject *dobj;

figoplin.H

      struct point *pts  [int numpts]

      boolean closed;

figospli.H

	struct figospli_cubit \{double xa, xb, xc, xd, ya, yb, yc, yd;\};    \


      struct figospli_cubit *cubit  [int cubit_size]

figotext.H

      char *text;



\subsection{Existing methods and Ness functions}


\bold{DoKey}(long key)  \{method on an im\}

\leftindent{Simulates typing of key on window.}

\bold{DoKeySequence}(char *keys)  \{method on an im\}

\leftindent{Simulates typing the string. }\


\bold{DoKeys}(subseq)	\{Ness function\}

\leftindent{Pretends the user typed the keys.

There are also methods and Ness functions to enqueue answers to queries 
posed by characters in the string.}


\bold{DoMenu}(char *itemname)  \{method on an im\}

\bold{DoMenu}(subseq s)   \{Ness function\}

\leftindent{The menu item indicated by the string argument is fired as 
though selected by the user.}



__________________________________________________________________

\section{functions to make available soon}


Specify a set of keys and a function to be called when any key in that set 
is typed.


Means to modify the figure, (other than complete replacement).


Author mode (boolean).



__________________________________________________________________

\section{view methods to be handled by awidgetview

}
void ObservedChanged (class observable *changed, long value);

virtual void InsertViewRegion(class view *parent, class region *region);

virtual void InsertView(class view * parent, struct rectangle * 
enclosingRect);

virtual void InsertViewSize(class view * EnclosingView, long 
xOriginInParent, long yOriginInParent, long width, long height);

virtual void SetDataObject(class dataobject *dataobject);

virtual class view * GetApplicationLayer();

virtual void DeleteApplicationLayer(class view *applicationLayer);

virtual char * DescriptionObject(char * format, long rock);

virtual enum view_DescriberErrs Describe(char *format, FILE *file, long 
rock);

virtual void PostKeyState(class keystate *keystate);

virtual void PostMenus(class menulist *menulist);

virtual void PostCursor(struct rectangle *rec,class cursor *cursor);

virtual void RetractCursor(class cursor *cursor);

virtual void RetractViewCursors(class view *requestor);

virtual void PostDefaultHandler(char *handlerName, ATK  *handler);

virtual char * GetInterface(char *type);

/* View linking/unlinking functions. */

virtual void LinkTree(class view *parent);

virtual void UnlinkTree();

virtual boolean IsAncestor(class view *possibleAncestor);

virtual void UnlinkNotification(class view *unlinkedTree);

virtual void ExposeSelf(boolean recurse);

virtual void ExposeChild(class view *v);

virtual char * GetWindowManagerType();

virtual long GetDevice();

virtual void InitChildren();

virtual boolean CanView(char *TypeName);

virtual boolean RecSearch(struct SearchPattern *pat, boolean toplevel);

virtual boolean RecSrchResume(struct SearchPattern *pat);

	assumption:  awidgetview knows about all embedded views

	and it knows about all bases views (like figure)

	so it can handle RecSearch by dealing with the base views

virtual boolean RecSrchReplace(class dataobject *text, long pos, long len);

virtual void RecSrchExpose(const struct rectangle &logical, \


		struct rectangle &hit);

virtual void PrintPSDoc(FILE *outfile, long pagew, long pageh);

virtual void *GetPSPrintInterface(char *printtype);

virtual void DesiredPrintSize(long width, long height, enum view_DSpass 
pass, long *desiredwidth, long *desiredheight);

virtual void PrintPSRect(FILE *outfile, long logwidth, long logheight, 
struct rectangle *visrect);

	PS printing is handled by slots already described

virtual struct view_printoptlist *PrintOptions();

virtual long GetPrintOption(class atom *popt);

virtual void SetPrintOption(struct view_printopt *vopt, long value);

virtual void ReceiveInputFocus()

virtual void LoseInputFocus();

virtual void WantInputFocus(class view *requestor);

	InputFocus is only implemented for author mode

	the 'viewer' does not get InputFocus, but one of its children may

virtual boolean AcceptingFocus();

virtual void ChildLosingInputFocus();

virtual void ChildReceivingInputFocus();

virtual void Traverse(enum view_Traversal trav);

virtual void WantExposure(class view *requestor, struct rectangle 
*childrect);


__________________________________________________________________

\section{im functions to make available later}


    schedule and cancel events and InteractionEvents


    virtual int GetCharacter()  ;

    virtual boolean WasMeta()  ;	/* Was the last ESC returned by

				 * GetCharacter a real escape,

				 * or part of a meta sequence? */

    /* These routines are used to handle argument passing with keystrokes */

    virtual struct im_ArgState * GetArgState()  ;

    virtual void ClearArg();

    virtual boolean ArgProvided()  ;

    virtual long Argument()  ;

    virtual void ProvideArg(long arg);

    /* These routine manipulate late the global last command value. */

    virtual long GetLastCmd()  ;	/* get it */

    virtual void SetLastCmd(long cmd);	/* set it */

    static long AllocLastCmd()  ;	/* allocate a value for it */

    virtual long BumpArg(long digit)  ; /* shift one place and add in new 
digit */

    virtual void DisplayArg();	/* display it */

    virtual char * GetTitle()  ;


    virtual void SetWindowCursor(class cursor *cursor) ;

?   virtual void ClearCursors(class cursor * C);

    static void SetProcessCursor(class cursor *cursor) ;

    static class cursor * GetProcessCursor()  ;


    virtual FILE * FromCutBuffer()  ;

    virtual FILE * ToCutBuffer()  ;

	make available functions to put a string into CB,

	append a string, extract a string, and rotate CBs


    /* These are window manager interface calls */

    virtual void ExposeWindow();

    virtual void HideWindow();	/* iconified */

    virtual void VanishWindow();	/* totally invisible */


    virtual boolean CreateWindow(char *host)  ;

    virtual boolean CreateTransientWindow(class im *other, ... )  ;

    virtual boolean CreateOverrideWindow(class im *other, ...);

    virtual boolean CreateOffscreenWindow(class im *other, ...)  ;

	need functions for dialog boxes

    virtual char * GetKeyBinding(ATK  *object, struct proctable_Entry *pe, 
long rock)  ;   \


    virtual boolean RequestSelectionOwnership(class view *requestor)  ;

    virtual void GiveUpSelectionOwnership(class view *requestor);


    /* Drag/Drop support.  These calls return the host/files that

     * have been dropped on a view (when a hit mouse action is a

     * file drop.  They may only be called once.

	... */

    virtual char ** GetDroppedFiles()  ; /* NULL terminated array of 
strings */

    virtual void DropFile(char *pathname, class cursor *cursor);

    virtual void DropFiles(char **pathnames, class cursor *cursor);


    static boolean AddFileHandler(FILE *file,...); \


    static void RemoveFileHandler(FILE *file);

    static boolean AddCanOutHandler(FILE *file, im_filefptr proc, char 
*procdata, long priority)  ;

    static void RemoveCanOutHandler(FILE *file);

    static void AddZombieHandler(int pid, ...);

    static void RemoveZombieHandler(int pid);

    static void SignalHandler(long signalNumber, im_signalfptr proc, char 
*procdata);

    static long ChangeDirectory(char *dirName)  ;

    static char * GetDirectory(char *outputString)  ;


    static boolean IsPlaying()  ;

    static boolean CheckForInterrupt()  ;  /* scans ahead for control-G */

    static void ForceUpdate();

	already have flush graphics  (do not need both)


    static void SuspendRecording();

    static void RecordAnswer(char *answer);

    static boolean AnswerWasCancel()  ;

    static void RecordCancellation();

    static char * GetAnswer()  ;

    static void QueueAnswer(char *answer);

    static void ResumeRecording();

    static void QueueCancellation();




\begindata{bp,6816584}
Version 2
n 0
\enddata{bp,6816584}
\view{bpv,6816584,4,0,0}

Copyright 1995 Carnegie Mellon University.  All rights reserved.




\bold{\italic{Andrew}}

\bold{User Interface System}



\majorheading{\bold{Widget Author's Guide}}



\flushright{author: Fred Hansen

January, 1996}


\section{Author mode}


There are various sorts of levels of editing documents:


Read-only:  no editing

Form: fill in form fields, operate widgets

Text-edit: can modify text and insert/delete widgets

Author: can modify widgets or edit styles

Scripting:  all the above and edit scripts as well


The mode of a document is one of its attributes.  (But where do they come 
from ????   Depends on file name, file permissions, extension, directory, 
.... Most general approach is to allow a function in a file's script to 
determine the acceptable modes.)


If they have sufficient permissions on a file, users can switch mode by an 
item in the File menu.  (Ideally, moving the mouse to the item will pop up 
a sub-menu for the options.)  This item replaces the current Edit Styles 
and Add Template options.  Ideally, it also supplants File/PrinterSetup, 
Page/Open-CloseFootnotes, and Ness/...;  these can become buttons or menu 
options in the style editor.


Default mode is text-edit, as at present.


The first time a file is edited in text-edit mode or higher, the tools 
window is brought up.  (There is only one tool window for all Andrew 
windows.)  It offers editing options for the current object, depending on 
object type:

\begindata{table,6817832}
\cols 142 193
'Object type	'Tools window
-	-
'text	'style editor (extended)
figure	'figure toolset
raster	raster toolset
widget	attribute editor
\enddata{table,6817832}
\view{spread,6817832,5,0,0}




button

	shape: rectangle, small square, diamond, circle, triangle, no border

	contents: empty, text, figure drawing, inset

	visibility: visible, invisible

	behavior: click, toggle, radio, none, editable

	multiplicity: one, n columns, m rows, scattered


all attributes may be adjusted


there are some standard buttons that choose attributes

contents (default text) may be adjusted


PushButton, TextResponse, CheckButton, RadioButton, List, PopupList


PushButton: rectangle, click, text \


CheckButton: small square, on/off

RadioButton: diamond, radio

TextResponse [TEXTAREA]: rectangle, text, editable

	also offers the option of a menu of values

	also offers password mode (echo keys as *s)

	<TEXTAREA NAME="positive" ROWS=20 COLS=60>

		</TEXTAREA>

PopupList [SELECT]: text and click spot

List [SELECT] - choose from list of strings;  perhaps with scrollbar

			popup menu of options or onscreen scrolled menu of options

	<SELECT NAME="what-to-do">

	<OPTION>  Drink Coffee

	<OPTION SELECTED>    Read A Book

	<OPTION> Take A Walk

	</SELECT> <P>

		SIZE= makes it an onscreen menu

			scrollbar if more than size entries

	<SELECT NAME="what-to-do" SIZE=10>

	<OPTION VALUE="drink">Drink Coffee

	<OPTION VALUE="read" SELECTED>Read A Book

	<OPTION value=walk>Take A Walk

	<OPTION value="Pig Out">Munch Teddy Grahams

	</SELECT> <P><SELECT NAME="what-to-wear" MULTIPLE SIZE=8>

		MULTIPLE= allows multiple selections

	<OPTION>T-Shirt

	<OPTION SELECTED>Jeans

	<OPTION>Wool Sweater

	<OPTION SELECTED>Sweatshirt

	<OPTION SELECTED>Socks

	<OPTION>Cotton Sweater

	<OPTION>Rugby Shirt

	</SELECT>



DEFERRED


DrawnButton: rectangle, click, figure drawing

Label: rectangle, no action

Indicator: circle, no action, application changes contents

ArrowButton: rectangle, icon, click

ToggleButton: rectangle, text, on/off

SpinBox:  various ways to switch among a multitude of values

	click on arrows or popup a menu

Scalar

	Slider, TextNum, Thumb, Bar, Dial, Gauge

XYGraph

PieChart


border params:

	areas: topshad, botshad, contents

	borderwidth

	colors: border light, border dark, inside foregnd

		inside light bkgnd, inside dark bkgnd


initialImage ASlotString:  is converted and copied to screenImage



(really need some sort of enumerated data type

maybe strings that get converted on readin

but using strings will make widgets bigger

	but so what)


With the table that defines the types of attributes can be a table

	that lists string translations for each type

	(case insensitive, of course)

sigh: we need Rob's idea of atoms which can be constants for switch stmts




Button: basis of all button classes

	Int ButtonShape:

		0 - rectangle

		1 - none	(no border)

		2 - square

		3 - diamond

		4 - triangle

		5 - circle

	Int ButtonBorderThickness: default 2

	Int ButtonContentsType:

		0 - empty

		1 - text

		2 - icon

		3 - figure

		4 - inset

	String ButtonText

	Icon ButtonIcon

		(ASlotIcon is new.  Xbitmap or Xpixmap; inline or file)

	Figure ButtonFig

	DataObj ButtonInset

	Bool ButtonVisibility: T=visible

	Val ButtonValue:

		data stream or objectname

(ASlotVal is an avarset dataobject, with an integer and text fields

	this corresponds to value object in ADEW

objectname is two more tags:

	NAME <string> <another tag>

	OBJ <string>

when NAME is encountered, the value is entered in a symbol table

when OBJ appears, the value is retrieved from the table


sigh:  How does this integrate with giving names to widget instances?


	\



MouseHit:

	click: mouse down - arm color

		move - revert color when outside

		up - if up inside: pull trigger,  revert to original color

		if there is a ButtonValue, set it to 1 while armed,

			but revert to 0 on mouse up  ???

	toggle: down - arm color change

		move - revert color when outside

		up - if up inside: pull trigger,  leave changed color

		if there is a ButtonValue, set it to 0 or 1 depending

			on state.  (Is there a value change when armed?)

	radio: down - arm color change 	\


		move - revert color when outside

		up - if up inside: pull trigger,  leave changed color

			(some radio button sets require at least one

			to be selected)

			if turn on, turn off all others in same set

		Ptr ButtonRadioSet

			stored in local scope  (how???)

			during readin, the set is constructed

			only one button of the set can be on

	none: does nothing

	editable:

		mousehit goes through to object



DesiredSize:

	all but rectangle are controlled entirely by fontheight

	could just use bodyfont, or could use the local font height (how?)


	rectangle depends on content size



Defered operations for which slots are defined


highlighting

	\


	Boolean Highlit

	Int HighlightThickness

	Boolean HighlightOnEnter

	\



helpCB

traversal



for things like background color, fontsize, focus policy,

	fetch must traverse tree

	and value must not be cached



\bold{\italic{Andrew}}

\bold{User Interface System}



\majorheading{\bold{Widget Developer's Guide}}



\flushright{author: Fred Hansen

January, 1996}


How is Authoring mode established?


------------------------

Sizing


When interaction with a widget needs more space, it pops up a window as 
large as it likes.  An attribute specifies max rectangle.


size tied to Bodyfont size


even if author drags to change size, the size should change proportionally 
with body font

	how is this done?


Size computations are done in the desired size module

There is a default module which has stored values:

	authoring bodyfont

	authoring size

and inputs

	current bodyfont

	space available

and outputs

	size based on everything but space available


draws itself at its computed size, despite clipping


need a "relax" option that gives natural desired size \


	ignoring authoring size

	but heeding authoring and current body font


in author mode, \


	try to fit in assigned space

	if contents are text, \


		choose font size to fit assigned space

	but for desiredsize, give size based on either

		body font or the font explicitly chosen for the text

	if figure, assume it is resizable

	otherwise, clip



action functions in author mode

	DesiredSize

		gives size based on contents

		text font is default or set by author

			(default font is andysans-bold, \


				in bodyfont size)

	FullUpdate

		record assigned size (for FullUpdate or LastPartial)

action functions in User mode

	DesiredSize

		compute based on authoring bodyfont,

			authoring size, and current bodyfont

			(authoring bodyfont != text contents font)

	Full Update

		draw to size established by DesiredSize


-------------------


widget is the baseclass for all widgets


	it provides the borders


label displays text or object

pushbutton adds behaviors

toggle button puts two labels together

	a small icon and a regular label



borders:  outline  shadows  highlight


Outline is a solid color of a given thickness

	default black of 0 thickness


Shadows does the 3-d look in two colors and two pixel thickness



_____________________

borders


the widget class has attributes

	LitBorderList and DarkBorderList

Each is a string which gives the border widths and colors

on a per pixel basis from outside to in.

Each element of list is of form \


	color <whitespace> [width]

The list is space separated.

Example:

	black background 2 lightgray 2 white





_____________________




Date: Fri,  7 Jul 1995 13:26:06 -0400 (EDT)

From: Robert Andrew Ryan <rr2b+@andrew.cmu.edu>

X-Andrew-Message-Size: 1304+0

Content-Type: X-BE2; 12

If-Type-Unsupported: alter

To: Fred Hansen <wjh+@andrew.cmu.edu>

Subject: new methods


Get(resourceIdentifier), Put(resourceIdentifier, value)

		called from view

these are polymorphic in two ways:

	the resourceIdentifier may be a string or an atom

	the value may be one of several types:

		integer, real, boolean, char *, ...

		and arrays of same


I assume values are wrapped up in avalue objects?  Or what does Get(...) 
return?


PutForChildren(resourceIdentifier, value) \


	call by view

the resourceIdentifier can be a fuzzy match as in X, but the root of the 
string will be this data object.  this resourceIdentifier will be checked 
against each request from a child and the value supplied if there is a match


GetForChild(resourceIdentifier)

		called from child to retrieve the value

this implements the recursive search up the tree.


NotifyAttr(resourceIdentifier, value)

	called from parent (for editres protocol)


The rest of this confuses me greatly...  Is this applying to red, green or 
blue?  It isn't following what my conception of red was...  What's this 
about 'ForChildren'?  My notion was that setting a resource would find the 
object it applied to and tell that object to update its value for that 
resource.  More detail is needed on NotifyAttr and AttachChild.  (Though I 
assume AttachChild is the obvious, just registering the child so automated 
traversal works...)


-Rob




\begindata{bp,6819672}
Version 2
n 0
\enddata{bp,6819672}
\view{bpv,6819672,6,0,0}

PushButton


char *label = "Button"

activatecb - callback for click


hscale

vscale

borderwidth


\begindata{bp,6820192}
Version 2
n 0
\enddata{bp,6820192}
\view{bpv,6820192,7,0,0}

TextResponse \



int rows = 1

int cols = 20

int maxlen =    (-1 means infinite)

text *contents	the actual text displayed

activatecb - callback for RETURN


\begindata{bp,6820416}
Version 2
n 0
\enddata{bp,6820416}
\view{bpv,6820416,8,0,0}
Password

	display *'s for char's


int cols = 20

int maxlen = infinity

text *contents

activatecb - callback for RETURN


\begindata{bp,6820776}
Version 2
n 0
\enddata{bp,6820776}
\view{bpv,6820776,9,0,0}

CheckButton


boolean checked


\begindata{bp,6821000}
Version 2
n 0
\enddata{bp,6821000}
\view{bpv,6821000,10,0,0}

RadioButton


AWidget *sibling - ptr to next in ring of sibling radio buttons

boolean checked - TRUE if selected


\begindata{bp,6821456}
Version 2
n 0
\enddata{bp,6821456}
\view{bpv,6821456,11,0,0}

ImageMap


image

char *align

	top|middle|bottom

	left|right

	texttop|absmiddle|baseline|absbottom

width

height

border  (width in pixels)

vspace  (extra space above and below)

hspace   (extra space left and right)

activatecb - callback for mouse click

	in[0].Integer() is x

	in[1].Integer() is y


\begindata{bp,6821680}
Version 2
n 0
\enddata{bp,6821680}
\view{bpv,6821680,12,0,0}

List


int rows


boolean multiple


text *contents

\leftindent{Colon separated list of options.  An initial "*" in an element 
means it is initially selected.}


\begindata{bp,6822088}
Version 2
n 0
\enddata{bp,6822088}
\view{bpv,6822088,13,0,0}

PopupList


boolean multiple


text *contents

\leftindent{Colon separated list of options.  An initial "*" in an element 
means it is initially selected.}

\}


\enddata{text,6689888}
