#ifndef _dataobject_H_
#define _dataobject_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <atom.H>
#include <namespace.H>
#include <stdio.h>

#include <observable.H>

/** Maintain, store, and manipulate information.
 *  An ATK inset consists of a dataobject and a view to display that
 *  dataobject.  While not technically an abstract class, it is not meant
 *  to be instantiated directly.  For help in creating a new dataobject-derived
 *  class, use createinset for sample code.
 *
 *  Data objects are used to represent permanent storage in the Andrew Toolkit.
 *  They can be read or written to files, and also provide private operations
 *  that make sense only for a particular set of data objects.  Views usually
 *  represent views onto data objects; in other words, when a view is created,
 *  it is usually associated with a  particular data object.  In normal use,
 *  data objects are used to represent the graphical or textual entities one
 *  edits.  These entities are usually stored in files for permanent storage,
 *  although other uses of data objects include, for example, the message line
 *  in an editor window, which is not a permanent object.
 *
 *  Each object has two sets of named parameters it can deal with:
 *  attributes and properties.  Attributes may be set at any time, and
 *  are expected to be acted upon immediately.  The default class does
 *  nothing with them; a derived class must override SetAttributes() to
 *  use them.  Properties may also be set at any time, but they are stored
 *  for later use.  The storage and retrieval of properties is implemented
 *  by the Put() and Get() methods in this class.  Usage of properties is
 *  generally scattered among other methods which need them.
 *
 *  The information in a data object is displayed through views onto that
 *  data object, and any one data object may have more than one view onto
 *  it at any given time.  If the data object is changed, then all the
 *  views are updated automatically.  Data objects are necessary in two
 *  cases:  first, if you want to be able to write data to a file, and
 *  second, if you want to have several different views on a single piece
 *  of data, for example, if you want both a table and a graph to display
 *  the same data.
 *
 *  For permanent storage, the data object is expected to write an ASCII
 *  <em>data stream</em>.  This format is used for any transmission of
 *  this data; it is the official serialization format for ATK.
 *
 *  The data stream is represented as a collection of data object definitions
 *  and views that refer to those data objects.  Data objects can be included
 *  in a file using the construct:
 *
 *  <pre><code>
 *     \\begindata{<em>datatype</em>, <em>dataID</em>}
 *         <em>&lt;data object data&gt;</em>
 *     \\enddata{<em>datatyp</em>e, <em>dataID</em>}
 *  </code></pre>
 *
 *  The \c \\begindata, \c \\enddata pair mark the extent of the data
 *  included in a particular data object.  The \e datatype is the type of
 *  data being stored (e.g. text, raster, etc).  The \e dataID is a
 *  unique ID number that identifies the particular data object.  No two data
 *  objects in the same file/stream will have the same \e dataID;
 *  however, two data objects in different streams may coincidentally have the
 *  same ID number.
 *
 *  Note that there are newlines after the \c \\begindata statement,
 *  after the data object dependent data, and after the \e \\enddata
 *  statement.  These newlines make data parsing easier.  However, few
 *  ATK objects conform to this convention.  Code allows \\begindata to
 *  appear in the middle of a line, and the terminating newline of the
 *  \\enddata to be optional.
 *
 *  For data streams, the characters \\, {, and } are special.  If these
 *  characters are to appear in the data stream they must be quoted.  Thus
 *  \\\\, \\{, and \\} produce literal occurrences of \\, {, and },
 *  respectively.  In non-data stream files, no special significance is
 *  attached to the reserved characters, and literal occurrences should not
 *  be preceded by an extra \\.
 */

class dataobject: public  observable {
public:
    virtual ATKregistryEntry *ATKregistry();
    dataobject(); /**< Default constructor.  Do not create dataobject objects. */
    virtual ~dataobject();

    /** Return codes from Read() */
    enum read_return {
	NOREADERROR, /**< A successful read operation. */
        PREMATUREEOF, /**< EOF found before \\enddata */
        NOTATKDATASTREAM, /**< No properly formatted \\begindata at start, or unrecognizable raw data. */
        MISSINGENDDATAMARKER, /**< No \\enddata after correctly formatted data */
        OBJECTCREATIONFAILED, /**< Error creating embedded object. */
        BADFORMAT /**< Incorrectly formatted data in stream (parse error). */
    };

