\begindata{text,538786488}
\textdsversion{12}
\template{default}
\define{global
}
\majorheading{Link Preferences

}\center{Changing the Link Appearance

}
\quotation{\center{Abstract}

You can select one of two "look-and-feels" for a Link inset.  You can also 
select whether they bring up another window.

}
environ::GetProfileSwitch("LinkMenus", TRUE),

environ::GetProfileSwitch("LinkReuseWindow", TRUE))


In addition to the preferences listed here, you can adjust the appearance 
of links by adjusting the preferences for pushbuttons.  See the help file 
for pushbutton.


\heading{LinkStyle

}The \typewriter{linkstyle} preference controls whether the link is a 
pushbutton or in-line in text.  The value should be 0 for regular, in-line 
text and 2, the default, to use pushbuttons.


You can change link styles on a global or per-application basis only.  You 
cannot, for instance, change the way links appear on a document or per-link 
basis.


To make a global change, put the following line in your preferences file:

\example{*.linkstyle: \italic{desired-style}

}where \italic{desired-style} is either 0 or 2.


To make a per-application change, put a line of the form:

\example{\italic{application}.linkstyle: \italic{desired-style}

}where \italic{application} is the name of the application (ez, table, 
\italic{etc}.) and \italic{desired-style} is again either 0 or 2.


\heading{LinkReuseWindow}

The \typewriter{LinkReuseWindow} preference, a boolean value, controls what 
happens if the target file of a link is not already in a window on the 
screen.  If the value is off, FALSE, or 0, a new window is created for the 
referenced file.  Otherwise, the window containing the link is reused to 
display the target file.  The switch buffer command ^X^O can be used to 
return to the previous window contents.


\heading{LinkMenus}

If the system has been compiled with the ATTBL_ENV option, there is a 
boolean preference \typewriter{LinkMenus}.  When set to TRUE, on, or 1, It 
causes the menu item \sansserif{\bold{Autolink target here }}to appear in 
the Inset menu card.  When this item is selected after selecting 
\sansserif{\bold{StartAutolink}} on a link, the current file is recorded as 
the target of that link.


\heading{Where can I find out more?}

The overview document for Link is 
\begindata{link,538983528}
4
Link.d
0
0
R
\begindata{link,538983528}
Datastream version: 2
Link
2
andy12b
black
white
\enddata{link,538983528}
\enddata{link,538983528}
\view{linkview,538983528,44,0,0}.


\begindata{bp,538983816}
Version 2
n 0
\enddata{bp,538983816}
\view{bpv,538983816,45,0,0}
Copyright 1992, 1996 Carnegie Mellon University and IBM.  All rights 
reserved.

\smaller{\smaller{$Disclaimer: 

Permission to use, copy, modify, and distribute this software and its 

documentation for any purpose and without fee is hereby granted, provided 

that the above copyright notice appear in all copies and that both that 

copyright notice and this permission notice appear in supporting 

documentation, and that the name of IBM not be used in advertising or 

publicity pertaining to distribution of the software without specific, 

written prior permission. 

                        

THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 

TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 

MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 

HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 

DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 

DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 

OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 

WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.



 $

}}\enddata{text,538786488}
