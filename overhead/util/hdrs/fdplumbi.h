/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS

/** \addtogroup libutil
 * @{ */
/** \addtogroup fdplumb
 * @{ */

/** Descriptor type for RegisterOpenFile */
enum fdleak_code {
    FDLEAK_OPENCODE_OPEN=1, /**< open(2) */
    FDLEAK_OPENCODE_FOPEN, /**< fopen(3) */
    FDLEAK_OPENCODE_POPEN, /**< popen(3) */
    FDLEAK_OPENCODE_QOPEN, /**< qopen */
    FDLEAK_OPENCODE_TOPEN, /**< topen */
    FDLEAK_OPENCODE_T2OPEN, /**< t2open */
    FDLEAK_OPENCODE_CREAT, /**< creat(2) */
    FDLEAK_OPENCODE_DUP, /**< dup(2) */
    FDLEAK_OPENCODE_PIPE, /**< pipe(2) */
    FDLEAK_OPENCODE_SOCKET, /**< socket(2) */
    FDLEAK_OPENCODE_OPENDIR /**< opendir(3) */
};

void RegisterOpenFile(int fd, const char *path, enum fdleak_code Code);
/**< Log a descriptor begin opened.  For implementation of additional
 *   fdplumb wrappers.
 *   \param fd The UNIX file descriptor
 *   \param path The name associated with this descriptor
 *   \param Code The category of this file descriptor */
void RegisterCloseFile(int fd);
/**< Log a descriptor being closed.  For implementation of additional
 *   fdplumb wrappers.
 *   \param fd The file descriptor; must match one passed to RegisterOpenFile(). */

/** @} */
/** @} */

ENDCPLUSPLUSPROTOS

