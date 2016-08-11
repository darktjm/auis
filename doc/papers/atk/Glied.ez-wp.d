\begindata{text,19077980}
\textdsversion{12}
\template{default}
\majorheading{EZ -  As a Word Processor


\smaller{\smaller{\smaller{As published in the \italic{Linux Journal}, 
August, 1994}}}}


In this article, Terry Gliedt introduces us to the Andrew project, and 
gives us a taste of one aspect of it; the EZ editor, which can edit text 
and graphics, and can be used as a word processor.


\heading{Introduction}


AUIS had its roots in 1982 when Carnegie Mellon University and the IBM 
Corporation decided to jointly develop a campus computing facility based on 
personal computers to replace the time-sharing system then on campus.  IBM 
provided not only generous funding, but also some talented individuals and 
access to IBM development programs.


The result was a graphical user interface we know as the Andrew User 
Interface System and a file system, the \italic{Andrew File System}.  The 
file system formed the basis of Transarc Corporation's \italic{Distributed 
File System} (DFS) and is offered as part of the Open System Foundation 
software.


The \italic{\bold{Andrew Consortium}}, composed of a number of corporations 
and universities, funds the current development of AUIS.  AUIS is available 
on a wide variety of platforms including Linux, AIX, Solaris, Ultrix, HP UX 
as well as others. \



In June, version 6.3 of the Andrew User Interface System (AUIS) was 
released by the Andrew Consortium.  This release provided support for Linux 
and shortly thereafter, the package \bold{auis63L0-wp.tgz} was made 
available at \italic{sunsite.unc.edu} in \italic{/pub/Linux/X11/andrew}. 
 This particular package contains just a small portion of AUIS that is 
suitable as a word processor.  This article will describe some of the word 
processing aspects of AUIS.  Future articles in \italic{Linux Journal} will 
describe other pieces of AUIS.


\heading{EZ is Easy}


\italic{ez} is the simplest, most general AUIS application possible.  It 
loads a document (or creates a new one) and displays it in a window. 
 That's all.  Everything else that happens is controlled \underline{by the 
document itself}.  For example, if you are editing a text document, you get 
all the text-editing commands.  If you're editing a picture, you get 
picture-editing commands.  If you're editing a text document with pictures, 
you get both, depending on which piece you are working on at the time.  The 
application, \italic{ez}, doesn't care (as if any software really cares).


\heading{WYSLRN}


At an initial glance, the document you see displayed by \italic{ez} appears 
to be What You See Is What You Get (WYSIWIG).  Upon further investigation, 
however, you'll see that it really is not. While the document appears with 
various fonts and pictures as you might see on paper, the application does 
not enforce any concept of a "page".  If you make the screen extra wide or 
narrow, the text will re-flow to fit the window, not some notation of a 
page.  Documents are printed in PostScript(tm) (well, not precisely, but 
that is what comes out in the end), and of course the X fonts do not quite 
match those used when printing.  So \italic{ez} might better be said to be 
a WYSIWIG-almost editor.  One pundit calls this "WYSLRN" (What You See 
Looks Really Neat).


\heading{The Basic Layout}


In Figure 1. you can see a sample of a tiny document.  The application and 
name of the file appear in the title bar.  A small message area appears at 
the bottom of the document.  Notice there is a menubar for pull-down menus 
at the top of the document.  You can select these menus as you'd normally 
expect.  As a shortcut AUIS applications will pop-up the menus when you 
press the middle mouse button (or both buttons simultaneously for a 
two-button mouse).


