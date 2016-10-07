/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* attribs.h
 * Declartion of a property list like structure for data object attribute
 * propagation. Used by the filetype module and the dataobject_SetAttributes
 * Method.
 */

#ifndef ATTRIBUTES_DEFINED

#define ATTRIBUTES_DEFINED

#include <atkproto.h>
BEGINCPLUSPLUSPROTOS
/** Generic attribute for use with dataobject::SetAttributes() */
struct attributes {
    struct attributes *next;  /**< Representation is a linked list. */
    const char *key;          /**< Attribute name. */
    union {
        long integer;         /**< Integer value. */
        const char *string;   /**< String value. */
    } value;  /**< Value can either be an integer or a string. */
};
ENDCPLUSPLUSPROTOS
/** @} */
#endif /* ATTRIBUTES_DEFINED */
