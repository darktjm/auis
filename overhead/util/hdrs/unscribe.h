/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libutil
 * @{ */

/*
	Include file for clients of unscribe routines.
*/

#include <atkproto.h>
#include <stdio.h>
BEGINCPLUSPLUSPROTOS

struct ScribeState; /**< Scribe processing state.  Create with UnScribeInit() */

extern int UnScribeInit(const char  *fieldvalue, struct ScribeState  **refstate);
/**< Allocate and initialize a state for converting scribe text to ATK text.
 *   \param fieldvalue The value of the X-Andrew-ScribeFormat: header.  If the
 *          format described by this header is not supported by the package,
 *          failure is returned.
 *   \param refstate If no error is encountered, this is filled in with
 *                   a newly allocated state for use with other functions.
 *   \return a code >=0 for OK, < 0 for error.  On error, no state is
 *           returned.  The code returned here must be passed to other
 *           processing functions, in addition to the refstate return.
 *           Error value of -1 means that the field value wasn't recognized;
 *           error value of -2 means that a malloc failed */

extern int UnScribe(int  code , struct ScribeState  **refstate, const char  *text, int  textlen, FILE  *fileptr);
/**< Writes converted scribe text to a file.  Multiple consecutive calls
 *   imply a continguous scribe document.
 *   \param code The return code from UnScribeInit()
 *   \param refstate The state structure returned by UnScribeInit()
 *   \param text The scribe text
 *   \param textlen The length of scribe text
 *   \param fileptr The output file
 *   \return The number of characters written (>=0) if OK.  -1 on error;
 *           errno contains more error information */
extern int UnScribeFlush(int  code, struct ScribeState  **refstate, FILE  *fileptr);
/**< Finish converting scribe text, outputing any remaining buffered text
 *   and freeing up processing state.
 *   \param code The return code from UnScribeInit()
 *   \param refstate The state structure returned by UnScribeInit()
 *   \param fileptr The output file
 *   \return 0 for OK, non-zero on error. */
extern int UnScribeAbort(int  code, struct ScribeState  **refstate);
/**< Abort converting scribe text, freeing up processing state.
 *   \param code The return code from UnScribeInit()
 *   \param refstate The state structure returned by UnScribeInit()
 *   \return 0 for OK, non-zero on error. */
ENDCPLUSPLUSPROTOS
/** @} */