\begindata{image,22335456}
format: gif
R0lGODdhxAAIAfAAAAAAAP///ywAAAAAxAAIAQAC/oSPqcvtD6OctNqLQQi5+w+GxjaRElei
4oOqo7ZsLjzOK81mZrRDttO7KTjERDE2fOF+OqaxmULIpqYZdVoTNlpPXPegcoKOOUwQ6Ppx
pUiN2FfFlpRJNj0LroJhO5mdIXdxFoh3t+eFqHQ2d/f2xZSW1xO2R6lHafjnuGWzlvdnZ9W2
eBK4eZhJJ/pY2SpluoVqMWg4OWp0FadTx5i69IVq6SpLrAmFG2Rbd1o4O3wCTONZnObXQqZK
jJkYVQp6yOzW4edGXiapm2lFK45dc01uTlhBGhO5TFGvtR/rK3ijD9erXP7s8TuYIhxCg96g
dVsIMaKQgK8Y5bqIMaPG/o0cO3r8CDKkyJFU3pE8iTKlypUsUUp8CTOmzJk0a9oM1TKnzp08
e2Y06TOo0KFEf5YrijSp0pxAlzp9ClVj06hUqzqdajWrVp9Yt3r9mrIr2LFkO4otizatnJ1J
SKIDedRamI18zJYke3ZkXCIiy+21e6Uu35+D6fr9mxVrM49H6+bFElhcFr6T37WDbLkdYoKH
+xSJo3TqZRKMW1H++Im0tclHVB+WTFpzYamfBUs7HfrvwM0EMVeDW62S6uGjV682bZeP7cED
kYq+i9o374t+C8sezty18di+AV//jD13b/DRjRenzR38cvLrW6snT1d79eqziSqW/Bi74+mR
/jvraTyaYHPddo2A6F2WWjza1ddSfmqF9aBODkboFoUN8mdhhlVNqGGHSXHoYYhCgShiiWxh
aGKKPJGoYosnsehijHDdRGONNRInV3Y47qhjj/T5mOOPQgZJJI9DGlkkkEguyZxlSR6pZJRQ
TvlklUxSeaWVUmr5WoBcfpllmFuKieWYZpaJ5nGSYGljm25CweWbcs7JQ5gwyuhhRQ6hiKeK
AilXGx59DtofbIAqtwShiq7zQm0D8rlonp8cGuhckfb5pxqTXjqonk54wimmnpqU6J2hStWW
fZuio8hbp0oImaqr8gLaq2xhBiCkHI3qqDSu2soSgAsy6BavhwoK/uytRf6okrG9JposU7FS
1lxYesJharTj7ahrac2co620y+bY7G5WZhvutD6iSwhFhaQLIVRdoForvCH9WpSnalDbrb3J
XIUTQ/X6u+sQS+lLqmvsAhvMwQE3lg7BGiIMMb9P4bviwlvNuy+yDvdbLMhoUSyQZx87p7FW
HGPk8YcpoyryyJuyjB/AMd/7cmKhYHtzyNT13F9QORcs6LGTAt3X0D8jTbPNrRrdcr73rRFx
1KaYAxvVD1stddGBHn0yZ5gU6CtvaxqKdjazXrwz1Fm7rBuBu9V89aNYY/02qRgP5SwiJqMc
N9mCD3x21opQXTVOTpf9NddCT71YOnuB/g3P3JXjh7a8qzZON+D4ZC44UICEDnro/0U1b6N+
w9P155K715TBk/Qhd9q/OX0JopRLTGHqbg/Mu8xe6/5u8BH6zjnwxo/VN6NKLy9h28mzDj1e
mxPfefXWg6O689oLb+j0exP2Pd/SY698+VY1/yfIb6vP1PnupE/befCXO/z8lgJH/f34Py0+
96mLbRfbFVeup7/xsQw3qGPaig7Ivd/1zzs9A2AjZlet9ORqLa+522n0Fp745W91ChzRLwx2
OMe1ilu6IBduWPOdE0UwgFQpGrJE975x3YVZ5EJQd+ATvRE6z4GcMVvpUKgv6AQoQU1K2LBI
CEELJpCIC/zZ/unqdUPdLGg+YHpifajYLi4EEIwl6YraFJTC8MnFh1v0kn5YGEIZSpGEZFyj
URA3u7FRj42WECPtRGEkJ/WEfdcSWhwjU8OJyY+OJiSW40bkwgcRcneae5mbHjdD9E3Qfw0U
Yvs4ubFFeg+UOvNkIUm5PlFmCpUbQiAd68hK1KgyibFkmykpWcuPzXGUucSdGDVZwl5mLJNT
FKYvu/dKY+qSccCEpTKXFr5mOvOZhWJmMfVCJzTIaJLF64uFplnJXa7Sm70T1S27ea9vmlOc
tOTf8daZuymKzGxfAact2Tkzcm7MkfD8pTwrpCy9AJFYJkJeMwFqHwPaz57HpNQn/gVqrWAN
lKAlMug19fkiplFqgC3iZvbcuS1MxuY8DF2m1i6azpS+CJFsCEw/kTlEhLZSHtDZ5izzqdIN
cfSQBXVlTCG6vp1SVEQWZSRQg2rGlzr0lBgNqFnEUtLcENOoTU0kPfHk0b+BlF7lok6PIumn
mz4yPk/laVl7c1WbnvOj5alpaCgaVbjhc6zosRYygJPWGGV1f1vFWdx0ZSC3uqiovMzpjArG
GLgq9Vm4bGtf61o/8ql1rjl0rEtDmtj45HWwPn1oVcn62R96FatirWxmT3tUl04nrp6LZzJT
C1mZ7jC2HS0t/WgrWUQaULerXawEg4nZ3Op2tz3EbYoI/jtOwwoXVxiSXV5Y2zrX/jS021qT
H5ebH+jKirK3Xe5ow7gWmioMUto131q1atkqztZo65goaPVq200S95B/fdYg5Gvcnp6Xr+n9
rj/LuAiz5rei8YURijqomcKVzDCXnax0Pavc0bZUdpAYKuHWaU2qRpi3LfWqQWRb2/0Ct8FN
80J64vFhEId1qoV9rBJfF42KrJS0IjZwXedTMjTErLwmZHFyXWzH/dDDwgMmameZuuHtfKsU
BG0uhk+q4ceyUXX5EPB8OVvjeZLvOwkBGo8xyV38Dvg6mQFCo5T75Sg+GMlAdhQDxaCgeE02
wy3uL3eKozcndve9WI7mP2Fr/h4GsapieOtWmgdZYC172H7zYJV6AeNgf76Wuo5p6lsA++QV
RtnOhBaqYINb5BDtdcSf5hGg4wxp+Pq4nZwGa19JbWVJZVnFbwapnh/taQKvGqcuhjXMduzb
MdKavM7JtK9Q2mawHHqYfp50kv17FWNLmtWoVcuy5bjmxlY7LdcOYphtXE4aN3u6z1a2tJFJ
7VRbO9jSDFk27TFnKNeZmmqWNLnpDWY6/xjfiJ41v3v87W5zctTP+3cyND1vg1/I3wpntr7T
3fD/ZRudEV94wCuObXtDGOMrQS7EOY5NhoP8fw/n9cgzKvKTo/ziKrfruDfe8qTtmq4xN0zK
tXfX/nCWTccC57Z/Opm20fn6X7Ikslcy00pw3FHMuTW00a2XdNPGauiz6eBI1VTGzlluz8E6
9MqsyHRcu5E1YCW02Tl6NjcjvWmppCtbqz12F2rRji92tXpuQ2awt70g+4Y7HKsu4UjiuYjZ
6dLtwrt3MoGb7gyMC2LcOHW0n7g9T997msLu3yPxcDOim9bPg3wJ+dD32tfyAdVzbt3wJDXr
jj8cB/8LGjOSnn12M7chj9vZ1lAdksMMsSdrrzIRcGpUgFBFzd0d9IYV3ODEhwW0ju9Xr6GY
5tCH5n9usfx/Z2qpUq/+rx+s++zze/uM5br3w9iL88/oaCzAvPoxGKee/t8PGWySP/xkfCXx
4/vd/O+///8PgAEogAM4Bu9nPgZ4gAj4OAq4gAyIaIJVMYZnfRIFbU2WdXXDX+jFealXd7HG
YNvxRm82dx7oEiQmXjpCHzz0VYoxda0Xgi4BMZ3GRxUoXnIDep2RYOj1gnEXQkMyggAWXp1W
LFrSeJz3XYbDLVOGhBb4JHlWXSAoVMsxeBHFJLX2HE+IgozHg7U2QPjXeEU0g64mhS+YUXhT
XIJmgjr0Qmq4hWnoRV+lWmUXeDPoZSQVglMYhVoIgimoh0x4WXxoVonghh80LJXHdo53h2R4
hinoJZrXQoXoVoD4hTWFiCEFeXhYhoCXRxIIP4Z1QyA7iGJJCG2DtkFBEjs0I0jrcmuHpVOg
RoP50opJE1SD6HkFlEhDGIt2d4EDZ38Kp3/q94vnR4DDyH8FAAA7
\enddata{image, 22335456}
\view{imagev,22335456,40,0,0}
Figure 1.  A Sample \italic{ez} Window


