/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("fad.H")
#include <dataobject.H>
#include <fad.H>
#include <fontdesc.H>
#include <attribs.h>
#define STARTHEIGHT 256
/* *********
struct fad *fad__NewData()
{
	struct fad *np;
	np = (struct fad *)malloc(sizeof(struct fad));
	fad_InitData(np);
	return(np);
	}
***************** */

ATKdefineRegistry(fad, dataobject, NULL);
static class fontdesc *my_DefineFont(const char  *fname);
static struct fadvector *newvector(struct fadpoint  *p1,struct fadpoint  *p2);
static boolean pointmatch(class fad  *self,struct fadpoint  *pt,long  x,long  y);


static class fontdesc *my_DefineFont(const char  *fname)
{
	char familyname[256];
	long fontStyle;
	long  fontSize;
	fontdesc::ExplodeFontName(fname,familyname, sizeof(familyname), &fontStyle, &fontSize);
	return fontdesc::Create(familyname,  fontStyle, fontSize);
	}
static struct fadvector *newvector(struct fadpoint  *p1,struct fadpoint  *p2)
{
	struct fadvector *nv;
	nv = (struct fadvector *)malloc( sizeof(struct fadvector));
	nv->p1 = p1;
	nv->p2 = p2;
	nv->v = NULL;
	nv->label = NULL;
	nv->mode = LINEMODE;
	return(nv);
	}
void fad::SetAttributes(struct attributes  *attributes)
{

    (this)->dataobject::SetAttributes( attributes);
    while (attributes) {
	if (strcmp(attributes->key, "readonly") == 0) {
	    (this)->SetReadOnly( attributes->value.integer);
	}
	attributes = attributes->next;
    }
}
void fad::SetReadOnly(boolean  readOnly)
{
    this->readonly = readOnly;
}
struct fadpoint *fad::newpoint(long  x,long  y)
{
	struct fadpoint *np,**iip;
	np = (struct fadpoint *)malloc(sizeof(struct fadpoint));
	np->x = x;
	np->y = y;
	np->p = NULL;
	if(ISICON(x)){
		for(iip = this->iconpoints; iip < this->iconpointend;iip++){
			if((*iip)->x == x && (*iip)->y == y) break;
			}
		 if(iip == this->iconpointend) *(this->iconpointend++) = np;
		}
	return(np);
	}

struct fad_frame *fad::newframe()
{
	struct fad_frame *fp;
	fp = (struct fad_frame *)malloc(sizeof(struct fad_frame));
	fp->f = NULL;
	fp->p = NULL;
	fp->v = NULL;
	fp->f = NULL;
	return(fp);
	}

fad::fad()
    {
    int i;
	this->f = (this)->newframe();
	this->bf = this->f;
	this->deleated = NULL;
	this->Frames = 30;
        *(this->fadname) = '\0';
	*(this->cfname) = '\0';
	*(this->currentistr) = '\0'; 
	this->iconpointend = this->iconpoints;
	this->iconpointnow = NULL;this->iconmode = 0;
	this->topinmp = 1; this->fad_square = 0;this->frtime = FRTIME;
	this->initializedfonts = 1;
	this->readonly = 0;this->currentfont = NULL;
	this->fp = NULL; this->lp = NULL;this ->pltnum = 0;
	this->ox = 0; this->oy = 0; this->w = 0; this->h = 0;
	this->desw = MAXWIDTH; this ->desh = STARTHEIGHT;
	strcpy(this->labelfontname,"andy12");
	this->labelfont = my_DefineFont("andy12");
	this->currentfont = NULL;
	this->mode = LINEMODE;
	for(i = 0; i < 15; i++)
	    this->fontpt[i] = NULL;
	for(i = 0; i < 200; i++)
	    this->iconpoints[i] = NULL;
	THROWONFAILURE( TRUE);
	}

void fad::SetName(char *name)
{
	strcpy(this->fadname,name);
}
struct fadpoint *fad::setpoint(long  x,long  y,int  type,struct fad_frame  *f)
{
	struct fadpoint *pt;
	if((pt = f->p) == NULL){
		if(type == OLD) return(NULL);
		f->p = (this)->newpoint(x,y);
		return(f->p);
		}
	while(1){
		if(pointmatch(this,pt,x,y)) return(pt);
		if(pt->p == NULL) break;
		pt = pt->p;
		}
	if(type == OLD) return(NULL);
	pt->p = (this)->newpoint(x,y);
	return(pt->p);
	}
static boolean pointmatch(class fad  *self,struct fadpoint  *pt,long  x,long  y)
{
	if(ISICONORLABEL(x))
		return(x == pt->x && y == pt->y);
	return((x < pt->x + PFUG) && (x > pt->x - PFUG )&& 
	  (y < pt->y + PFUG) &&( y > pt->y - PFUG)) ;
	}
