/*                               /Net/dxcern/userd/timbl/hypertext/WWW/Library/src/HTTCP.html
                              GENERIC NETWORK COMMUNICATION
                                             
 */
/*
**      (c) COPYRIGHT CERN 1994.
**      Please first read the full copyright statement in the file COPYRIGH.
*//*

   This module has the common code for handling TCP/IP and DECnet connections etc. The
   main topics of functions in this module are:
   
      Connection establishment
      
      Cache of host names
      
      Errno Messages
      
      Host and mail addresses
      
      Signal Handling
      
   This module is implemented by HTTCP.c, and it is a part of the Library of Common Code.
   
 */
#ifndef HTTCP_H
#define HTTCP_H
#include "HTAccess.h"/*

Connection Management

   All connections are established through the following functions.
   
  ACTIVE CONNECTION ESTABLISHMENT
  
   This makes an active connect to the specified host. The HTNetInfo structure is parsed
   in order to handle errors. Default port might be overwritten by any port indication in
   the URL specified as <host>:<port> If it is a multihomed host then HTDoConnect measures
   the time to do the connection and updates the calculated weights in the cache of
   visited hosts.
   
 */
extern int HTDoConnect ( HTNetInfo       *net,
                                char            *url,
                                u_short         default_port,
                                u_long          *addr,
                                BOOL            use_cur);/*

  PASSIVE CONNECTION ESTABLISHMENT
  
   This function makes a non-blocking accept on a port and polls every second until
   MAX_ACCEPT_POLL is reached.
   
 */
extern int HTDoAccept (HTNetInfo *net);/*

Caching Hosts Names

   This part of the HTTCP module maintains a cache of all visited hosts so that subsequent
   connects to the same host doesn't imply a new request to the DNS every time.
   
   Multihomed hosts are treated specially in that the time spend on every connect is
   measured and kept in the cache. On the next request to the same host, the IP-address
   with the lowest average connect time is chosen. If one IP-address fails completely,
   e.g. connection refused then it disabled and HTDoConnect tries one of the other
   IP-addresses to the same host.
   
   If the connect fails in the case of at single-homed host then the entry is removed from
   the cache and HTDoConnect tries again asking the DNS.
   
  RECALCULATING THE TIME-WEIGHTS ON MULTIHOMED HOSTS
  
   On every connect to a multihomed host, the average connect time is updated
   exponentially for all the entries.
   
 */
extern void HTTCPAddrWeights (char * host, time_t deltatime);/*

  CLEANUP THE MEMORY
  
   This function is called from HTLibTerminate.
   
 */
extern void HTTCPCacheRemoveAll (void);/*

  CONTROL VARIABLES
  
   This parameter determines the maximum number of hosts in the cache. The default number
   is 500.
   
 */
extern unsigned int     HTConCacheSize;/*

System Description of Error Message

   Return error message corresponding to errno number given. We need to pass the error
   number as a parameter as we on some platforms get different codes from sockets and
   local file access.
   
 */
extern CONST char * HTErrnoString (int errornumber);/*

  PARSE A CARDINAL VALUE
  
 */
/*      Parse a cardinal value                                 parse_cardinal()
**      ----------------------
**
** On entry:
**      *pp points to first character to be interpreted, terminated by
**      non 0..9 character.
**      *pstatus points to status already valid,
**      maxvalue gives the largest allowable value.
**
** On exit:
**      *pp points to first unread character,
**      *pstatus points to status updated iff bad
*/

extern unsigned int HTCardinal (int *            pstatus,
                                       char **          pp,
                                       unsigned int     max_value);/*

Internet Name Server Functions

   The following functions are available to get information about a specified host.
   
  PRODUCE A STRING FOR AN INTERNET ADDRESS
  
   This function is equivalent to the BSD system call inet_ntoa in that it converts a
   numeric 32-bit IP-address to a dotted-notation decimal string. The pointer returned
   points to static memory which must be copied if it is to be kept.
   
 */
extern CONST char * HTInetString (struct sockaddr_in * sin);/*

  PARSE AN INTERNET NODE ADDRESS AND PORT
  
   This function finds the address of a specified host and fills out the sockaddr
   structure. str points to a string with a node name or number, with optional trailing
   colon and port number. sin points to the binary internet or decnet address field.
   
   On exit *sin is filled in. If no port is specified in str, that field is left unchanged
   in *sin. On success, the number of homes on the host is returned.
   
 */
extern int HTParseInet ( struct sockaddr_in *    sin,
                                CONST char *            str,
                                BOOL                    use_cur);/*

  NAME OF A MACHINE ON THE OTHER SIDE OF A SOCKET
  
   This function should have been called HTGetHostByAddr but for historical reasons this
   is not the case.
   
   Note:This function used to be called HTGetHostName but this is now used to find you own
   host name, see HTGetHostName()
   
 */
extern char * HTGetHostBySock (int soc);/*

  HOST ADDRESS RETUNED FOR SPECIFIED HOST NAME
  
   This function gets the address of the host and puts it in to the socket structure. It
   maintains its own cache of connections so that the communication to the Domain Name
   Server is minimized. If OK and single homed host then it returns 0 but if it is a
   multi-homed host then 1 is returned.
   
 */
extern int HTGetHostByName (char *host, SockA *sin, BOOL use_cur);/*

  GET NAME OF THIS MACHINE
  
   This function returns a CONET char pointer to a static location containing the name of
   this host or NULL if not available.
   
 */
extern CONST char * HTGetHostName (void);/*

  SET NAME OF THIS MACHINE
  
   This function overwrites any other value of current host name. This might be set by the
   user to change the value in the ID value parsed to a news host when posting. The change
   doesn't influence the Mail Address as they are stored in two different locations. If,
   however, the change is done before the first call to HTGetMailAddress() then this
   function will use the new host and domain name.
   
 */
extern void HTSetHostName (char * host);/*

  CLEANUP MEMORY
  
   Called from HTLibTerminate
   
 */
extern void HTFreeHostName (void);/*

  GET DOMAIN NAME OF THIS MACHINE
  
   This function rerturns the domain name part of the host name as returned by
   HTGetHostName() function. Changing the domain name requires a call to  HTSetHostname().
   
 */
extern CONST char *HTGetDomainName (void);/*

  GET USER MAIL ADDRESS
  
   This functions returns a char pointer to a static location containing the mail address
   of the current user. The static location is different from the one of the current host
   name so different values can be assigned. The default value is <USER>@hostname where
   hostname is as returned by HTGetHostName().
   
 */
extern CONST char * HTGetMailAddress (void);/*

  SET USER MAIL ADDRESS
  
   This function overwrites any other value of current mail address. This might be set by
   the user to change the value in the  From field in the HTTP Protocol.
   
 */
extern void HTSetMailAddress (char * address);/*

  FREE MEMORY
  
   Called by HTLibTerminate
   
 */
extern void HTFreeMailAddress (void);/*

Signal Handling

   This is only necessary to compile on a few platforms and only if the application does
   not have its own signal handling.
   
 */
#ifdef WWWLIB_SIG
extern void HTSetSignal (void);
#endif

#endif   /* HTTCP_H *//*

   End of file */
