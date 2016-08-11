/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/nbutterv.ch,v 1.1 1993/10/13 15:52:18 rr2b Stab74 $ */
/* $ACIS: $ */
/* $Source: /afs/cs.cmu.edu/project/atk-src-C++/overhead/c++conv/chfiles/RCS/nbutterv.ch,v $ */

class nbutterv : sbuttonv {
overrides:
    Touch(int ind, enum view_MouseAction action) returns boolean;
};

