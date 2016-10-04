#ifndef _ezdiff_H_
#define _ezdiff_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libezdiff Object ezdiff
 *  A dynamically loadable ATK class file.  Contains ezdiff.
 * @{ */
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
/** @} */
#endif /* _ezdiff_H_ */
