Andrew Toolkit
==============

I decided to revive and clean up the Andrew User Interface System
(http://www.cs.cmu.edu/~AUIS/), formerly known as the Andrew Toolkit
(ATK), known mostly for its editor, ez, whose main appeal was the
ability to embed documents of different types in each other.  I have
since removed one of its biggest non-ATK components, AMS, so I've
changed the title of this README back to "Andrew Toolkit".  Why did I
do it?  I don't know.  I guess I sort of liked ez back in the day, as
well as some of the other concepts introduced by atk.  I'm sure I'll
abandon this again soon enough as the frustration and futility
overwhelm me, so take advantage of my work while you can.

I downloaded the latest version, released nearly 20 years ago in
February 1997 (ftp://ftp.andrew.cmu.edu/pub/AUIS/andrew-8.0.Z), and
checked that in as the first revision.  I then made it compile, and
somewhat work on my Linux-amd64 system.  I then proceeded to do
further cleanups and enhancements.

If you are looking for the original AUIS README, look at
doc/README.ez.  Yes, you'll need a working ez to read this properly,
although given its ASCII format you can probably get the gist without.
After buliding, this is also available in plain text as doc/README and
after installation as ${ANDREWDIR}/README.  You can also obtain the
ASCII version by downloading the original or reverting this file to
its original checkin.  Similarly, the files Porting, FAQ and INSTALL
were removed as they can be rebuilt from their ez source using
ez2ascii.

There is a similar atk maintenance project on sourceforge
(https://sourceforge.net/projects/atk).  It appears to be abandoned;
the last actual code change was made in 2002.  Some of the changes
include renaming symbols, moving files around, and source code
reformatting, all without any kind of document describing (in CVS or
otherwise) what they did, or why.  These and other pointless changes
make it unreasonably difficult for me to actually extract anything
useful.  So, while my project might benefit from some of the changes
made there, I won't even bother.

Legal Information
-----------------

AUIS is copyrighted material; see the COPYRITE files in doc/ for
details on that.  Any changes I made are not assigned to the Andrew
Consortium, IBM, or anyone else, but instead are released in the
public domain.  That is, the product I'm making here is a cumulative
"diff" file from the last release, rather than a release in and of
itself.  Unfortunately, SVN flagged some files as binary on initial
checkin which weren't, so a plain git/SVN diff may not work as
expected.  Instead, just diff the latest with upstream manually, and
keep in mind that I changed one file (COPYRITE in the top-level
directory) into a link.

Building and Installation
-------------------------

To build this on Linux, just run "make MFLAGS= World" in the top-level
directory, with /usr/local/auis writable by the user executing this
command.  Edit config/site.h if you don't like my installation
directory.  If you need the original INSTALL instructions, see
doc/atkprogramming/INSTALL.ez (or convert that to ASCII with ez2ascii
to get the original, as modified by me).  Yes, I know how stupid it is
to need an installed binary to read the installation instructions.
Again, atk writes ASCII files, so you should be able to read it anyway.

The following known dependencies currently exist:

  - X and other UNIX utilities scattered throughout.
    in particular, imake, even though it's not used much by X anymore.
  - ImageMagick.  You can change this at build-time to DevIL or
    FreeImage by changing IMAGEIO in config/site.mcr.
  - GNU roff.
  - gv. This can be overridden with the PSCPreviewCommand preference.
  - xditview.  This can be overridden with the PreviewCommand
    preference.  See below for replacing this with gv.
  - GNU make, or at least a make capable of dealing with GNU-style
    `$(shell ..)` and `ifeq` .. `endif`, such as makepp.
  - URW Nimbus fonts, installed in your X font path.  This can be
    overridden by either editing xmkfontd/non-auis.alias before
    installation or $ANDREWDIR/X11fonts/fonts.alias after installation,
    or inserting your own set of font aliases in the font path before
    andrew's.

I recommend the following preference settings in ~/.preferences:

  - `*.ErrorsToConsole: No`

    Without this, messages to standard error will disappear into the
    ether, unless you are running the AUIS commmand "console".

  - `*.ThumbScroll: Yes`
  
    This will enable continuous scrolling while dragging the knob.  In
    general, keep in mind that scrollbars don't work as you might think,
    especially if you've never used Athena-based X apps.  You might
    want to look at "help scroll".

  - `*.PreviewCommand: grops>/tmp/gps.$$; gv /tmp/gps.$$; rm /tmp/gps.$$`

    gv is a slightly better previewer than xditview.  Piping grops
    directly into gv only works for the first page, at best.  Note this
    only affects previews of troff-generated output; direct postscript
    output is handled by PSCPreviewCommand.

  - `*.Geometry: WxH`
  
    Replace W and H by something other than the weird default of
    512x512.

  - `*.UseXLFDNames: No`
  
    Use URW scaled fonts.

Incompatibilities
-----------------

So far, the following incompatibilities have been introduced:

 - When bulit for 64-bit systems, IDs stored in files are 64-bit,
   which means they may be too large to read in correctly on 32-bit
   systems.  That is, you can read files from 32-bit versions, but
   32-bit versions may have trouble reading new files.  Correcting
   this issue is on my todo list.

 - If the image serialization format is set to png (the new default),
   old versions of ATK will not be able to read it.  This change was
   necessary, because the old GIF reader/writer was 8-bit colormap
   only, and jpeg is/was lossy.

 - As part of cleaning up and correcting org, written org files are not
   compatible with the old org.  However, the old org was buggy as hell,
   so this is not a real problem.  Note that it can still read, and
   mostly cope with, old-style org files, even if they are partially
   broken (like the old example3.org).

 - I have removed AMS, and anything related ("White Pages", various
   mail utilities, AMS delivery).  A modern email client/server
   without imap support?  Well, having native atk/ez support is
   nice, and the AFS integration was probably nice when at CMU, but
   it's not worth trying to keep this crufty beast alive.  This
   includes "White pages" support (obsoleted by pervasive use of
   LDAP) and AFS support.  I did build AMS without AFS and White
   Pages support before tossing it, so if you're really interested,
   you can start from just before I chucked it.  While mostly unrelated,
   I have also removed metamail, since there are better ways to get it
   (other than the AMS hacks, but since I'm chucking AMS anyway...)

 - I have removed AFS support.  It was mostly a hack, anyway. The
   only useful things I can think of are directory permission and
   file flush semantics, neither of which is that important with
   any of the supplied apps.  Actually, I guess the console app
   does AFS status monitoring as well, but I'm not going to set up
   a dummy AFS cell just to test it.  Like AMS above, I actually
   removed the code, so that I don't have to deal with it when
   doing mass updates.

 - I have made the destructor for every class derived from traced
   protected, and added a dummy protected destructor to those classes
   which had none.  I did this using a macro, which will generate hate
   from anti-macro C++ers, but it saves typing, ensures consistency of
   implementation, and provides a hint to users of the class as to why
   they are not allowed to destroy the object directly.  As a side
   effect, I turned illegal object declarations into a sort of
   auto-freeing pointer using a template class (in violation of the
   anti-template coding guideline).

 - I have removed the ch files and tools required to convert out-of-tree
   pre-C++ code to C++.  Since I am unaware of any such code, this does
   not bother me.  The last checkin before I deleted it may be usable if
   this is a problem, but since I never compiled the associated code,
   it probably doesn't compile and/or work.

 - I have removed FlexOrLexFileRule(file); only flex should be supported.
   This means that any project using this rule must rename file.flex
   to file.l, and remove file.lex.  Maintaining two versions, one of
   which will likely never be used, is stupid.  Just force people to
   install flex (even in the cases where old-stlye lex is superior).
   Note that flex-generated scanners are essentially public domain, so
   this does not cause any licensing issues.  See the bottom 3 lines
   of flex's "COPYING".

 - I have removed NEOS (contrib/mit).  I doubt it's in use any more,
   and I don't feel like maintaining it (just updating to krb5 would
   take way too much effort).

 - I have removed all support for fbd font files (both the converter
   and the fbd-format orignals of the Andrew fonts).

 - I have removed all known compatibility aliases for proc names.
   This means that old key/menu binding files and ness scripts may no
   longer work.  These should be fixed to use the proper fully
   qualified proc name.  All 39 commands formerly prefixed by raster-
   are now prefixed by rasterv-.  In addition: 

     exit                  =>   im-exit-program

     play-keyboard-macro   =>   im-play-keyboard-macro

     redraw-window         =>   im-redraw-window

     srctextview-tab       =>   srctextview-reindent

     start-keyboard-macro  =>   im-start-keyboard-macro

     stop-keyboard-macro   =>   im-stop-keybaord-macro

  - When I added the generic image reader, I made the makefile somewhat
    dependent on GNU make.  That is, the make you use must support the
    `$(shell ...)` function and the `ifeq` .. `endif` construct.  In
    particular, the Single UNIX Spec make does not support these
    features.  Fix atk/basics/{common,lib}/Imakefile if this is a
    problem for you.  Adding doxygen support made such a dependency as 
    well; fix doc/Imakefile if you don't have `$(shell ...)`.

  - I have removed many obsolete symbols.  Any external code relying on
    them should study the SVN logs and update to the correct symbol (or
    remove the usage of that symbol) as needed.  I will not list these
    here; compile your code and if it fails, do the work to figure out
    why yourself.

  - I have changed the default font aliases file to use URW Nimbus
    fonts instead of Adobe fonts.  Adobe fonts shiped by MIT and
    AUIS are not scalable, so this allows scalable (unpixelated)
    fonts. The menus do not do the same font searching that the
    rest of the system uses.  I added fixed aliases for the few
    fonts used by default by the menus, but any preference
    overrides may now fail.  Use full X font names instead, or add
    your own aliases to the font path.  Similarly, if you don't
    want to edit fonts.alias yourself to use a different set of
    scaled fonts, you can add an override to your font path before
    the AUIS directory.  If you're using a dumb X terminal, you
    can probably get a font server to give you the appropriate
    fonts.

  - I changed parser::ParseNumber and cparser_ParseNumber without
    changing their documentation or prototypes:  beware!  If you pass
    in a length pointer, it must be initialized to the length of the
    text you want to parse.  This is to fix a bug where all current
    usages are in text buffers without termination, even though the
    routines assumed zero termination.

  - This is minor, but my coding rules differ from AndrewCoding.ez in
    the following ways:
    
    - Never use unlink; use remove instead.  Yes, unlink is
      technically safer than remove, since it doesn't remove
      directories, but remove isn't recursive, so removing an
      empty directory won't hurt anyone. In fact, anything in
      c89 should be preferred over "POSIX".  Also, only the open
      Single UNIX specification should be followed, not POSIX
      (opposite of AndrewCoding.ez).  Note that raise() is POSIX
      standard, but is so rarely used by real programs that
      maybe kill() is better. In general, I intend to rid this
      code of many portability macros, as the only systems that
      diverge are too old for me to care about.
      
    - Never use a BSD function unless it's in Single UNIX (yes, Single
      UNIX was around at the time of AUIS, but named X/Open instead)

    - C++ has been around long enough now that using multiple
      inheritance, nested types, exceptions, RTTI, and templates is probably
      fine.   The main thing to research is that they are compatible
      with dynamic loading, and how to portably instantiate templates
      at particular locations.  In particular, nested types are
      completely safe (and I am using them to clean up name spaces).
   
    - Leaving the burden of space ownership to the callee is safe, but
      inefficient.  Instead, document space ownership.  For example,
      if a function takes a string, it's likely the string is a constant
      whose lifetime is the program lifetime anyway, so why duplicate it?

    - One of the first things I introduced was UNUSED, which is a
      compiler-specific macro, in violation of AndrewCoding.ez.  I have
      added three more in the mean time.  Three of these are benign:
      they are merely lint-like modifiers to the warnings issued by the
      compiler.  NO_DLL_EXPORT is different, in that it is a
      non-portable way to control symbol exports.  There is no portable
      way to do this, unfortunately.

    - Only slightly related: almost none of the code in AUIS matches
      the guidelines in CodeAppearance.ez, so the fact that they violate
      my own guidelines shouldn't matter.  In particular:
      
      * (1.) I use `} else {' and '} while()' in violation of this rule,
        as I find it more readable and intuitive that code belonging to
	the brace is on the same line as the brace.  They even violate
	the rule themselves in 7.2.

      * (2.) Capping function complexity based on human short-term memory
        studies is retarded.  A function should do one thing that it's
	supposed to do, and nothing else.  Further breaking up of function
	should be based on code duplication removal, rather than complexity.
	Even the guideline itself admits the guideline is too stupidly
	restrictive.  Making a lot of little functions does not help
	legibility of the code, no matter what people have been saying for
	the last few decades.  Adding usable comments does a lot more for
	that than breaking up the code.

      * (3.1) I use 4-space indentation, and allow my editor to convert
	space aligned with 8-space tab stops to tab chars.  Yes, the
	dreaded "mixed" form, which everyone hates the most.  Bite me.
	Actually, I use 2-space tabs sometimes as well (e.g. for
	literate code, or for shell code).  Also, I never use two spaces
	within code for separation.  For the usages they provide, I strongly
	disagree with && and || and usually use a newline for ; and :.

      * (3.2) I prefer #if 0 over commenting, even though I still use
        commenting by default out of habit.  #if 0 allows my editor to
	syntax-highlight the commented-out code, and avoids any issues with
	embedded comments.

      * (3.3) "Avoid the use of #define".  Wow, I've heard that mantra for
        30 years now.  Admittedly, using enum in C and const in C++
        for constants costs little and is in some ways superior.  Now
	that some debuggers finally understand macros, there is little
	incentive not to use them.  Also, re the indentation:  all modern
	compilers support it (although I don't use it myself).

      * (4.) I guess it's not a bad naming convention, but there's too
	much stuff that doesn't use it to care.  Adding a convention for
	new code does nothing to alleviate that problem.  Just avoid
	names which conflict with std::..   It would've been better to
	have just adopted a namespace for all C++ code than a naming
	convention that is too little, too late.

      * (4.1) All upper-case needs to die.  Even for C macros.  I still
        use it by habit, though.

      * (4.2-4.4) come out of OO courses.  Screw them.  Some of these are
        just retarded, as well.  "fnd" for "find"?

      * (5.) Adding gratuitous newlines is rarely helpful.  Especially
        since for some reason monitors have been getting wider instead
        of taller.
	
      * (6.) Avoid goto doesn't belong in the same point, but in any
	case, they had a perfect opportunity to promote C++ exceptions,
	but didn't.  Maybe "avoid longjmp even more" would've helped.
	Also, as usual, their solution is convoluted and adds nothing
	over just using goto.

      * (7.) trailing brace comments are worthless.  Use your editor's
        brace matching while editing, and ensure proper indentation when
	not.

      * (7.3) I prefer "FALL THROUGH" over "DROP THRU".  Whatever.
	Also, I'll put the default case wherever it makes sense.
	One of my gripes with Ada was that I couldn't do that any more.
	Also, I prefer half-dedent for case rather than full-dedent.

      * (8.) Not placing the type to the left of the function name is
        the worst aspect of Andrew coding style.  I will remove that
	everywhere, if possible.  Also, repeating the name of the
	function you are about to define in the comment above it is
	redundant and adds maintenance overhead.

      * (9.)  Once again, half-dedent for accessibility keywords.  Also,
        private types may need to go above the public section.  In fact,
	I don't see any reason to force the pure binary split at all.

      * (9.2.) There's a reason the friend feature was put in.  Use it if
        you need it.

Goals
-----

What I intend to do with this is in the tjm-todo file, but to repeat
the main points:

  - ANSIfy.  Much of this code was written in the 80s, but modern code
    really needs to assume ANSI, and probably c99 at that (for stdint.h,
    at least).

  - constify - if a function doesn't modify *p, make p a pointer to
    const.  Also, assume string constants are const.  Also, any static
    data that isn't meant to be modified must be declared const.
    Too bad C has no way to acknowledge "taking ownership" of
    parameters that I know of, but if there were, I'd do that as well.

  - link-clean: ensure that all exported symbols are defined in a
    header, and all imported symbols come from a header (i.e., no extern
    in C files).  Also, any symbols not meant to be exported must be
    declared static.  Note that in C++, I am not aware of a method to
    prevent exporting class information other than the GCC-specific
    method of preventing leaks outside of a shared object.

  - make it as warning-clean as possible with the strictest set of
    warnings I can manage.

  - document - provide as complete documentation as possible for all
    provided classes, functions, and executables

  - ensure everything actually works (note that this means that I will
    just remove anything that I can't or don't want to put in the effort
    to verify, such as AMS, AFS, and support for hardware I don't have)

Progress
--------

So far, what I've done is:

  - Made it compile and run on some simple ez documents.  In particular,
    loaded all supplied documents into ez with valgrind running without
    raising an alarm.  This includes enabling ness scripts when present,
    and playing with active insets.  Note that I didn't actually play
    with all the ness or view every part of every document, so I'm
    continuing to find and fix issues as I do that.

  - Cleaned up warnings in everything remaining after the big
    deletes.  Note that the main thing that made most things work
    was printf/scanf warning removal, as most of the code assumed
    sizeof(int) == sizeof(long) == sizeof(void *) == 4.  Some such
    assumptions still exist, and require manual location and
    removal.

  - Started on documentation, now in a plain ez text file instead
    of org (org seemed nifty, but it can't print and you can't
    assign styles to node labels and the app is in general not
    very user-friendly).  If I don't get cracking on the latex
    output, I'll probably abandon this as well in favor of a plain
    latex document.  Also started a doxygen document, with some things
    actually documented.   I'm not happy with the formatting or
    contents (excessive in most places), but I'll continue investing
    in it.

  - Fixed org so that it actually almost works.  I don't think org
    was production-ready even in '97.  It still has some usability
    issues and general code ugliness.

  - Fixed bugs in bush as well, and made it usable as a dired
    replacement.

  - Removed all uses of csh.  Some systems don't even install csh
    any more.  Even tcsh is far superceded by zsh and other
    sh-variants. Note that in a few cases, this means I assume
    /bin/sh is a POSIX-compliant sh.  Back when I still had access
    to non-Linux UNIX, this was not the case everywhere, even on
    systems that had a POSIX sh (e.g. Solaris).  At the time, I
    detected that situation and re-execed with a POSIX sh. I'm not
    going to do that here, though, so if you try to run one of
    these scripts on one of those systems, tough.  At least I
    avoided the convenience of array variables, which, while
    suppported in at least three major POSIX-sh variants (ksh,
    bash, zsh), is not standard, and isn't supported by at least
    one known POSIX-sh variant (busybox).  I didn't make the scripts
    any more robust than they already were:  don't use spaces in file
    names.

  - Removed internal, sometimes obsolete image import/export routines
    and replaced with a generic external library (DevIL, FreeImage,
    or ImageMagick).
