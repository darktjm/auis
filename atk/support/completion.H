ATK_INTER
#ifndef _completion_H_
#define _completion_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

/* complete.H
 * Class header for message line completion support module.
 */

#include <message.H>

struct result {
    char *partial;
    int partialLen; /* Length in chars not including terminating NUL */
    char *best;
    int max;
    int bestLen; /* Length in chars not including terminating NUL */
    enum message_CompletionCode code;
};

#include <ATK.H>

class completion : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        static void CompletionWork(const char *string, struct result *data);
        static int GetFilename(class view *view, const char *prompt, const char *startPath, char *buffer, long bufsiz, boolean directoryP, boolean mustMatch)  ;
        static long FindCommon(const char *string1, const char *string2)  ;
        
	static boolean InitializeClass();

	static void FileHelp(const char *partialPath, long dummyData, message_workfptr helpTextFunction, long helpTextRock);
        static enum message_CompletionCode FileComplete(char *pathname, boolean directory, char *buffer, int bufferSize)  ;
};

#endif /* _completion_H_ */