\begindata{image,19892800}
format: gif
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
\enddata{image, 19892800}
\view{imagev,19892800,41,0,0}
Figure 2.  Sample Split Window


The scroll bar looks rather Motif(tm)-like, but behaves differently.  You 
can scroll forward by pressing the left mouse button, as usual, but unlike 
some scrollbars, you can scroll back by pressing the right mouse button. In 
larger documents you can "grab" the "box" in the scroll bar to quickly 
scroll through the document.


Most commonly used functions have been assigned to a sequence of keys. 
 Many of the common Emacs keybindings apply to \italic{ez} (e.g. ^x^c). 
 Several function keys have been assigned useful bindings.  F1-3 will 
invoke the Copy, Cut and Paste functions.  AUIS applications will remember 
the past few Copy/Cut buffers and F4 will allow you to Paste them in your 
document and then cycle through them.  This allows you to Paste two or 
three objects by simply pressing F4 a few times without needing to repeat 
the select-copy-paste sequences.  I use this feature all the time and feel 
crippled when I work with other systems that do not have this capability.


F5 will \italic{italicize} the selected text and F6 will make it 
\bold{bold}.  F7 (Plainer) will remove the last style applied against an 
area.  For instance, if you press F5, F6 and then F7, your text will be 
left in \italic{italics} because the Plainer function removed the 
"\bold{bold}" style.  F8 (Plainest) will remove all nested styles applied 
to the area.


