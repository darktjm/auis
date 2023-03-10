\begindata{text,539682408}
\textdsversion{12}
\template{default}
\define{global
}
\define{notetotypesetter
}
\define{para
}
\define{wayup
menu:[Region,WayUp]
attr:[Script PreviousScriptMovement Point -15]}


This file illustrates using Ness primitives and ez proctable entries to 
process a file.  The exercise is to capitalize all words over 3 letters 
long.  \



\wayup{\
\begindata{cel,539790408}
\V 2
\begindata{value,539790552}
>7
\enddata{value,539790552}
10 539790552 1 0 0 0 
>OBJ< value
>VIEW< buttonV
>REF< Cap Ness
\begindata{text,539790728}
\textdsversion{12}
[string] <style> ()

[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (10)

[string] <bodyfont> ()

[string] <label> (Cap Ness)

\enddata{text,539790728}
\enddata{cel,539790408}
\view{celview,539790408,34,0,0}   }capitalizes using mostly Ness 
primitives

\wayup{\
\begindata{cel,539791704}
\V 2
\begindata{value,539791848}
>4
\enddata{value,539791848}
10 539791848 1 0 0 0 
>OBJ< value
>VIEW< buttonV
>REF< Cap EZ
>LINK< ze
\begindata{text,539792056}
\textdsversion{12}
[string] <style> ()

[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (10)

[string] <bodyfont> ()

[string] <label> (Cap EZ)

\enddata{text,539792056}
\enddata{cel,539791704}
\view{celview,539791704,35,0,0}   }capitalizes via ez proc-table routines

\wayup{\
\begindata{cel,539793016}
\V 2
\begindata{value,539793160}
>5
\enddata{value,539793160}
10 539793160 1 0 0 0 
>OBJ< value
>VIEW< buttonV
>REF< Lower
>LINK< d
\begindata{text,539793368}
\textdsversion{12}
[string] <style> ()

[string] <background-color> ()

[string] <foreground-color> ()

[long] <bodyfont-size> (10)

[string] <bodyfont> ()

[string] <label> (Lower)

\enddata{text,539793368}
\enddata{cel,539793016}
\view{celview,539793016,36,0,0}   }converts all words to lower case



\begindata{ness,539794184}
\origin{00\\10 Nov 1995 at 16:36:49 EST\\robr:  Robert Andrew Ryan\\00}
\template{default}
\define{fullwidth
menu:[Justify,Full Width]
attr:[LeftMargin LeftMargin Cm -25431]
attr:[RightMargin RightMargin Cm -27743]}
\define{sans
menu:[Font,Sans]
attr:[FontFamily AndySans Int 0]}
function \bold{main}()

	printline ("hello");

	replace (start(base(currentselection(defaulttext))), "\italic{Hi there!} 
\\n");

	setcurrentselection(defaulttext, next(currentselection(defaulttext)));

end function


extend "\bold{Cap Ness}" on mouse "any" \


	if mouseaction = mouseleftup then

		SaveState();

		SelectAll();

		captok();

		RestoreState();

	end if;

end mouse end extend


extend "\bold{Cap EZ}" on mouse "any" \


	if mouseaction = mouseleftup then

		SaveState();

		SelectAll();

		caplong();

		RestoreState();

	end if;

end mouse end extend


extend "\bold{Lower}" on mouse "any" \


	if mouseaction = mouseleftup then

		SaveState();

		SelectAll();

		lowerall();

		RestoreState();

	end if;

end mouse end extend

 \


function \bold{sel}() return currentselection(defaulttext); end function

function \bold{pnext}(where) print (where ~" \\"" ~ next(start(sel())) 
~"\\"\\n"); end function


marker SavedSelection;

 \


function \bold{SaveState}()\italic{

	}SavedSelection := sel();

end function;

function \bold{RestoreState}()

	setcurrentselection(defaulttext, SavedSelection);

end function;


function \bold{SelectAll}()

	setcurrentselection(defaulttext, base(sel()));

end function



void function \bold{caplong}()

	-- capitalize all words in currentselection over three letters

	-- in this version we use the textview proctable entries

	-- This version took several iterations to get right

	-- partly because of the curious definitions of forward and backward word

	-- and partly trying to figure out how to handle the end of the text

	marker endpreviousword;

	marker startthisword;

	marker endoriginalselection;


	--pnext("start");

	endoriginalselection := finish(sel());

	if sel() = "" then \


		pnext("empty");

		return;

	end if

	textview_beginning_of_line(defaulttext); -- start of selection

	endpreviousword := sel();

	--pnext ("here");

	while True do

		-- at this point, the selection is \


		--	a caret at the end of the last word processed

		--	and endpreviousword is the same

		textview_forward_word(defaulttext);     -- end of next word

		textview_backward_word(defaulttext);  -- start of next word

		startthisword := sel();

		if extent(startthisword, endpreviousword) /= "" then

			-- we have the same word as last time

			-- because we are at the end of text

			return;

		elif extent(startthisword, endoriginalselection) = "" then

			-- we are past the end of the original selection

			return;

		end if;

		textview_forward_word(defaulttext);	-- end of word

		endpreviousword := sel();

		if extent(next(next(next(next(startthisword)))),

				endpreviousword) /= "" then

			-- this word is more than three letters long

			textview_capitalize_word(defaulttext);

		end if;

	end while;

end function


-- change all characters in the base text to lowercase

function \bold{lowerall}()

	setcurrentselection(defaulttext, base(sel()));

	textview_lowercase_word(defaulttext);

end function


-- change all words to lower case

-- define words by using the token operator

function \bold{captok}() \


	marker m;

	marker endsel; \


	marker letters;

	letters := "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

			~ "qwertyuiopasdfghjklzxcvbnm";

	endsel := finish(sel());

	--printline("captok  " ~ textimage(length(allprevious(sel()))));

	m := token(start(sel()), letters);

	while m /= ""  and  extent(m, endsel) /= "" do \


		if extent(next(next(next(next(start(m))))), m) /= "" then

			-- word is three or more letters

			setcurrentselection(defaulttext, m);

			textview_capitalize_word(defaulttext);

		end if

		m := token(finish(m), letters);

	end while

end function


function \bold{init}()

	if isreadonly(sel()) then

		textview_toggle_read_only(defaulttext);

	end if;

end function

\enddata{ness,539794184}
\view{nessview,539794184,37,0,311}




\bold{\chapter{1.  The Andrew System:

 \


A Prototype University Computing System}}



\smaller{\italic{\center{James H. Morris, Mahadev Satyanarayanan, Michael 
H. Conner,

John H. Howard, David S. H. Rosenthal, F. Donelson Smith


Information Technology Center

Carnegie-Mellon University


January, 1986}}}



\section{1.1  Background}


The Information Technology Center was founded with the charter to develop 
software systems, based upon IBM hardware then under development, to 
support C-MU's program to integrate computing into all of its educational 
programs.  The total plan is aimed at nothing less than revolutionizing 
university education and will probably last many years and take many 
surprising turns.  Its ultimate success depends upon the energy and 
creative talents of the entire university, starting with the faculty some 
of who have already discovered novel ways to use computers in their 
research and teaching. \




\section{1.2  Technical Choices and Design Goals}


Many significant design choices exist within the framework of creating an 
environment for educational computing.  We have made a particular set of 
choices based upon our environment and expectations for technology. \



\subsection{1.2.1  Personal Computers vs. Timesharing}


The cost of interactive computing cycles is considerably lower when they 
are packaged in a personal computer (PC) .  The timesharing systems are 
being used extensively for text editing and document preparation. These two 
facts suggest that the entire community will be better served if everyone 
has a PC for day-to-day computing use and large machines are devoted to 
computing intensive activities. Centralized machines are still going to be 
around for a long time: for big computing jobs, for massive data bases, and 
to run useful software not converted to run on PCs. \



\subsection{1.2.2  Hardware: High Function, Ubiquitous Personal Computers}


C-MU chose to adopt a very powerful PC and to aim for deployment to all 
students while waiting for the price of such machines to come within an 
affordable range.  This choice was based upon our belief that personal 
computing is still in early region of the classical S-shaped curve where 
the benefits of increasing computing power far outweigh the costs.  There 
are two specific features of PCs we were willing to wait for: graphic 
displays and virtual memory. \



\paragraph{1.2.2.1  Bit-mapped, or all-points-addressable, displays} \


Bit-mapped, or all-points-addressable, displays are changing notions of how 
people can interact with computers.  The display is not merely a better 
interface to a computer --  it and the computer form an entirely new medium 
for expressing ideas. Very few people have learned to use that medium. We 
intend that C-MU be one of the first places to achieve a critical mass of 
such people. \




\section{1.6  The Window Manager}


The window manager mediates between the person sitting in front of the 
screen, called the user, and a program interacting with the screen, called 
a client.  The user of a PC running sees his screen divided into regions 
called windows.  Each has a headline, containing an identification of the 
program attached to it, and the machine the program is running on. As the 
mouse is moved, a cursor tracks it on the screen, changing shape as it 
moves from window to window. Characters typed on the keyboard appear in the 
window containing the cursor. \



When the mouse buttons are pressed various window-specific things happen; 
menus pop up, items are selected, objects move, and so on.  Menus normally 
pop up on the down stroke of a mouse button. They appear near the cursor, 
overlapping the windows under them. While the mouse button is down the menu 
item under the cursor is highlighted. On the up stroke of a mouse button, 
the menu is removed, and the highlighted action is performed by the client.


New windows appear on the screen when a client processes requests them. 
They cause existing windows to be re-sized or hidden; creating a window 
does not involve interaction with the user. In fact, users cannot create 
windows directly; they can only create processes that will create windows. \




\section{1.7  The Base Editor Toolkit}


A multi-font text-editing toolkit has been developed on top of the window 
manager.  Its major application is an interactive text editor that 
reformats on the fly, supports standard document preparation styles, and 
creates hardcopy on a laser printer.  It  supports two modes of viewing a 
document.  In the normal one, the text is reformated to fit whatever window 
is available to it and is entirely editable.  In the second mode (preview) 
the text is paginated and formated precisely as it will appear as hardcopy. 


Virtually all displayed text in the system is handled by this toolkit so 
that moving information between data-bases, printable documents, and 
conventional tty-oriented computers is straight-forward. The package 
provides the means for sub-dividing windows and creating permanently 
visible menus. \



The toolkit sits between the client program and the window manager, issuing 
graphics operations to keep screen images up-to-date, handling editing 
requests from the user and client, and informing each about the other's 
changes.  The user is informed of changes in the data object by observing 
changes in the image on the screen.  The client program is informed either 
by procedure calls or by inspection of the data structures. The toolkit 
takes care of all I/O, dispatching mouse hits and characters, performing 
full redraws and incremental updates of the image. \




\section{1.9  An Educational Application:  Graphs and Tracks}


An example of an educational application which exploits the possibilities 
of the Andrew system is the program gt (Graphs and Tracks).  Gt simulates a 
physics experiment, to help students learn the physics  of a rolling ball 
without forcing them to deal with a real apparatus.  In the simulated 
experiment a ball rolls along a sequence of five elevated tracks of 
differing slope.   As the ball rolls, a graph is plotted to show the 
student the changing position as a function of time.  An option lets the 
student switch the graph to show velocity versus time or acceleration 
versus time instead of position versus time.


Gt challenges the student by showing a graph of the ball's behavior for 
some secret arrangement of heights for the ramp ends.  The student then 
adjusts the simulated apparatus--by pressing with the mouse on the 
image--and rolls the ball.  Since the graph charted by the ball on the 
student's tracks is plotted on the same image with the given graph, the 
student can adjust the tracks repeatedly to find the correct solution.



\section{1.10  Conclusion}


The software described above is still under development.  There will be 
significant enhancements to its functionality and performance over the next 
few years.  It is crucial, however, to get it into the hands of users now 
because they need to get an idea of what can be done with this new 
technology.  More important, we want to base our continued development upon 
the evolving needs of the applications developers. \




-- 
\begindata{bp,539817304}
Version 2
n 0
\enddata{bp,539817304}
\view{bpv,539817304,38,0,0}

-- Copyright 1992 Carnegie Mellon University and IBM.  All rights reserved.
\enddata{text,539682408}
