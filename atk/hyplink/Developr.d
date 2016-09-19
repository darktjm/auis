\begindata{text,538700472}
\textdsversion{12}
\template{default}
\define{global
}
\define{footnote

attr:[Flags OverBar Int Set]
attr:[FontSize PreviousFontSize Point -2]}
\majorheading{Hypermedia Author Documentation}

\center{How to Make Linked Documents}


\quotation{\center{Abstract}

The Link and Plink insets provide a simple, fast way of building a set of 
cross-referenced hypermedia documents.  Using these insets requires no 
programming.

}
\heading{Why do I want to use Links?}

By using linked documents, you, the document developer, can tie together 
large amounts of information which can be conveniently navigated by users 
of your document (for a description of the user interface, see 
\begindata{link,538936152}
4
Help.d
0
0
R
\begindata{link,538936152}
Datastream version: 2
Help
2
andysans8b
black
white
\enddata{link,538936152}
\enddata{link,538936152}
\view{linkview,538936152,37,0,0}).  Links also give you new freedom in 
structuring your documents, because with links, you can break large pieces 
of information into smaller, more manageable chunks which cross-reference 
each other.


The target of a link is a filename which may contain \italic{environment 
variables} such as $HOME and $ANDREWDIR.  The value of the variable is used 
when the link is traversed, in order to construct the complete target 
filename.  Thus, for example, a link pointing to "$HOME/preferences" will 
always take the user to his or her \italic{own preferences file} because 
the value of $HOME is always the current user's home directory.


A plink has a filename target, which behaves the same as a link's target. 
In addition, the plink has a tag target, which refers to a particular point 
in the target file. You specify this point by putting a texttag inset in 
the target file, which contains the same label as the plink's tag. (See the 
"ez-refs" help document for information on creating texttag insets.)


It is perfectly legitimate to create a plink which refers to its own 
document. This allows you to create hyperlinks from one part of a document 
to another.


\heading{How do I insert a Link?}

\quotation{Note:  The following procedure assumes that you have customized 
your ".ezinit" file to allow for Autolinking.  If you haven't, please see 
\begindata{link,538937176}
4
Procs.d
0
0
R
\begindata{link,538937176}
Datastream version: 2
Procs
2
andysans8b
black
white
\enddata{link,538937176}
\enddata{link,538937176}
\view{linkview,538937176,38,0,0} before continuing.}

Links are inserted just like any other inset.  For instance, most ATK 
objects capable of holding insets (like text) use "Esc-Tab" to specify the 
inset.  To insert a link or plink in a text document, you would:

\indent{\description{- select an insertion point (where you want the Link 
inset to appear)

- press the "Esc" key and then the "Tab" key

- type "link" or "plink" and press the "Enter" key}}

At this point, you should see an empty Link inset (which has no target, and 
no label, displaying itself as 
"\
\begindata{link,538937944}
4

0
0
E
\begindata{link,538937944}
Datastream version: 2

2

black
white
\enddata{link,538937944}
\enddata{link,538937944}
\view{linkview,538937944,39,0,0}").  The easiest way to specify the target 
(and a label for the link) is to use the "Autolink" procedure.  To do this, 
with the Link inset selected:

\indent{\description{- press the menu button on your mouse and choose 
"Autolink" from the menu.

- move the mouse to the window of the file you want to link to, and bring 
up the menus again.  Choose "Autolink target here".}}

At this point, the Link inset will change its name to the filename of the 
target document, and it will be ready for use as a Link.  To test it, just 
click your left mouse button over the link inset, and you should be warped 
to the target document.  \



The link inset provides four additional menu options for setting the link 
destination: \


\sansserif{\leftindent{Link to File

Link to Relative File

Link to $\{...\}/...

Link to URL}}

These are described in the next section.


To create a plink, use the same procedures as for a link. Once you have 
specified the file target, re-select the plink inset and choose the Set Tag 
menu item. Enter a text string to be the plink's tag.


Now you must create a texttag inset in the target document. Use the same 
procedure as before: select an insertion point, press "Esc" and then "Tab", 
and type "texttag". A texttag will appear, displayed as 
"\footnote{\
\begindata{texttag,538938712}
\textdsversion{12}
tag-label\
\enddata{texttag,538938712}
\view{texttagv,538938712,40,0,0}}", with the insertion mark immediately 
after it. Type the same text string you entered for the plink; the text 
will appear after the tag inset, with a thin line over it. Now left-click 
the tag inset to close it. The plink inset in the original document will 
now function; to test it, click the left mouse button over it.