AUIS applications are highly tailorable.  Almost everything described thus 
far can be made to behave differently.  The look and feel I describe here 
reflects the auis63L0-wp distribution.  A future article will describe the 
various configuration files and settings and how you can control these.


This distribution of AUIS has over 800KB of help text (and this is about 
one-third of the total).  There is extensive help information on how to use 
all the aspects of the system, including its configuration and 
personalization files.  After installing AUIS, probably the first command a 
novice would enter is \italic{auishelp}.


In Figure 2. we can see there is a second window.  You can, of course, edit 
more than one file at a time.  You can also have two windows on the same 
data, as shown here.  If you enter data or make changes in either window, 
the other is immediately updated (if the changes are in a part of the 
document that is visible).


\heading{Checkpointing}


If you work for a few minutes without saving, \italic{ez} will 
automatically save for you.  You can tell this is happening when the 
message line at the bottom suddenly shows "\italic{Checkpointing...}" and 
then "\italic{Checkpointed}".  The file is written to a separate file named 
like the original filename except ".CKP" is appended.  Thus if you edit 
\bold{test.d}, the checkpoint file will be \bold{test.d.CKP}.  When you use 
the "Save" command, your original file is replaced with the new version and 
the checkpoint file is removed.


\heading{EZ Magic}


It is important to understand that the word processing files created by 
\italic{ez} have their own distinctive format, just like your favorite 
commercial word processors.  This format was conveniently designed from the 
start to allow \italic{ez} documents to be sent via conventional electronic 
mail systems.


