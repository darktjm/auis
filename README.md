Andrew User Interface System
============================

This is my slightly cleaned up version of the Andrew User Interface
System (http://www.cs.cmu.edu/~AUIS/), formerly known as the Andrew
Toolkit (ATK), known mostly for its editor, ez, whose main appeal was
the ability to embed documents of different types in each other.  Why
did I do it?  I don't know.  I guess I sort of liked ez back in the
day, as well as some of the other concepts introduced by atk.  There
is a similar project on sourceforge, one of the many projects that
were created and immediately abandoned without publishing any code.

I downloaded the latest version, released nearly 20 years ago in
February 1997 (ftp://ftp.andrew.cmu.edu/pub/AUIS/andrew-8.0.Z), and
checked that in as the first revision.  I then made it compile, and
somewhat work on my Linux-amd64 system.  Only one serious issue
remains: On the amd64 system, IDs stored in files are 64-bit, which
may make them incompatible with old, 32-bit systems.  I have that on
my todo list, though.

AUIS is copyrighted material; see the COPYRITE files in config/ for
details on that.  Any changes I made are not assigned to the Andrew
Consortium, IBM, or anyone else, but instead are released in the 
public domain.  That is, the product I'm making here is a cumulative
"diff" file from the last release, rather than a release in and of
itself.  Unfortunately, SVN flagged some files as binary on initial
checkin which weren't, so a plain git/SVN diff may not work as
expected.

See the file doc/README.ez (yeah, you'll need ez running to see this
properly) or download the README from the original distribution if you
care about that obsolete information.  I removed it so that this file
would appear in bitbucket.

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

  - Made it compile and run on some simple ez documents.
  
  - Removed all printf/scanf format warnings in code I was able to
    compile.  This was the main step needed to allow saving/loading
    files, since sizeof(int) != sizeof(long).

  - cleaned up warnings in much of overhead directory
  
  - started on documentation, partly in the form of an org file.  I
    will probably abandon this document eventually and make something
    better
  
  - fixed org so that it actually almost works.  I don't think org
    was production-ready even in '97.  It still has some usability
    issues and general code ugliness.