If you leave a plink's tag empty, or set the tag to a string which is not 
in any texttag in the target document, the plink will warp to the top of 
the target document, just as a link does.


You can insert as many links as you want into a document, and you can link 
to any other document (even system files.) You can set any number of 
plinks, in any number of documents, to refer to the same texttag in a 
particular target document. You may \italic{not} link a particular plink to 
\italic{more} than one texttag; if you try, the plink will warp only to the 
first texttag that matches the plink's label.


\heading{How do I change a Link?}

You can autolink again, but it won't change the label.  The reason for this 
is that you may want to set your label manually before autolinking, and the 
autolink procedure won't change an already defined label.  You can change 
the label manually using the Set Label menu item (on the Pushbutton card), 
which will prompt you for a new text string.  You can also change the font, 
using the Set Font menu item.  \



You can change a plink's tag using the Set Tag menu item. Remember that if 
you set the tag to the empty string, the plink will behave like a link and 
warp to the top of the target document.


See the "ez-refs" help document for information on examining and editing 
texttag insets.


Finally, you can use one of the following menu options for typing in the 
link string. \



\sansserif{\bold{Link to File}}


\leftindent{This option prompts for the name of a file to link to and uses 
the "completion" package to help you specify a filename.  If the link 
already has a value, it will be shown as the initial value.}

\sansserif{\bold{
Link to Relative File}}


\leftindent{This option appears to operate exactly the same as the Link to 
File option.  However, the link value stored is \italic{relative} to the 
current file.  For instance, if it is in the same directory, just the 
filename will be in the link.  Then if the linking file and linkee file are 
moved, but both in the same doirectory, the link will continue to work.}

\bold{\sansserif{
Link to $\{...\}/...}}


\leftindent{The string you type at the prompt is stored without 
interpretation and is evaluated when the link is clicked on.  File 
completion is not utilized.  You may include \italic{environment variables} 
as part of the filename, specified as $VARIABLE, $(VARIABLE), or 
$\{VARIABLE\}.  (Such variables may also appear in the values for Link to 
File and Link to Relative File, but they break file completion.)}


\sansserif{\bold{Link to URL}}


\leftindent{The string you type at the prompt is stored without 
interpretation and is evaluated as a URL when the link is clicked on.  File 
completion is not utilized.  You should not use $ variables in URL values.}



\heading{How do I remove a Link?}

You can remove a Link inset like any other inset:  just select text 
containing it and choose the cut option from the menu, or backspace over it 
(and confirm the dialog prompt).


\heading{Caveats}

\description{You shouldn't put Links (or any other inset or style) into 
system documents (like programs and init files).  This will cause the 
document to become unreadable by the system.

Don't change the label of a link to the empty string.  It will become 
invisible and very hard to select again.  You will probably need to 
backspace over it.}


\heading{How can I find out more?}

You can use the overview document 
\begindata{link,538940664}
4
Link.d
0
0
R
\begindata{link,538940664}
Datastream version: 2
Link
2
andy12b
black
white
\enddata{link,538940664}
\enddata{link,538940664}
\view{linkview,538940664,41,0,0} to find out more about Links.


\begindata{bp,538941160}
Version 2
n 0
\enddata{bp,538941160}
\view{bpv,538941160,42,0,0}
Copyright 1992 Carnegie Mellon University and IBM.  All rights reserved.
\enddata{text,538700472}