This format is the "magic" that allows \italic{ez} to be so simple.  The 
data quite literally defines \underline{what} can be done to it.  As the 
data is read, programs that match the data are automatically loaded for 
you.  \italic{Ez} is not a single program, but a group of programs that can 
cooperate with each other, each of which knows how to manipulate a certain 
type of \italic{object}.  If \italic{ez} reads document with text in it, 
the text-editing program edits the text \italic{object}.  If \italic{ez} 
reads a document with a bitmap (picture), the bitmap-editing program edits 
the bitmap \italic{object}.  If your document has several different 
\italic{objects}, all the different programs cooperate to edit the objects.


However, you need to do more than edit your documents.  You need to print 
them and send them to other people, both electronically and on paper. 
 Therefore, filters have been written to convert AUIS documents to several 
different formats.  To print your documents, you can convert them to 
PostScript.  To exchange documents with people running other word 
processing systems, you can convert them to RTF format, the most common 
document interchange format.  You can even convert your documents to 
straight ASCII, though of course all your fonts and pictures will be lost.


\heading{Styles}


One of the simplest things you can do with \italic{ez} is to add 
\bold{styles} to a document. Styles appear as menu items.  If you look at 
the menus, you will find quite a few styles can be chosen.  In Figure 1. 
you can see some of the more common styles - \italic{italics}, \bold{bold}, 
centered, or left/right justified, various font sizes to name a few.


Most styles can co-exist peacefully - \italic{italics} and \bold{bold} make 
\italic{\bold{bold-italics}}.  Others cancel each other out - like center 
and right-justify text.  Some styles you can apply more than once - 
changing font sizes can be done by successive applications of 
\bigger{b\bigger{i\bigger{g\bigger{g\bigger{e\bigger{r}}}}}}. To remove a 
style, select and area and chose the Plainer (F7) menu item.  To remove all 
styles, choose the Plainest (F8) menu item.  You can see what styles have 
been applied by placing the cursor at some point in your document and then 
entering "\bold{Esc-s}" (press the escape key and then press the "s" key). 
In the message area at the bottom of the screen you will see a description 
of the styles that apply at that point.


\heading{Insets}


In Figure 1. you can see our document has two types of data - conventional 
text and a small picture.  Technically each of these are 
\italic{\bold{insets}} to \italic{ez}.  An \italic{inset} is just a piece 
of data (an object) that has been inserted within some other data (other 
object).  An inset behaves exactly as if it were a separate document.  The 
only difference is that it may not have scroll bars. \



\italic{ez} can be configured so that when a file is to be created, a 
certain inset will be the default.  For instance, if you invoke \italic{ez} 
on a file called \bold{test.d} or \bold{test.doc}, the menus will contain 
the text inset with menu items you need for writing a paper like this one. 
 Of course, once the file has been created, \italic{ez} will automatically 
use the insets indicated by the data itself.  Similarly, editing a file 
called \bold{test.ras} results in the \italic{raster} inset being created. 
Editing \bold{test.html} results in an \italic{html} inset being created.


To create a new inset in a text document, move the cursor to where you want 
the inset to exist and select an item from the "Media" menu (or press 
Esc-Tab). In Figure 1. I selected the "raster" inset.  The causes a raster 
object to be created, initially appearing as an empty box.  Many insets 
(but not all) allow other insets to be embedded in them.


After adding a raster inset, you are editing two documents - each with 
different attributes. Which "editor" (\bold{object}) has control is based 
on where the cursor is positioned.  In this example if you click in the 
raster box, the menu cards will change to those of the \italic{raster} 
program.  Click in the text area outside the box and the text menus return.



\heading{Page Breaks, Footnotes and Headers}