    virtual long Read (FILE *file, long id);
    /**< Read data stream representation of this object from a file.
     *   This routine is used both to read embedded streams and entire
     *   files.  In the former case, this is called with the file pointer
     *   after the newline following the \\begindata statement, and the
     *   ID from that statement (always non-zero) is passed in as \e id.
     *   In the latter case, \e id is zero, and the \\begindata statement
     *   must be parsed first.  This also allows for the possibility of
     *   supporting non-datastream files if the file pointer is not on
     *   a \\begindata statement.  A successful read terminates at the
     *   end of the stream.  For data in non-stream format, this is
     *   dependent on the stream format, but is usually the end of the
     *   file.  For data in stream format, this is the point after the
     *   \\enddata statement, including the terminating newline, if present.
     *   Upon encountering \\begindata at a supported location for nested
     *   insets, this routine should create a new object of the type named
     *   in that statement, extract the ID, and call Read for the new
     *   object to read the nested inset.
     *   \param file The file to read from.  Do not close this file.
     *   \param id The data ID supplied in the stream's \\begindata.
     *             You should verify that the \\enddata's datatype is
     *             this type and that its data ID is \e id.  If this is
     *             0, no \\begindata has been parsed yet, and should
     *             be parsed by this routine, unless non-stream data
     *             formats are supported, in which case that data should
     *             be read instead.
     *   \return A status code, from dataobject::read_return.
     */
    virtual long Write (FILE *file, long writeid, int level);
    /**< Write data stream representation of this object to a file.
     *   This may perform one of three functions:
     *
     *   It may do nothing.  If this has been called with the same
     *   \e writeid before, it has already been written to the file.
     *   Use GetWriteID() to verify this, and SetWriteID() to update if
     *   this is not the case.
     *
     *   It may write its contents in a non-stream format.  If
     *   \e level is 0, no stream components have been written yet, so
     *   it is permissible to write the contents in a recognizable
     *   object-specific format.  This format must also be readable using
     *   Read() when it is passed a zero data ID.  The file stream pointer
     *   should be left at the end of the file.
     *
     *   It may write a data stream.  This must begin with
     *   \c \\begindata{<em>class</em>,<em>ID</em>} on a line by itself,
     *   where \e class is the registered ATK class name of this class,
     *   and \e ID is a unique ID for this object instance, and may also
     *   include leading whitespace.  The UniqueID() and GetID()/SetID()
     *   methods can assist in generating an ID.  The stream must end with
     *   \c \\enddata{<em>class</em>,<em>ID</em>} on a line by itself.
     *   The parameters must match those in the \\begindata statement.
     *   The actual contents between these two are defined by the data
     *   object, but they must not contain \\enddata preceded by an even
     *   number of (or no) backslashes.  This is so that streams containing
     *   data objects currently unrecognized by the system can skip these
     *   unknown objects, retaining their data for future use.
     *   Well-formed data streams should be pure ASCII, with short lines
     *   (76 characters), and with all occurrences of \\, {, and } escaped
     *   with \\.
     *
     *   Note that no provision is made to signal failure except via the
     *   error indication on the file pointer.
     *   \param file The file to write to.  Do not close this.
     *   \param writeid A unique ID generated for a combined stream.  Do not
     *                  write anything if this ID has not changed since the
     *                  last save.  It is not necessary to remember every
     *                  ID used in the past, though.
     *   \param level The nesting depth of this object in the stream (0 means
     *                it is the top-level object).  This must be incremented
     *                when calling Write() for embedded objects.
     *   \return The ID passed into \c \\begindata and \\enddata.  This
     *           will be used with embedded objects to match the object
     *           with views on that object.  Note that even if \e writeid
     *           indicates that no data is to be written, this should be
     *           the ID that was used when last written.
     */

    /** Formats supported by ReadOtherFormat()/WriteOtherFormat() */
    enum otherformat {
	OTHERFORMAT_MAIL = 1
	/**< Save/Read data in MIME format.
	 *   The mime-type for the data should be either the raw type (e.g.
	 *   text/plain for plain text), in which case raw data should be
	 *   encoded and written, or application/andrew-inset, in which case
	 *   the ATK data stream should be encoded and written.  The output
	 *   from WriteOtherFormat() should start with a newline, followed by:
	 *   <pre><code>
	 *     --<em>boundary</em>
	 *     Content-type: <em>mime/type</em>
	 *     Content-Transfer-Encoding: <em>encoding</em>
	 *   </code></pre>
	 *   The \e encoding is usually one of \c quoted-printable or
	 *   \c base64.  This header is followed by another newline and the
	 *   encoded data.
	 */
    };
    
    virtual boolean ReadOtherFormat(FILE *file, const char *fmt, const char *encoding, const char *description);
    /**< Read object in alternate format.
     *   Reads the object in a format other than the ATK stream format used
     *   by Read().  Note that it is impossible to even notify this function
     *   of the format, so it only supports MIME.
     *   \param file The input file.  Do not close.
     *   \param fmt The MIME type (e.g. "text/plain")
     *   \param encoding The MIME encoding (e.g. base64 or quoted-printable)
     *   \param description The MIME description (from content-disposition)
     *   \return TRUE if the read was successful; FALSE otherwise. */
    virtual long WriteOtherFormat(FILE *file, long writeid, int level, otherformat usagetype, const char *boundary);
    /**< Write object in alternate format.
     *   Writes the object in a format other than the ATK stream format used
     *   by Write().  Currently, all implementations of this method simply
     *   ignore the requested format and only support MIME.
     *
     *    Note that no provision is made to signal failure except via the
     *    error indication on the file pointer and the return of 0 indicating
     *    lack of support for this format.
     *    \param file The output file.  Do not close.
     *    \param writeid The write ID.  See Write() for details.
     *    \param level The level.  See Write() for details.
     *    \param usagetype The format to write.  Currently, this must be OTHERFORMAT_MAIL.
     *    \param boundary The MIME boundary string.
     *    \return The unique object ID if this format is supported.  Otherwise, 0.
     */

