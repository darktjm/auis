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

/** \addtogroup libutil
 * @{ */
/** \addtogroup fdplumb
 * @{ */
/** \file include/fdplumb.h Include this file to override system
 *        file operations with fdplumb versions.  */
#define open dbg_open /**< fdplumb override */
#define fopen dbg_fopen /**< fdplumb override */
#define close dbg_close /**< fdplumb override */
#define fclose dbg_fclose /**< fdplumb override */
#define popen dbg_popen /**< fdplumb override */
#define pclose dbg_pclose /**< fdplumb override */
#define topen dbg_topen /**< fdplumb override */
#define tclose dbg_tclose /**< fdplumb override */
#define t2open dbg_t2open /**< fdplumb override */
#define t2close dbg_t2close /**< fdplumb override */
#define qopen dbg_qopen /**< fdplumb override */
#define qclose dbg_qclose /**< fdplumb override */
#define creat dbg_creat /**< fdplumb override */
#define dup dbg_dup /**< fdplumb override */
#define dup2 dbg_dup2 /**< fdplumb override */
#define pipe dbg_pipe /**< fdplumb override */
#define socket dbg_socket /**< fdplumb override */
#if !defined(hp9000s300) && !defined(M_UNIX)
#define socketpair dbg_socketpair /**< fdplumb override */
#endif
#define opendir dbg_opendir /**< fdplumb override */
#define closedir dbg_closedir /**< fdplumb override */

/** @} */
/** @} */

ENDCPLUSPLUSPROTOS

#endif /* PLUMBFDLEAKS */
