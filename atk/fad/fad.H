#ifndef _fad_H_
#define _fad_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libfad libfad.so
 *  A dynamically loadable ATK class file.  Contains fad, fadview.
 * @{ */
#ifndef FAD_DEFS
#define FAD_DEFS
#define fad_NEWFAD 10000
#define OLD 1
#define NEW 0
#define PFUG 4
#define min(a,b) (a < b) ? a:b
#define ICONFLAG -10
#define LABELFLAG -10000
#define ANIMATEFLAG -9999
#define ISICONORLABEL(A) ((A) < 0)
#define ISICON(A) ((A) < 0 && (A) != LABELFLAG)
#define ISICONVEC(V) (ISICON(V->p2->x))
#define MAXWIDTH 20000
#define MAXHEIGHT 20000
#define LINEMODE 0
#define BOXMODE 1
#define ANIMATEMODE 2
#define FRTIME 30 	/* default minimum number of milliseconds between frames */
struct anivect {
	float x1,y1,y2,x2,dx1,dx2,dy1,dy2;
	char *label;
	char mode;
	};
struct fadpoint {
	long x,y;
	struct fadpoint *p;
	};
struct fadvector {
	struct fadpoint *p1,*p2;
	struct fadvector *v;
	char *label;
	char stat;
	char mode;
	};
struct fad_frame {
	struct fadvector *v;
	struct fadpoint *p;
	struct fad_frame *f;
	struct anivect *av;
	};
#endif /* FAD_DEFS */
#include <dataobject.H>
class fad : public dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();


     long Read(FILE *f, long id)  ;
     long Write(FILE *f, long writeid, int level)  ;
     void SetAttributes(struct attributes *attributes);

    virtual struct fadpoint * newpoint(long x,long y)  ;
    virtual struct fad_frame * newframe()  ;
    virtual void SetName(char *name);
    virtual struct fadpoint * setpoint(long x,long y, int type,struct fad_frame *f)  ;
    virtual struct fadvector * setvector(struct fadpoint *pp1,struct fadpoint *pp2,struct fad_frame *f)  ;
    virtual void delvector(struct fad_frame *f);
    virtual short iconnum(const char *s)  ;
    virtual void freeframe(struct fad_frame *ff);	
    virtual int flipicons() ;
    virtual int unflipicons() ;
    virtual void SetReadOnly(boolean readOnly);

    
	fad();
    
	virtual ~fad();

    short ox,oy,w,h,pltnum,desw,desh;
    struct fad_frame *f,*bf,*deleated;
    struct fadpoint *fp,*lp;
    struct fadvector *veclist[64];
    short  Frames,iconmode,currentfontindex,topinmp ,fad_square,frtime,readonly,initializedfonts ;
    char cfname[128],currentistr[128], currenticon,fadname[256], iconnamebuf[512],*inmp[15],labelfontname[256];
    class fontdesc *fontpt[15], *currentfont,*labelfont;
    struct fadpoint *iconpoints[200],**iconpointend, **iconpointnow;
    class cursor *cursor;
    char mode;
};
/** @} */
#endif /* _fad_H_ */