    virtual long GetModified();
    /**< See if this object has been modified.
     *   This is meant to be compared to a previous reading.  If they are
     *   equal, then there have been no modifications since the previous
     *   reading.  Otherwise, there has been a modification.
     *   \return Current modification sequence counter. */
    virtual void SetModified();
    /**< Claim that this object has been modified.
     *   Every time the object's stream represenetation changes,
     *   this should be called.  It increments a revision counter. */
    virtual void RestoreModified(long oldmodified);
    /**< Revert modification state to previous value.
     *   Some modifications are temporary.  Such modifications can be
     *   reverted by passing the GetModified() result before such
     *   modifications to this method.  However, the results may not
     *   be as expected, as other objects modified as a side effect may
     *   become confused.  This function is not officially supported.
     *   \param oldmodified A previous modificaiton sequence to restore. */
    virtual const char * ViewName();
    /**< Find a view appropriate for the data.
     *   This returns the default view for viewing this object.
     *   It returns the first existing class name which is a descendent of
     *   "view" obtained by appending \c view, \c v, \c View and \c V to
     *   this object's class name; if none of these exist, it returns "view".
     *   If the view object has not yet been loaded, this function will load
     *   and initialize it.
     *   \return The name of an ATK class for viewing this object.
     */
    virtual void SetAttributes(struct attributes *attributes);
    /**< Set named object attributes.
     *   An attribute is a named value; it can be either a string or an
     *   integer (the attribute name defines the type).  The supported
     *   attributes are object-dependent:  an object is free to ignore
     *   unused attributes.  The following attributes are "standard" in
     *   that they are generally set after loading an object, as returned by
     *   filetype::Lookup():
     *   - filename - A string giving the name of the file loaded into the
     *                object.
     *   - extension - If the filename is valid, this string is the set of
     *                 characters after the last dot.
     *   - filesize - An integer giving the size of the file loaded into the
     *                object, if known.
     *   - readonly - An integer which, if non-zero, indicates that the
     *                object should behave as if read-only.  This is not only
     *                a reflection of the file's writability, but also
     *                a flag which may be toggled by the user.
     *
     *   This function must be overridden to process any attributes,
     *   including the "standard" ones.  The list should be passed down
     *   to the parent class, in case it is interested in any.
     *
     *   \param attributes A linked list of attributes to set for the object.
     */
    virtual void Put( const class atom * property, const class atom * type, long rock );
    /**< Set named properties of object.
     *   This adds a property with the given name, type, and value to the
     *   object's property list.
     *   \param property The property name.
     *   \param type The property's type name.
     *   \param rock The property's value.  The interpretation of this value
     *               is generally depdent on the type. */
    virtual short Get( const class atom * property, const class atom ** type, long * rock );
    /**< Retrieve named properties of object.
     *   This obtains the last type and value set for a given named property.
     *   \param property The property name.
     *   \param type The property's type name is returned here.
     *   \param rock The property's value is returned here.
     *   \return TRUE if the property was found; FALSE otherwise. */
    virtual int ListCurrentViews(class view **array,int size);
    /**< Retrieve views currently watching this object.
     *   \param array An array to fill in with view objects.  If there is
     *                enough room after the list of views, it will be
     *                NULL-terminated.
     *   \param size The size of the array.
     *   \return the number of current views.  If this is more than \e size,
     *           only \e size elements are returned.  */

    inline long UniqueID() { return uniqueid; }
    /**< Return a unique long integer.
     *   This integer is guaranteed to be different, but constant for every
     *   object that calls this.
     *   \return An integer guaranteed to be unique to this object. */
    static const long UNDEFINEDID = -1; /**< A special WriteID value saying the object has never been written. */
    inline long GetWriteID() { return ( (this)->writeID ); }
    /**< Return the object's last write ID.
     *   \return The last ID set by SetWriteID(), or UNDEFINEDID if never set. */
    inline void SetWriteID(long newValue) { ( ((this)->writeID) = (newValue) ); }
    /**< Set the object's last write ID.
     *   This should be called every time Write() is called to update the
     *   last time the object was written.
     *   \param newValue The new write ID. */
    inline long GetID()      { return ( (this)->id ); }
    /**< Return the object's unique ID.
     *   This is the ID that is written to a data stream to distinguish
     *   between data objects.
     *   \return An integer unique to this object. */
    void SetID(long newValue);
    /**< Set the object's unique ID.
     *   This is the ID that is written to a data stream to distinguish
     *   between data objects.  It is UniqueID() by default.  Changing this
     *   to something which matches another object's GetID() has
     *   unpredictable consequences.
     *   \param newValue The ID to assign. */

  private:
    long id, uniqueid;
    long writeID;
    long modified;
    class Namespace * properties;
};
/** @} */
#endif /* _dataobject_H_ */