Some insets have special meaning for the formatting of the text document. 
 They generally have their own menus.  If you select the "Insert Pagebreak" 
menu item, a thin horizontal line appears on the screen. This causes a new 
page to begin when the document is printed or previewed.


Footnotes can be added to your document with the "Insert Footnote" menu 
item.  The footnote will appear as a small square with a star in it. 
Footnotes can be open or closed by simply clicking in the box. They are 
displayed in-line as shown in Figure 3., but print at the bottom of the 
page, as you would normally expect.

\begindata{image,22887112}
format: gif
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
Ow==
\enddata{image, 22887112}
\view{imagev,22887112,42,0,0}
Figure 3. Sample Footnote and Header Insets


A page header or footer can be specified for your document by inserting the 
Header/Footer inset on the Media menu card.  As shown in Figure 3., you can 
specify various fields.  There are special $-keywords you can use for 
variable data.  Select the \italic{AUIS Help} menu and then select 
\italic{Show Help On...} When prompted, enter \bold{headers} to get more 
detail on headers.  Alternatively, you may enter \bold{auishelp headers} in 
an xterm window.


\heading{Having It Your Way with Styles}


The list of styles that come with \italic{ez} is usually sufficient for 
simple documents.  However, you may need better control of tab stops, 
paragraph indenting or outdenting, or double spaced lines.  Perhaps you 
want your own combination style that sets the margins, font, point size all 
from a single menu item.  All this and more can be done by selecting the 
"Edit Styles" menu option.  This brings up a second window which is divided 
into several panels, as shown in Figure 4.


\begindata{image,22332880}
format: gif
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
oZ2MHJP0e5cS3VCBFBEV5b7sHLuIeV2TLM6ABtMV/rzTlITOFDeo+nbNDmuoQCZQVfwTLbWA
tozQ3HC2/4lnnjmNwEagvDm0WSqyTDnVq+KqrpTVYb23K6XMTWVtMYvR0DKjhyDFuXa1orZ0
DJq6vpzWdPfWdxHX/ee7pqbKR82L5CrFeS2p04Z1fznWNqui42vCMezSqlvX89HTQwfY4HjR
LVpIj8VUh2y7lG3Kvws3VY2CfZyjiE3GINt6JHwfT6bYfMpkksk4eZZuLH23QNy9Wbm7ke2/
FnnaAJ3Rsd2nd+qT5rvXdP3T3AZpxNdKRoq8D3nOQQ2i4me/30jRGwzRYLuNcgJ3sSXTe8Oo
WJ19Abtj2E3VUHp90Q2v/jjX1h4bTVHddKQdJINFmsIoML/YlRhqLDRp30YMrfqdwPiIsnn1
jjV6uVZV3bgKOFIZ3I+cbNmL3yzG4KBZwSloS4wSryT4P3AkzDl5otVnzK4G3wiGhcSqYA8O
4aFI4tFKpNh42eZL3CV8m3Tc2CFenSgc4Hst4+ntc9A9c7hL3R954D1pi/o5obQWyanr3n5o
zR8+3YpHvPGY2fSZ217GkfF946nsc0qOlVWeyChapI18cf/93MA9l8VFZKBWqzG+tFFO4wJ+
b1qt5nVU47XNfTNcnqxKmTZu0Goq5TBOkT6qZBmuuyE65997c02+bE+u5SJuxETeknoNpy7O
/sy83axijn9Cjr9x3nGW1uMxNbOBHllujuNg6alQUpLfduRqXeC9jTZ8fmdDPIGL3OePObLg
DdmSXusLvqCwKNLkiza5znaA6J+2jtvaNtkYbOxDCOr87dfDvumRyK/lN4+hJ+3TTu3sYOub
h8TYHkCpru3Z3u0Tze3fTufiTh6oTe5Wfu7gAXTS15zpfsHG5+6MWeXJHu/WPej1fmXQnYHq
pz/oh+8k5u/rI6b/rrAjiISt6aVUSPDb0ksHSMALP2T8cvAAus8PD/Elxz2qc/FuZ/BMuDlY
tPFCnfE6GPJkLvEez4TlU/Koiz0VuPIZatYMKCb93oUvf7zLh/PM/u5g5m7zp97zqafzBB/0
+M7zP8+jRm+WQ0/0SC/vTD/rTh+bk0XvSUywFj/FU28eQ+vzUD+MsB3uDP/qnc7pYh+8aJqk
XN9w3A57MAy4u/7Dbi9mBI72Sa/oNzeG1SrkVk9yubn15gWxW+1qWO+uZlvmPUzCT+nqX8/l
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
\enddata{image, 22332880}
\view{imagev,22332880,43,0,0}
Figure 4. Edit Styles Window


