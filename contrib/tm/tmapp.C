/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <tmapp.H>

#include <application.H>
#include <im.H>
#include <frame.H>
#include <tm19.H>
#include <tmview.H>
#include <environ.H>


ATKdefineRegistry(tmapp, application, NULL);

tmapp::tmapp()
{
    this->args=NULL;
    this->title=NULL;
    this->fileMenus=FALSE;
    this->menufile=NULL;
    this->rows=this->cols=0;
    THROWONFAILURE(  TRUE);
}

boolean tmapp::ParseArgs(int  argc,const char  **argv)
{
    if(!(this)->application::ParseArgs(argc,argv))
	return FALSE;

#define GETARGSTR(var)\
{\
    if((*argv)[2]!='\0')\
        var= ((*argv)[2]=='=' ? &(*argv)[3] : &(*argv)[2]);\
    else if(argv[1]==NULL){\
	fprintf(stderr,"%s: %s switch requires an argument.\n",(this)->GetName(),*argv);\
        return FALSE;\
    }else\
    	var= *++argv;\
}

    while(*++argv!=NULL && **argv=='-')
	switch((*argv)[1]){
		const char *temp;
	    case 't':
		GETARGSTR(this->title);
		break;
	    case 'F':
		this->fileMenus=TRUE;
		break;
	    case 'm':
		GETARGSTR(this->menufile);
		break;
	    case 'r':
		GETARGSTR(temp);
		this->rows=atoi(temp);
		break;
	    case 'c':
		GETARGSTR(temp);
		this->cols=atoi(temp);
		break;
	    default:
		fprintf(stderr,"%s: unrecognized switch: %s\n", (this)->GetName(), *argv);
		return FALSE;
	}

    if(*argv!=NULL)
	this->args=argv;

    return TRUE;
}

boolean tmapp::Start()
{
    class tm19 *tm;
    class tmview *tmv;
    class im *im;
    class frame *framep;

    if(!(this)->application::Start())
	return FALSE;

    if(this->args==NULL){
	static const char *args[2];
	args[0]=environ::Get("SHELL");
	if(args[0]==NULL)
	    args[0]="/bin/sh";
	args[1]=NULL;
	this->args=args;
    }

    {
	const char *p=strrchr(this->args[0],'/');
	if(p==NULL)
	    im::SetProgramName(this->args[0]);
	else
	    im::SetProgramName(p+1);
    }

    tm=new tm19;
    if(tm==NULL)
	return FALSE;

    if(this->rows>0 || this->cols>0){
	if(this->rows==0)
	    this->rows=(tm)->GetHeight();
	if(this->cols==0)
	    this->cols=(tm)->GetWidth();
	(tm)->SetScreenSize(this->cols,this->rows);
    }

    (tm)->StartProcess(this->args);

    tmv=new tmview;
    if(tmv==NULL)
	goto destroytm;

    (tmv)->SetFileMenus(this->fileMenus);
    (tmv)->SetTitle(this->title);

    if(this->menufile!=NULL)
	(tmv)->ReadShMenus(this->menufile);
    else{
	const char *menupref=environ::GetProfile("shmenu");
	const char *home=environ::Get("HOME");
	char buf[500];

	if(menupref!=NULL)
	   (tmv)->ReadShMenus(menupref);
	else if(home==NULL ||
		!(sprintf(buf,"%s/.shmenu",home),
		  (tmv)->ReadShMenus(buf))){
	    
	    const char *fileName;
	    
	    fileName = environ::AndrewDir("/lib/shmenu");
	    (tmv)->ReadShMenus(fileName);
	}
    }

    (tmv)->SetDataObject(tm);

    framep=new frame;
    if(framep==NULL) {
	fprintf(stderr,"tm: Could not allocate enough memory; exiting.\n");
	goto destroytmv;
    }

    (framep)->SetView((tmv)->GetApplicationLayer());
    (framep)->SetCommandEnable(TRUE);

    im=im::Create(NULL);
    if(im==NULL) {
	fprintf(stderr,"tm: Could not create new window; exiting.\n");
	goto destroyframe;
    }

    (im)->SetView(framep);

    (tmv)->WantInputFocus(tmv);

    return TRUE;

destroyframe:
    (framep)->Destroy();
destroytmv:
    (tmv)->Destroy();
destroytm:
    (tm)->Destroy();

    return FALSE;
}
