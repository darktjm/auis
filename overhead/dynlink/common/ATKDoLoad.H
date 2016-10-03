/* Copyright 1994 Carnegie Mellon University All rights reserved. */
/** \addtogroup libATKDynLoad libATKDynLoad
 * This library is used to load dynamic ATK objects.
 * @{ */
extern "C" int (*ATKDoLoad(const char *path, int trace))(int argc, char **argv);
/**< Load a dynamic ATK class object file into memory.
 *   \param path The file to load
 *   \param trace Non-zero to maybe print additional debug information
 *   \return A function to call to initialize object.  For systems which
 *           automatically initialize dynamic objects, this may be a dummy
 *           function which does nothing. */
/** @} */