struct fadvector *fad::setvector(struct fadpoint  *pp1,struct fadpoint  *pp2,struct fad_frame  *f)
{
	struct fadvector *vec,*pv;
	vec = newvector(pp1,pp2);
	if(f->v == NULL)
		 f->v = vec;
	else{
		for(pv = f->v; pv->v != NULL; pv = pv->v) ;
		pv->v = vec;
		}
	return(vec);
	}
void fad::delvector(struct fad_frame  *f)
{
	struct fadvector *vec,*pv;
	vec = NULL;
	if(f == NULL) return;
	pv = f->v;
	if(f->v != NULL){
		for(pv = f->v; pv->v != NULL; pv = pv->v)
			vec = pv ;
		if(vec == NULL) f->v = NULL;
		else vec->v = NULL;
		if(pv->label) free(pv->label);
		free((char *)pv);
		}
	return;
	}
short fad::iconnum(const char  *s)
{
	int i;
	char *c;
	for(i = 1; i < this->topinmp; i++){
		if(strcmp(s,this->inmp[i]) == 0) break;
		}
	if(i == this->topinmp){
		/* need test for too many fonts */
		if(this->topinmp == 1)  c = this->iconnamebuf;
		else for(c = this->inmp[this->topinmp - 1]; *c != '\0'; c++) ;
		this->inmp[this->topinmp++] = ++c;
		strcpy(c,s);
		this->currentfont = my_DefineFont(c);
		this->fontpt[i] = this->currentfont;
		}
	else this->currentfont = this->fontpt[i]; 
#ifdef DEBUG
	fprintf(stderr,"i = %d, self->topinmp = %d,s = %s\n",
		i,this->topinmp,s);
#endif /* DEBUG */
	return(-i);
	}
		
void fad::freeframe(struct fad_frame  *ff)
{
	struct fadpoint *pt,*nextpt;
	struct fadvector *vv,*nextvv;
	for(pt = ff->p;pt != NULL; pt = nextpt){
		nextpt = pt->p;
		free((char*)pt);
		}
	for(vv = ff->v; vv != NULL; vv = nextvv){
		nextvv = vv->v;
                if(vv->label) free(vv->label);
		free((char *)vv);
		}
	}
long fad::Write(FILE  *f,long  writeid ,int  level)
{
	int i;
	struct fadvector *vv;
	struct fad_frame *ff;
    if (this->writeID == writeid)  return (this)->GetID();
    this->writeID = writeid;
	fprintf(f,"\\begindata{fad,%ld}\n",this->id);
	for(i = 1; i < this->topinmp; i++)
		fprintf(f,"$N %s\n",this->inmp[i]);
	fprintf(f,"$C %d\n",this->Frames);
	fprintf(f,"$T %d\n",this->frtime);
	fprintf(f,"$L %s\n",this->labelfontname);
	fprintf(f,"$P %d,%d,%d,%d\n",this->ox,this->oy,this->desw,this->desh);
	for(ff = this->bf; ff != NULL; ff= ff->f){
		fprintf(f,"$F\n");
		for(vv = ff->v; vv != NULL; vv = vv->v){
		        if(vv->mode == BOXMODE)
				fprintf(f,"$B %ld,%ld %ld,%ld\n",vv->p1->x,vv->p1->y,vv->p2->x,vv->p2->y);
		        else if(vv->mode == ANIMATEMODE)
				fprintf(f,"$A %ld,%ld %ld,%ld\n",vv->p1->x,vv->p1->y,vv->p2->x,vv->p2->y);
			else if(vv->label == NULL)
				fprintf(f,"$V %ld,%ld %ld,%ld\n",vv->p1->x,vv->p1->y,vv->p2->x,vv->p2->y);
			else
				fprintf(f,"$S %ld,%ld\n%s\n",vv->p1->x,vv->p1->y,vv->label);
			}
		}
	fprintf(f,"$$\n");
	fprintf(f,"\\enddata{fad,%ld}\n",this->id);
	return (this)->GetID();
	}
