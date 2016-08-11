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


 

class bufferlist : observable[observe] {
    overrides:
	ObservedChanged (struct observable *changed, long value);
    methods:
        AddBuffer(struct buffer *buffer);
        RemoveBuffer(struct buffer *buffer);
        Enumerate(procedure mapFunction, long functionData) returns struct buffer *;
        CreateBuffer(char *bufferName, char *fileName, char *objetName, struct dataobject *data) returns struct buffer *;
	
/* Lookup functions */
        FindBufferByName(char *bufferName) returns struct buffer *;
        FindBufferByFile(char *filename) returns struct buffer *;
        FindBufferByData(struct dataobject *data) returns struct buffer *;

/* File functions. */
        GetBufferOnFile(char *filename, long flags) returns struct buffer *;
        GuessBufferName( char *filename, char *bufferName, int nameSize);
        GetUniqueBufferName( char *proposedBufferName, char *bufferName, int nameSize);
        SetDefaultObject(char *objectname);
    classprocedures:
        FinalizeObject(struct bufferlist *self);
        InitializeObject(struct bufferlist *self) returns boolean;
    data:
	struct listentry *head;
};