The style editor window has its own menus.  The top part of the window, 
above the double line, is used to select a style to edit.  The left-hand 
panel is a list of all the menu cards that have style options.  If you 
select a menu card name, it is highlighted and its list of styles appears 
in the right-hand panel. Once you select a menu card and style name, the 
rest of the panel becomes active.  It displays all the attributes that 
apply to the selected style.


To change the attributes of a style, highlight the different options in the 
attribute panels.   The document will not change until the document is 
redrawn.  You can select "Update Document" to force this to happen right 
away.


Choose the menu item "Add Style" to create your own style. When you do this 
you are prompted in the message area at the bottom of the screen for a menu 
name.  You should enter a pair of names like "\bold{Region,MyWay}".  This 
will add a menu item "\italic{MyWay}" to the "\italic{Region}" menu. You 
should then select the attributes you want for this new \italic{MyWay} 
style.


When you edit or add styles with the style editor, the changes only affect 
the document you are currently editing.  They are saved in the data and 
will exist when you edit the document later.  If you copy text containing 
the new style and paste it in a new document, the style will be transferred 
to the new document, but the attributes of that style will not be 
transferred. This is because the attributes of the style are saved at the 
top of the file in an area you cannot see.  So when you copy the selected 
area, you only get the \underline{name} of the style, but not 
\underline{the definition of the style} (which you cannot see). You will 
need to either add a new \italic{MyWay} style to the second document or 
learn about \bold{templates}.


\heading{Templates}


A \bold{template} is a set of ready-made formatting information or 
instructions that you can apply to a text document.   Templates describe 
two types of information: \



\leftindent{\bold{\italic{Style specifications}}, which determine what 
formatting styles (such as boldface and centering) can be applied to a 
document, and exactly how they change the appearance of the text to which 
you apply them.  Thus templates lend a standard appearance to documents by 
making a style always look the same.  \



\italic{\bold{Set text}}, which is text that you want to include over and 
over in many documents.  A template saves you time in this case because you 
only have to type that text once.    \


}
\italic{ez} gives you an extensive set of templates.  Most of these include 
style specifications only, but some also include set text (the template 
used for creating memos is an example).  Creating a template is described 
in good detail in the on-line help (enter the command 
\italic{\bold{auishelp templates}} in an xterm window).


\heading{Printing and Previewing}


The entire AUIS system is designed to print using PostScript.  This was a 
decision made many years ago and is still in transition.  In this version 
AUIS objects all generate \italic{troff} output - along with copious 
amounts of embedded PostScript.  The troff is then processed to generate 
the necessary PostScript.  In the auis63L0-wp distribution the default 
print command will invoke a shell, \italic{/usr/andrew/etc/atkprint}.  The 
default preview command calls the shell 
\italic{/usr/andrew/etc/atkpreview}.  Each of these shells will invoke the 
\italic{groff} formatter to generate the PostScript output.  In 
\italic{atkpreview} the \italic{groff} output is piped into 
\italic{ghostview}.


