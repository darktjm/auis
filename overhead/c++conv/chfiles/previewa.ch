/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/previewa.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS:previewa.ch 1.2$ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/previewa.ch,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidpreviewapp_H = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/previewa.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $";
#endif /* !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS) */

class previewapp[previewa] : application[app] {
    classprocedures:
      InitializeObject(struct ezprintapp *self) returns boolean;
    overrides:
	ParseArgs(int argc,char **argv) returns boolean;
        Start() returns boolean;
	Run() returns int;
};
