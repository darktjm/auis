/* C++ified by magic !@#%&@#$ */
#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/


 

#ifndef PLUMBFDLEAKS
#define PLUMBFDLEAKS
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <util.h>

#define open dbg_open
#define fopen dbg_fopen
#define close dbg_close
#define fclose dbg_fclose
#define vclose dbg_vclose
#define vfclose dbg_vfclose
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

#endif /* PLUMBFDLEAKS */

ENDCPLUSPLUSPROTOS
 
