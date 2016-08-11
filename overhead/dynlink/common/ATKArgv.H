/* Copyright 1994.  Carnegie Mellon University. All rights reserved.
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

// ATKArgv: a class for manipulating C style argument lists

#include <andrewos.h>

class ATKArgv {
  private:
    char **argv;
    int argc;
    int size;
    int iterator;
    int error;
  protected:
    // A utility function to count the number of arguments in a NULL terminated
    // argument list.
    static int CountArgs(char **args);   
  public:
    // Passed as a position this indicates the position just after the last argument.
    static const int End;
    
    // Passed as a count this indicates that all the elements at and after the given
    // position should be effected.
    static const int CountArguments;

    // Sets the error flag,  so client code can delay dealing with an error if desired.
    // This function will only set the error flag to a non-zero value.  There is
    // no way to clear the error flag.
    void SetError(int err) { if(error==0) error=err; }
    
    // If an error occurred during an Insertion or Deletion Error()
    // or if SetError was called with a non-zero value then return non-zero.
    int Error() { return error; }
    
    //  Inserts the first count strings pointed to by args into the list just before pos.
    // A count of Count (-1) indicates that the args list is NULL terminated, and all the arguments
    // should be inserted.
    // A pos of End (-1) indicates that the args should be inserted at the end of the list.
    // The strings and the array passed in may be freed after this call.
    // The error flag will be set if memory cannot be allocated for the new arguments.
    void InsertArguments(char **args, int pos=End, int count=CountArguments);
    
    // Inserts a single argument at pos
    inline void InsertArguments(char *arg, int pos=End) {
	InsertArguments(&arg, pos, 1);
    }
    
    // Deletes count arguments at pos.  The deleted arguments are freed and should not be
    // referenced after this call.  A call with no arguments will delete all the arguments in
    // the list.
    void DeleteArguments(int pos=0, int count=CountArguments);
    
    // Resets the iterator to begin at the first argument with the next call to NextArgument
    void ResetIterator() { iterator=0; }
    
    // Returns the next argument, the returned C string should not be referenced after a call
    // to DeleteArguments which may have deleted it.  NULL will be returned if all the
    // arguments have been returned since the last call to ResetIterator.
    // The effect of Insert and Delete on an iteration in progress is undefined.
    char *NextArgument() { return (iterator>=0 && iterator<argc)?argv[iterator++]:NULL; }
    
    // Returns the number of arguments in the Argv.
    int Count() { return argc; }

    // Returns a char **argv suitable for use in exec calls for example.
    // The returned vector and it elements should not be referenced after
    // the ATKArgv is modified or destroyed.  NULL is returned if
    // an error was encountered constructing or inserting any argument.
    char **Argv() { return error?NULL:argv; }

    // Returns a string suitable for execution via 'system'.  If quote is TRUE all
    // non-alphanumeric characters will be quoted. The string is stored in an
    // internal buffer which will be rewritten or possibly deallocated by the next
    // call to SystemString.  NULL will be returned if an error occurred building
    // the argument list or if memory for the string couldn't  be allocated.
    char *SystemString(boolean quote=TRUE);
    
    // Constructs an ATKArgv from an existing argv, optionally only taking the first count
    // arguments.
    ATKArgv(char **args, int count=CountArguments);
    
    // Constructs an ATKargv from a single argument.
    ATKArgv(char *arg);

    // Create an empty argument list.
    ATKArgv();
    
    ~ATKArgv();
};

