/*                                                                   Multithreaded Management
                           MULTITHREADED MANAGEMENT OF SOCKETS
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module contains the routines for handling the set of active sockets currently in
   use by the multithreaded clients. It is an internal module to the Library, the
   application interface is implemented in the Event Module. Look for more information in
   the Multithread Specifications.
   
   This module is implemented by HTThread.c, and it is a part of the Library of Common
   Code.
   
 */
#ifndef HTTHREAD_H
#define HTTHREAD_H
/*

Initiation

   This function initiates the arrays of socket descriptors used in the Library. It is
   currently done in the private HTAccessInit function.
   
 */
extern BOOL HTThreadInit        NOPARAMS;/*

   NOTE It is VERY important that this one is called before the first request, as
   otherwise the socket bit arrays are uninitialized.
   
Registration of a Thread

   These two functions put a HTNetInfo object into the list of threads and takes it out
   again respectively. A normal place to call these functions would be on creation and
   deletion of the HTNetInfo data structure.
   
 */
extern void HTThread_new        PARAMS((HTNetInfo * new_net));
extern int  HTThread_clear      PARAMS((HTNetInfo * old_net));/*

Get Bit-arrays for Select()

   This function returns a copy of the current bit-arrays contaning the active sockets
   registered for READ and WRITE.
   
 */
extern int HTThreadGetFDInfo    PARAMS((fd_set * read, fd_set * write));/*

Registration of the State of a Socket

   When a new request is initiated from the client and a socket has been created, is gets
   registered in a linked list of HTNetInfo objects. The object stays in this list until
   the request has ended (either having an error or success as result). Every time the
   program execution gets to a point where a socket operation would normally block the
   program, this function is called in order to register the socket as waiting for the
   actual operation.
   
 */
typedef enum _HTThreadAction {
    THD_SET_WRITE=0,
    THD_CLR_WRITE,
    THD_SET_READ,
    THD_CLR_READ,
    THD_SET_INTR,
    THD_CLR_INTR,
    THD_CLOSE
} HTThreadAction;/*

 */
extern void HTThreadState PARAMS((SOCKFD sockfd, HTThreadAction action));/*

   This function makes life easier if you want to mark all sockets as interrupted.
   
 */
extern void HTThreadMarkIntrAll PARAMS((CONST fd_set * fd_user));/*

Is a Thread Interrupted?

   This function returns YES if the socket is registered as interrupted
   
 */
extern BOOL HTThreadIntr        PARAMS((SOCKFD sockfd));/*

Any Threads Registered?

   This function returns YES if any HTTP sockets are still registered in the set of active
   sockets. Otherwise it returns NO.
   
 */
extern BOOL HTThreadActive      NOPARAMS;/*

Select an Active Thread

   When the select function has returned a set of pending sockets this functions selects
   one of them and finds the correseponding request structure.
   
 */
extern HTRequest *HTThread_getRequest   PARAMS((CONST fd_set * fd_read,
                                                CONST fd_set * fd_write));

#endif/*

   End of HTThread module */
