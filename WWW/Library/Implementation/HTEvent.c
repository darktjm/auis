/*				    				      HTEvent.c
**	MULTITHREADED ACCESS MANAGER
**
**	(c) COPYRIGHT CERN 1994.
**	Please first read the full copyright statement in the file COPYRIGH.
**
** Authors:
**	HFN    	Henrik Frystyk Nielsen <frystyk@dxcern.cern.ch>
**
** History:
**      05 Jul 95 Written from scratch
**
** Bugs
**
*/

/* Library include files */
#include "tcp.h"
#include "HTUtils.h"
#include "HTAccess.h"
#include "HTError.h"
#include "HTThread.h"
#include "HTList.h"
#include "HTEvent.h"					 /* Implemented here */

/* Type definitions and global variables etc. local to this module */
PRIVATE HTList *UserList = NULL;	      /* List of call back functions */
PRIVATE	fd_set	HTfd_user;		   /* Bit array of sockets from user */

/* --------------------------------------------------------------------------*/

/*							        HTEventRegister
**
**	Registers a socket as active for READ together with a callback
**	function to call when select returns that the socket is pending.
*/
PUBLIC BOOL HTEventRegister (HTEventCallBack * user_fd)
{
    static BOOL done=NO;
    if (!user_fd) {
	if (THD_TRACE)
	    fprintf(TDEST, "EventRegist. Bad argument\n");
	return NO;
    }
    if (!done) {
	FD_ZERO(&HTfd_user);
	done = YES;
    }
    if (!UserList)
	UserList = HTList_new();
    HTList_addObject(UserList, (void *) user_fd);
    HTThreadState(user_fd->sockfd, THD_SET_READ);
    FD_SET(user_fd->sockfd, &HTfd_user);
    return YES;
}


/*							         HTEventCleanup
**
**	Clears the list of registered user sockets and call back functions
*/
PUBLIC void HTEventCleanup (void)
{
    if (UserList) {
	HTList_delete(UserList);
	UserList = NULL;
    }
}


/*							      HTEventCheckState
**
**	This function checks the return code from the client call back 
**	function HTEventHandler() and HTEventRequestTerminate().
**
**	Returns:
**		YES	If we stay in eventloop
**		NO	If we return to client application
*/
PRIVATE BOOL HTEventCheckState (HTRequest * request, HTEventState ret)
{
    switch (ret) {
      case EVENT_TERM:					   /* Ignore it here */
      case EVENT_OK:
	break;
	
      case EVENT_INTR:
 	{
	    int sockfd;
	    if (request && request->net_info &&
		(sockfd = request->net_info->sockfd) >= 0)
		HTThreadState(sockfd, THD_SET_INTR);
	}
	break;

      case EVENT_INTR_ALL:
	HTThreadMarkIntrAll(&HTfd_user);
	break;
	
	break;

      case EVENT_QUIT:
	return NO;
	break;	       
	
      default:
	if (THD_TRACE)
	    fprintf(TDEST, "HTEventLoop. Invalid return code\n");
    }
    return YES;
}    


