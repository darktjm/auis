\begindata{text,538637352}
\textdsversion{12}
\template{help}
\define{global
}
\define{symbol

attr:[FontFamily symbol Int 0]}
\define{symbola

attr:[FontFamily symbola Int 0]}
\define{sansserif
menu:[Font~1,Sans Serif~42]
attr:[FontFamily AndySans Int 0]}
\define{strikethrough
menu:[Font~1,Strike Through~42]
attr:[Flags StrikeThrough Int Set]}
\define{changebar
menu:[Font~1,Change Bar~43]
attr:[Flags ChangeBar Int Set]}
\define{caption
menu:[Title~3,Caption~30]
attr:[Justification Centered Point 0]
attr:[Flags KeepPriorNL Int Set]
attr:[Flags KeepNextNL Int Set]
attr:[FontFace Bold Int Set]}
\define{verbatim
menu:[Region~4,Verbatim~15]
attr:[Justification LeftJustified Point 0]
attr:[Flags NoFill Int Set]
attr:[Flags KeepPriorNL Int Set]
attr:[Flags KeepNextNL Int Set]
attr:[FontFace FixedFace Int Set]
attr:[FontFamily AndyType Int 0]}
\define{index
menu:[Title~3,Index~91]
attr:[FontFace Italic Int Set]}
\define{indexi
menu:[Title~3,InvisibleIndex~92]
attr:[Script PreviousScriptMovement Point -2]
attr:[FontFace Italic Int Set]}
\majorheading{Link Procedure Table Entries}

\center{Link and your .ezinit file}


\quotation{\center{Abstract}

Link defines several proctable entries.  Only one, linkview-set-target, is 
intended for use in your .ezinit file.  The rest are used by Link itself 
(and posted on its own menu), but you could bind keys to them, or use them 
in compound procedures.

}
\heading{Modifying your .ezinit file}

You will probably want to modify your .ezinit file if you want to develop 
Linked documents, so that autolinking will work.  You don't need to do 
this, as you can make links without autolink, but this will make 
development easier.


Basically, in order for Autolinking to work properly, you will need a way 
of executing the "linkview-set-target" procedure in the target frame 
(window).  One way to do this, is to add the following line to your 
~/.ezinit file (please read the help document on \helptopic{initfiles} 
before attempting this operation):

\example{addmenu linkview-set-target "Link,Autolink target here" frame

}
\heading{Proctable Entries}

These are the three proctable entries defined by the Link inset.


\bold{Name:} \typewriter{\smaller{ linkview-set-target}}

\bold{Type of object}:\typewriter{\smaller{ frame}}

\bold{Documentation string}: \typewriter{\smaller{Execute this proc from 
the frame of the the buffer for the target file of a link.  To be called 
after linkview-autolink.}}

\bold{Comments:}  Usually bound to a menu item reading something like 
"Autolink target here".  Execution of this procedure after starting an 
Autolink with "linkview-autolink" (see next proc) completes the autolink 
procedure by indicating to the source link that this frame contains a 
buffer on a file which is to be the target.


\bold{Name}: \typewriter{\smaller{linkview-autolink}}

\bold{Type of object}: \typewriter{\smaller{linkview}}

\bold{Documentation string}: \typewriter{\smaller{Starts the autolink 
process.  Waits for linkview-set-target to be invoked, which tells this 
link what file to link to.}}

\bold{Comments}:  This item will be bound to the Link card's "Autolink" 
item.  Selecting this item will start the autolinking procedure.  You will 
need to invoke \typewriter{\smaller{linkview-set-target}} in order to 
complete the autolink procedure, which is preferably bound to "Autolink 
target here" on the Link menu card.


\bold{Name}: \typewriter{\smaller{linkview-set-link}}

\bold{Type of object}:\typewriter{\smaller{ linkview}}

\bold{Documentation string}: \typewriter{\smaller{Prompts for user to set 
target filename of the link button.}}

\bold{Comments}:  The Link inset binds this proc entry to the Link card's 
"Set Link" entry.


In addition, the Plink inset defines the following entry:


\bold{Name}: \typewriter{\smaller{plinkview-set-tag}}

\bold{Type of object}:\typewriter{\smaller{ plinkview}}

\bold{Documentation string}: \smaller{\typewriter{Prompts for user to set 
search tag of the link button.}}

\bold{Comments}:  The Plink inset binds this proc entry to the Link card's 
"Set Tag" entry.



\heading{Where can I find out more?}

The overview document for Link is 
\begindata{link,538773672}
4
Link.d
0
0
R
\begindata{link,538773672}
Datastream version: 2
Link
2
andy12b
black
white
\enddata{link,538773672}
\enddata{link,538773672}
\view{linkview,538773672,22,0,0}.


\begindata{bp,538774248}
Version 2
n 0
\enddata{bp,538774248}
\view{bpv,538774248,23,0,0}
Copyright 1992 Carnegie Mellon University and IBM.  All rights reserved.

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

}}\enddata{text,538637352}
