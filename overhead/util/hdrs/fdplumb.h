/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#ifndef PLUMBFDLEAKS
#define PLUMBFDLEAKS

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <util.h>

#define open dbg_open
#define fopen dbg_fopen
#define close dbg_close
#define fclose dbg_fclose
#define popen dbg_popen
#define pclose dbg_pclose
#define topen dbg_topen
#define tclose dbg_tclose
#define t2open dbg_t2open
#define t2close dbg_t2close
#define qopen dbg_qopen
#define qclose dbg_qclose
#define creat dbg_creat
#define dup dbg_dup
#define dup2 dbg_dup2
#define pipe dbg_pipe
#define socket dbg_socket
#if !defined(hp9000s300) && !defined(M_UNIX)
#define socketpair dbg_socketpair
#endif
#define opendir dbg_opendir
#define closedir dbg_closedir

ENDCPLUSPLUSPROTOS

#endif /* PLUMBFDLEAKS */