/*								HTEventLoop
**
**	This is the internal socket event loop that is executed when the
**	client doesn't want to keep the event loop. The client can interrupt
**	this loop by sending an event on on of the registered sock descriptors
**	in HTThreadInit() in the HTThread.c module
**
**	The input parameters are for the first load, i.e. the home page.
**	The keywords can be NULL. The Home Anchor can easily be found using
**	HTHomeAnchor().
**
**	Returns:
**		HT_ERROR	if can't get first document or error in select
**				(only if it failed with no blocking)
**		HT_OK		On normal exit
*/
PUBLIC int HTEventLoop (HTRequest *	homerequest,
			     HTParentAnchor *	homeanchor,
			     CONST char *	homekeywords)
{
    BOOL user_event = NO;
    int status, selres;
    int cnt;
    HTEventState state;
    HTRequest *cur_request = NULL;

    /* Load the Home Page and jump into the event loop */
    status = (homekeywords && *homekeywords) ?
	HTSearch(homekeywords, homeanchor, homerequest) :
	    HTLoadAnchor((HTAnchor *) homeanchor, homerequest);
    if (status != HT_WOULD_BLOCK) {
	if (THD_TRACE)
	    fprintf(TDEST,"HTEventLoop. Calling Terminate\n");
	state = HTEventRequestTerminate(homerequest, status);
	if (!HTEventCheckState(homerequest, state)) {
	    return HT_OK;
	}
    }

    while (1) {
	fd_set fd_read, fd_write;		      /* Set up working copy */
	int maxfdpl = HTThreadGetFDInfo(&fd_read, &fd_write);

#ifdef __hpux
	selres = select(maxfdpl, (int *) &fd_read, (int *) &fd_write,
			   (int *) NULL, NULL);
#else
	selres = select(maxfdpl, &fd_read, &fd_write, NULL, NULL);
#endif
	if (THD_TRACE)
	    fprintf(TDEST, "HTEventLoop. Select returns: %d\n", selres);
	if (selres < 0) {
	    HTErrorSysAdd(homerequest, ERR_FATAL, socerrno, NO, "select");
	    return HT_ERROR;
	}
	if (THD_TRACE) {
	    int cnt;
	    fprintf(TDEST, "HTEventLoop. Sockets ready for read : ");
	    for (cnt=0; cnt<maxfdpl; cnt++) {
		if (FD_ISSET(cnt, &fd_read))
		    fprintf(TDEST, "%d ", cnt);
	    }
	    fprintf(TDEST, "\n");
	    fprintf(TDEST, "HTEventLoop. Sockets ready for write: ");
	    for (cnt=0; cnt<maxfdpl; cnt++) {
		if (FD_ISSET(cnt, &fd_write))
		    fprintf(TDEST, "%d ", cnt);
	    }
	    fprintf(TDEST, "\n");
	}

        /* Any events on a user socket? - all sockets are examined */
	for (cnt=0; cnt<maxfdpl; cnt++) {
	    if (FD_ISSET(cnt, &fd_read) && FD_ISSET(cnt, &HTfd_user)) {
		HTList *cur = UserList;
		HTEventCallBack *pres;
		while ((pres = (HTEventCallBack *) HTList_nextObject(cur))) {
		    if (FD_ISSET(cnt, &HTfd_user)) {
			if (THD_TRACE)
			    fprintf(TDEST,"HTEventLoop. Calling handler %p\n",
				    pres->callback);
			state = (*(pres->callback))(&cur_request);
			if (state == EVENT_TERM)
			    state=HTEventRequestTerminate(cur_request,status);
			if (!HTEventCheckState(cur_request, state))
			    return HT_OK;
			user_event = YES;
		    }
		}
	    }
	}
	if (user_event) {
	    user_event = NO;
	    continue;			 /* Go check for any new User events */
	} else {
	    if (THD_TRACE)
		fprintf(TDEST, "HTEventLoop. No user events found - look for Library events\n");
	}

	/* Any events on a Library socket? - only do this when no user sockets
	   are pending */
	if ((cur_request = HTThread_getRequest(&fd_read, &fd_write)) != NULL) {
	    HTMethod method = cur_request->method;

	    /* The Library call-back function should be decided in HTProtocol
	       structure as a function of the method used */

	    HTProtocol *callback = (HTProtocol *)
		HTAnchor_protocol(cur_request->anchor);
	    if (method == METHOD_GET || method == METHOD_HEAD) {
		status = (*callback->load)(cur_request);
	    } else {
		if (THD_TRACE)
		    fprintf(TDEST, "HTEventLoop. Method not supported @@@\n");
	    }
	    if (THD_TRACE)
		fprintf(TDEST, "HTEventLoop. Load returns: %d\n", status);
	    if (status != HT_WOULD_BLOCK) {
		HTLoadTerminate(cur_request, status);
		if (THD_TRACE)
		    fprintf(TDEST,"HTEventLoop. Calling Terminate\n");
		state = HTEventRequestTerminate(cur_request, status);
		if (!HTEventCheckState(cur_request, state)) {
		    return HT_OK;
		}
	    }
	} else {
	    if (THD_TRACE)
		fprintf(TDEST, "HTEventLoop. No library events found\n");
	}
    }
    return HT_ERROR;				    /* Should never get here */
}

