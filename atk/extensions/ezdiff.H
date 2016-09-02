ATK_INTER
#ifndef _ezdiff_H_
#define _ezdiff_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


#include <observable.H>

class ezdiff : public observable {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void ObservedChanged (class observable *changed, long value);

    static boolean InitializeClass();
    ezdiff();    
    virtual ~ezdiff();

    static class ezdiff *Create(class buffer *buf1, class buffer *buf2, char *args);

    class mark **m1;
    class mark **m2;
    class buffer *buf[2];
    char *bname[2];
    int numbufs;
    int nummarks;
    int cmark;
    class ezdiff *next;
    boolean asking;
};

#endif /* _ezdiff_H_ */