If you do not have a PostScript printer, you can modify \italic{atkprint} 
to pipe the PostScript output through \italic{ghostscript} to generate the 
correct stream for your printer.  Andreas Klemm <andreas@knobel.knirsch.de> 
has written a filter, \italic{apsfilter} (available from ftp.germany.eu.net 
in /pub/comp/i386/Linux/Local.EUnet/People/akl//apsfilter*),  which works 
with your /etc/printcap entry and will automatically convert PostScript 
into the correct DeskJet stream.  I have a DeskJet 500 printer which works 
very well in this mode.


There are other ways to control printing/previewing by AUIS.  These will be 
discussed in a future article about tailoring your AUIS applications.


\heading{For More Information}


A mailing list is available at info-andrew@andrew.cmu.edu (mail to 
info-andrew-request@andrew.cmu.edu for subscriptions).  The newsgroup 
comp.soft-sys.andrew is dedicated to the discussion of AUIS.  A World Wide 
Web home page can be found at 
http://www.cs.cmu.edu:8001:/afs/cs.cmu.edu/project/atk-ftp/web/andrew-home.h\
tml.  A book, \italic{Multimedia Application Development with the Andrew 
Toolkit}, has been published by Prentice-Hall (ISBN 0-13-036633-1).  An 
excellent tutorial is available from the Consortium by sending mail to 
info-andrew-request@andrew.cmu.edu and asking about the manual, \italic{A 
User's Guide to AUIS}.



\heading{About the Author, Terry Gliedt}


\begindata{image,24232520}
format: gif
R0lGODdhaQCbAPcAAOTk5Ojo6OXl5d7e3tzc3N3d3dnZ2eDg4Ozs7O3t7ePj4+bm5vDw8Onp
6fHx8fT09O7u7tvb29ra2tPT0+vr6+rq6tbW1sbGxszMzOLi4tjY2MnJyaysrKKiooaGhoGB
gYyMjHp6enx8fKCgoKSkpLe3t8TExMDAwKmpqZGRkY6OjoCAgHt7e3FxcWpqand3d5OTk2dn
Z2tra2RkZGNjY2BgYFtbW1hYWFJSUldXV9XV1UpKSkxMTGxsbHh4eI2NjbKyst/f30REREZG
RomJiUVFRUFBQTw8PEBAQElJSU9PT7i4uDo6Oj4+PjQ0ND09PT8/P/Ly8jg4ODExMTU1NTk5
OTIyMjY2NoODg0JCQq2trc3NzWZmZqampm5ubp+fn9DQ0HR0dDMzM3V1dS4uLjAwMC0tLSoq
Kh0dHZeXl7S0tLCwsBgYGL6+vsfHxyEhIcjIyMrKyqGhoSQkJG1tbRoaGikpKVZWVqOjo7+/
v9HR0SYmJlFRUVlZWV9fX7y8vPj4+MHBwZaWllVVVSUlJbOzs5iYmF1dXU1NTTs7O8/Pz0ND
Q0dHRy8vLxISEhkZGR4eHigoKCwsLAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACwAAAAAaQCbAAAI/gABBBAwgACBAgAMFDgQgGGA
hwIQDEwwUMEBAAQWRAwgkSOCBB8ZNGhwcWQDjg4YMHiwUiVLBglignSQkiXLmg4eyITwIOfK
mjZ7wgTZIIGDBDoFHGigoECEAhImNBXwEMHIhw9BDgSgYOTHAAw+SkzQ4OtGsg1gqjSqUyXN
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
ByGVHdYFy3zhTkAAADs=
\enddata{image, 24232520}
\view{imagev,24232520,44,0,0}

After spending over twenty years with IBM, Terry left Big Blue last year. 
 Although he has worked with Un*x and AUIS for over six years, he is a 
relative new-comer to Linux.  Terry does contract programming, teaches 
classes in C/C++ and Unix and writes the occaisional technical document. 
 You can reach him at \italic{Software Toolsmiths}, (507) 356-4710 or by 
e-mail at \bold{tpg@mr.net}.


\enddata{text,19077980}