long fad::Read(FILE  *f,long  id)
{
	char *c,s[256],*cc,*cp,str[256];
	int p1x,p1y,p2x,p2y,newf = 0,szz;
	struct fadvector *vv;
	struct fad_frame *ff = NULL;
	struct fadpoint *fp,*lp;
        if(id != 0L)this->id = (this)->UniqueID();
        this->modified = 0;
    /*Not currently concerned with embedded objects */
	c = s;
	c++;
	cc = c + 2;
	while(fgets(s,256,f) != NULL){
		if(*s == '\\'){
                    if(strncmp(s,"\\enddata",8) == 0) break;
                    if(strncmp(s,"\\begindata{",11) == 0){
                        char nnm[64]; long foo;
                        sscanf(s,"\\begindata{%s,%ld}",nnm,&foo);
                        if(strcmp(nnm,"fad") == 0) this->id = foo;
                    }
                }
		if(*s != '$') continue;
		switch(*c){
		case 'T': 
			this->frtime  = atoi(c+2);
			break;
		case 'C':
			this->Frames = atoi(c+2);
			break;
		case 'L':
			strcpy(this->labelfontname,c+2);
			for(cp = this->labelfontname; *cp != '\n' && *cp != '\0'; cp++);
			*cp = '\0';
			this->labelfont = my_DefineFont(this->labelfontname);
			break;
		case 'N':
			for(cp = cc; *cp != '\n' && *cp != '\0'; cp++);
			*cp = '\0';
			(this)->iconnum(cc);
			break;
		case 'F':
			if(newf){
				ff->f = (this)->newframe();
				ff = ff->f;
				}
			else newf = 1;
			break;
		case 'P':
			sscanf(s,"$P %d,%d,%d,%d\n",&p1x,&p1y,&p2x,&p2y);
			this->ox = 0; this->oy = 0; /* should be read as 0 */
			this->desw = p2x; this->desh = p2y;
			ff = this->f;
			this->bf = ff;
			newf = 0;
			break;
		case 'V':
			sscanf(s,"$V %d,%d %d,%d\n",&p1x,&p1y,&p2x,&p2y);
			fp = (this)->setpoint(p1x,p1y,NEW,ff);
			lp = (this)->setpoint(p2x,p2y,NEW,ff);
			vv = (this)->setvector(fp,lp,ff);
			if(ISICON(p2x)) this->currenticon = p2y;
			break;
		case 'A':
			sscanf(s,"$A %d,%d %d,%d\n",&p1x,&p1y,&p2x,&p2y);
			fp = (this)->setpoint(p1x,p1y,NEW,ff);
			lp = (this)->setpoint(p2x,p2y,NEW,ff);
			vv = (this)->setvector(fp,lp,ff);
			vv->mode = ANIMATEMODE;
			break;
		case 'B':
			sscanf(s,"$B %d,%d %d,%d\n",&p1x,&p1y,&p2x,&p2y);
			fp = (this)->setpoint(p1x,p1y,NEW,ff);
			lp = (this)->setpoint(p2x,p2y,NEW,ff);
			vv = (this)->setvector(fp,lp,ff);
			vv->mode = BOXMODE;
			break;
		case 'S':
			sscanf(s,"$S %d,%d\n",&p1x,&p1y);
			fgets(str,256,f);
			fp = (this)->setpoint(p1x,p1y,NEW,ff);
			lp = (this)->setpoint(LABELFLAG,LABELFLAG,NEW,ff);	
			vv = (this)->setvector(fp,lp,ff);
			szz = strlen(str);
			str[szz - 1] = '\0';
			vv->label = (char *)malloc(szz);
			strcpy(vv->label,str);
			break;
		case '$':
			break;
			}
	}
	(this)->NotifyObservers(fad_NEWFAD);
	return dataobject_NOREADERROR;
}
fad::~fad()
    {/* bug : label strings not currently freed */
	struct fad_frame *lf,*sf;
	for(lf = this->bf ;  lf != NULL ; lf = sf){
            sf = lf->f;
            (this)->freeframe(lf);
        }
}

int fad::flipicons()
{
	struct fadpoint *iip;
	if(this->iconpointend == this->iconpoints) return(0);
	if(this->iconpointnow == NULL
		 || ++(this->iconpointnow) ==this->iconpointend )
			this->iconpointnow = this->iconpoints;
	iip  = *(this->iconpointnow);
	this->currentfont = this->fontpt[-(iip->x)];
	this->currentfontindex = iip->x;
	this->currenticon = iip->y;
#ifdef DEBUG 
	fprintf(stderr,"cf = %d, ci = %d at %d\n",
		iip->x,iip->y,(int)(this->iconpointend - this->iconpoints));
#endif /* DEBUG  */
	return(iip->y);
	}
int
fad::unflipicons()
{
	struct fadpoint *iip;
	if(this->iconpointend == this->iconpoints) return(0);
	if(this->iconpointnow == NULL)
			this->iconpointnow = this->iconpoints;
	else if ( this->iconpointnow ==this->iconpoints ){
		this->iconpointnow = this->iconpointend - 1;
		}
	else 	this->iconpointnow--;
		
	iip  = *(this->iconpointnow);
	this->currentfont = this->fontpt[-(iip->x)];
	this->currentfontindex = iip->x;
	this->currenticon = iip->y;
#ifdef DEBUG 
	fprintf(stderr,"cf = %d, ci = %d at %d\n",
		iip->x,iip->y,(int)(this->iconpointend - this->iconpoints));
#endif /* DEBUG  */
	return(iip->y);
	}
