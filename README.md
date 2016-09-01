Andrew User Interface System
============================

This is my slightly cleaned up version of the Andrew User Interface
System (http://www.cs.cmu.edu/~AUIS/), formerly known as the Andrew
Toolkit (ATK), known mostly for its editor, ez, whose main appeal was
the ability to embed documents of different types in each other.

If you are looking for the original AUIS README, look at
doc/README.ez.  Yes, you'll need a working ez to read this properly.
After buliding, this is also available in plain text
as doc/README and after installation as ${ANDREWDIR}/README.  You can
also obtain the ASCII version by downloading the original or reverting
this file to its original checkin.  Similarly, the files Porting, FAQ
and INSTALL were removed as they can be rebuilt from their ez source
using ez2ascii.

To build this on Linux, just run "make World" in the top-level
directory, with /usr/local/auis writable by the user executing this
command.  Edit config/site.h if you don't like my installation
directory.  If need the original INSTALL instructions, see
doc/atkprogramming/INSTALL.ez (or convert that to ASCII with ez2ascii
to get the original, as modified by me).  Yes, I know how stupid it is
to need an installed binary to read the installation instructions.

I downloaded the latest version, released nearly 20 years ago in
February 1997 (ftp://ftp.andrew.cmu.edu/pub/AUIS/andrew-8.0.Z), and
checked that in as the first revision.  I then made it compile, and
somewhat work on my Linux-amd64 system.  I then proceeded to do
further cleanups and enhancements.

Why did I do it?  I don't know.  I guess I sort of liked ez back in
the day, as well as some of the other concepts introduced by atk.
There is a similar atk maintenance project on sourceforge, one of the
many projects that were created and immediately abandoned without
publishing any code.

AUIS is copyrighted material; see the COPYRITE files in config/ for
details on that.  Any changes I made are not assigned to the Andrew
Consortium, IBM, or anyone else, but instead are released in the 
public domain.  That is, the product I'm making here is a cumulative
"diff" file from the last release, rather than a release in and of
itself.  Unfortunately, SVN flagged some files as binary on initial
checkin which weren't, so a plain git/SVN diff may not work as
expected.

So far, the following incompatibilities have been introduced:

 - When bulit for 64-bit systems, IDs stored in files are 64-bit, which
   means they may be too large to read in correctly on 32-bit systems.
   Correcting this issue is on my todo list.

 - As part of cleaning up and correcting org, written org files are not
   compatible with the old org.  However, the old org was buggy as hell,
   so this is not a real problem.

 - I have removed the ch files and tools required to convert out-of-tree
   pre-C++ code to C++.  Since I am unaware of any such code, this does
   not bother me.  The last checkin before I deleted it may be usable if
   this is a problem, but since I never compiled the associated code,
   it probably doesn't compile and/or work.

In any case, what I intend to do with this is in the tjm-todo file,
but to repeat the main point:

  - ANSIfy.  Much of this code was written in the 80s, but modern code
    really needs to assume ANSI, and probably c99 at that (for stdint.h,
    at least).

  - constify - if a function doesn't modify *p, make p a pointer to
    const.  Also, assume string constants are const.  Also, any static
    data that isn't meant to be modified must be declared const.

  - link-clean: ensure that all exported symbols are defined in a
    header, and all imported symbols come from a header (i.e., no extern
    in C files).

  - make it as warning-clean as possible with the strictest set of
    warnings I can manage.

  - document - provide as complete documentation as possible for all
    provided classes, functions, and executables

  - ensure everything actually works

So far, what I've done is:

  - Made it compile and run on some simple ez documents.  In particular,
    loaded all supplied documents into ez with valgrind running without
    raising an alarm.  This includes enabling ness scripts when present,
    and playing with active insets.
  
  - Removed all printf/scanf format warnings in code I was able to
    compile.  This was the main step needed to allow saving/loading
    files, since sizeof(int) != sizeof(long).

  - Removed all warnings related to string literals not being constant

  - Cleaned up warnings in much of overhead and atk/basics directories
  
  - Removed pervasive incorrect use of "it's"

  - Started on documentation, partly in the form of an org file.  I
    will probably abandon this document eventually and make something
    better
  
  - Fixed org so that it actually almost works.  I don't think org
    was production-ready even in '97.  It still has some usability
    issues and general code ugliness.

  - Fixed bugs in bush as well, and made it usable as a dired replacement
