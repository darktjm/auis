#ifndef _graphic_H_
#define _graphic_H_

/** \addtogroup libbasics Object basics
 * @{ */
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <point.h>
#include <rect.h>
#include <shadows.h>
#include <observable.H>
#include <color.H>

class ddimage;
/** Generic graphic context.
 *  This context is generally associated with a view; it is where
 *  all things are drawn.  Both the context and the methods to use this
 *  context to draw graphics are held here.  This is mostly an abstract
 *  class; children are created specific to the type of graphics device
 *  (e.g. \ref xgraphic).
 *
 *  The graphic context contains the following elements:
 *  -  A rectangle of bits representing pixels on the display.  The
 *     mapping of bits to pixels is set up by the interaction manager
 *     (\ref im) using the \ref pixmap class.  This defines the
 *     context's <em>visual rectangle</em>.  A new grpahic object must
 *     be associated with real pixels either by calling im::SetView or
 *     graphic::InsertGraphic.
 *  -  A logical coordinate system is set up on a virtual plane of pixels.
 *     Its origin coincides with the visual rectangle.  Its size is
 *     effectively infinite, but it is clipped to the visual rectangle for
 *     display and storage.
 *  -  An enclosing transformation from the logical coordinate system to
 *     the enclosing context's coordinate system.
 *  -  A drawing cursor.  This is an invisible point used by other
 *     drawing functions as a point of origin.  This is currently only used
 *     by a limited set of line drawing functions and the text drawing
 *     function.  Officially, all other functions ignore and invalidate the
 *     drawing cursor.  In addition, resizing the graphic context invalidates
 *     the drawing cursor.  Its initial position is (0,0).
 *  -  A drawing color.  The interpretation of this color is dependent on
 *     the display hardware.  This includes a foreground color and a
 *     background color.
 *  -  A line width.  Solid lines and curves are extended from their
 *     nominal path perpendicular to the path by 1/2 the line width in
 *     either direction.  A line width of 0 is a special degenerate, fast
 *     version of 1.  Its initial value is 1.
 *  -  A line drawing pattern.  This includes a method (\ref LineDash)
 *     to draw alternating segments, and a pattern of alternating segments,
 *     and a "phase" adustments to make continued lines' patterns line up.
 *  -  A tiling pattern offset.  This is used with the tiling pattern to
 *     properly align multiple regions' patterns.
 *  -  Line end point drawing techniques (\ref LineCap, \ref LineJoin).
 *  -  A text font.  The default is andysans 12.
 *  -  Text spacing.  After each space, additional pixels may be skipped
 *     via a "shim".
 *  -  A transfer function.  This is a set of bits.  This set is indexed
 *     by a 3-bit number.  The lowest bit of that number is the destination
 *     bit.  The second is the source bit.  The third is the mask bit.
 *     For example, if destination, source, and mask bits are 0, 1, and
 *     0, respectively, bit number 010<sub>2</sub> (2) is the final value,
 *     generally placed at the destination.  Note that the mask bit is
 *     currently unused and undefined, so it is safest to construct
 *     transfer modes with the first 4 bits repeated in the upper bits.
 *     In addition to the masks, further functionality may be added to
 *     this, so it is expected to be stored in a short (16-bit) integer.
 *     To help construct the desired function, two constants,
 *     graphic::SOURCE and graphic::DEST, have been defined that allow a
 *     programmer to specify the desired effects without having to do
 *     the bit-by-bit calculations.  Instead, one can just perform the
 *     logical operations directly on the constants. For example, the
 *     AND operation can be expressed as (graphic::SOURCE & graphic::DEST).
 *     Some common combinations are provided as well.  The default value
 *     is graphic::COPY.
 *  -  A clip region.  This can be used to reduce the number of drawing
 *     commands.  The drawing will be clipped to this, regardless.  The
 *     default is NULL, i.e., no clipping region.
 */
class graphic : public  observable {
public:
    TRACED_CLASS(graphic);

    static const int PROGRAMMERVERSION = 7;

/* Some notes on the structure:
    The attached graphic provides some bits to write on. Nothing is
assumed initially about the relationship between bounds in a graphic
and graphic bounds. A graphic is associated witha graphic in one of two
ways. First, a graphic can be directly laid on top of
graphic (Setgraphic), in which case the call explicitly links the portion 
of graphic to the graphic. Second, a graphic can be inserted into
another graphic, in which case the graphic information is gleaned
relative to the parent. Either case is very system dependent. One 
assumes that only im will do the former, and that everyone else will
do the latter which will change with each machine, but the change
should be invisible to the user.

The local bounds specify, in local graphic coordinates, what the
coorindate system is for this graphic. Initially this is 0,0 in upper
left corner, though it can be changed by moving the origin explicitly.
The visual rectangle shows
which part of the local bounds is actually displayed on the screen
(also in local coordinates). The enclosedRect is the same as the local
bounds rectangle except that it is expressed in the parent's coordinate
space (if any exists). Like the physical rectangle of the graphic, the
width and height information are redundant with the width and height of
the localbounds.

 If any mapping between local coordinates
and underlying window manager coordinates is necessary, the window
manager system will take care of it (state will be part of graphic).

graphic will contain the window manager depedent information, such as 
window id number. The fontdesc is the font descriptor to be used for
diplaying text. When a descriptior was created, a size, famliy and
style was specified, this to change to bold or larger font, a 
new descirptor must be set.

The transfer mode explains how bits will be pasted on the screen when
drawing lines, writing text, blitting bits or filling figures. 
Essentially the same defintion as for wm is used, except the tiling mask
is available only for bitblitting.

The line width is self-explanatory. The current point is used to 
determine where to draw lines or text. The clipping rectangle is a user
controllable way to limit drawing commands. It is in local coordinates,
but is usually null (no explicit clipping).

*/

/* Transfer function bit encoding. One specifies the transfer function
   by giving a table source and destination bits, encoded in a hex digit.
   The binary interpretation is that 1 is black and 0 white.
   The table encodes bits as follows:
	Bit 3   Bit 2   Bit 1   Bit 0
Source	    1(B)    1(B)    0(W)    0(W)
Dest	    1(B)    0(W)    1(B)    0(W)
Result	    ?	    ?	    ?	    ?
Thus, Bit 3 specifies what the result should be if both the source and
destination are 1; bit 2 specifies the result if the source is 1 and the
destination 0; bit 1, when the source is 0 and the destination 1; and
bit 0 when both the source and destination are 0. For example, to
generate the AND function, bit 3 would be one and bits 2, 1 and 0 would
be 0, so the resulting hex digit would be 0x4. If we want the source to
be copied directly without regard to the current value of the 
destination, bits 3 and 2 would be 1 and bits 1 and 0 would be 0,giving
the hex digit 0xc.

Because we believe that we might expand our transfer function to
three inputs (using a mask as a third input), we now require the transfer
function be two hexadecimal digits -- the one generated as above 
repeated. Thus, the value for AND would be 0x44 and for copying the
source, 0xcc.

To help construct the desired function, two constants graphic::SOURCE
and graphic::DEST have been defined that allow a programmer to specify
the desired effects without having to do the bit by bit calculations.
Instead, one can just perform the logical operations directly on
the constants. For example, the AND operation can be expressed as
(graphic::SOURCE & graphic::DEST). Some common combinations are provided
as well.
*/

    /* Predefined constants for use in transfer modes */
    static const short SOURCE = 0xCC; /**< Transfer function to transfer source without regard to destination. */
    static const short DEST = 0xAA; /**< Transfer function to transfer destination without regard to source. */
    /* This doesn't do anything useful but take up namespace right now. */
    static const short MASK = 0xF0; /**< Transfer function to transfer mask. Unimplemented; do not use.  */

    /* Some synonyms for ease of use */
    static const short COPY =	SOURCE; /**< Transfer function to transfer source directly into destination. */
    static const short OR =	(SOURCE | DEST); /**< Transfer function to OR the source into the destniation. */
    static const short XOR =	(SOURCE ^ DEST); /**< Transfer function to XOR the source into the destination. */
    static const short AND =	(SOURCE & DEST); /**< Transfer function to AND the source into the destination. */
    static const short WHITE =	0x00; /**< Transfer function to always render in white, regardless of the source or destination. */
    static const short BLACK =	0xFF; /**< Transfer function to always render in black, regardless of the source or destination. */
    static const short INVERT =	((~DEST)&0xFF); /**< Transfer function to invert the destination, regardless of the source. */

    /* These don't really do anything useful, since MASK is undefined. */
    static const short COPY_MASK = ((SOURCE & MASK) | (DEST & ~MASK));
       /**< Transfer function to only transfer source if mask is set.  Unimplemented; do not use. */
    static const short COPY_NOTMASK = ((SOURCE & ~MASK) | (DEST & MASK));
       /**< Transfer function to only transfer source if mask is not set.  Unimplemented; do not use. */

    /** When a position may be specified relative to boundaries,
     *  or one ore more of these together to indicate where. */
    enum position {
	NOMOVEMENT = 0, /**< Use by itself: position is absolute.  */
	ATLEFT = 1, /**< Position at left edge */
	ATRIGHT = 2, /**< Position at right edge */
	BETWEENLEFTANDRIGHT = 4, /**< Position centered left-to-right */
	ATTOP = 010, /**< Position at top edge */
	ATBOTTOM = 020, /**< Position at bottom edge */
	ATBASELINE = 040, /**< Position at bottom edge, adjusted upwards for text baseline */
	BETWEENTOPANDBOTTOM = 0100, /**< Position centered between top and bottom */
	BETWEENTOPANDBASELINE = 0200 /**< Position centered between top and bottom, adjusting downwards for text baseline */
    };

    /** Fixed line dash types are specified by one of these.
     *  They correspond directly with the X line styles of the same name.
     *  See XCreateGC(3) for details.  */
    enum LineDash {
	LineSolid, /**< Solid line. */
	LineDoubleDash, /**< Dashes in alternating fill styles. */
	LineOnOffDash /**< Solid dashes alternating with nothing */
    };

    /** Line end appearance is specified by one of these.
     *  They correspond directly with the X cap styles of the same name.
     *  See XCreateGC(3) for details.  */
    enum LineCap {
	CapNotLast, /**< Same as \ref CapButt, but last endpoint is not drawn if width is 0. */
	CapButt, /**< Line stops at end point and is strictly rectangular, centered at the end point. */
	CapRound, /**< In addition to CapButt behavior, a semicircle is extended beyond the line end centered at the end point if the line width is greater than 0. */
	CapProjecting /**< Like \ref CapButt, but the rectangle extends beyond the end points by 1/2 the line width. */
    };

    /** For multi-line drawing, such as polyline or polygons, the joints
     *  between segments are specified by one of these.  The correspond
     *  directly with the X join styles of the same name.  See
     *  XCreateGC(3) for details.  */
    enum LineJoin {
	JoinMiter, /**< For angles of 11 degrees or more, outer edges of lines extend until they meet, with the trapezoidal area defined by these and the line end edges filled.  Otherwise, like JoinBevel. */
	JoinRound, /**< The joint is covered by a circle whose diameter is the line width. */
	JoinBevel /**< The outer edges of the lines are joined by a single line segment, with the triangular area defined by that and the line ends filled. */
    };

    /** Display class.  These correspond directly with the X display classes
     *  of the same name.  See the Xlib - C Language X Interface, chapter 3,
     *  "Window Functions" for details. */
    enum DisplayClass {
	Monochrome = (1<<1), /**< A \ref StaticGray display with 2 colors has this ored in. */
	Color = (1<<2), /**< Any color display has this ored in. */
	PseudoColor = (1<<3), /**< Color display; User-modifyable color table maps pixel values to colors.  This used to be the most common display type:  a limited number of bits per pixel (e.g. 8) with a full RGB palette. */
	GrayScale = (1<<4), /**< Monochrome display; Pixel values map to user-modifyable gray scale.   I'm not sure I've ever seen such a display.  */
	DirectColor = (1<<5), /**< Color display; RGB values map to user-modifyable RGB levels.  I'm not sure I've ever seen such a display.  */
	TrueColor = (1<<6), /**< Color display; RGB values map to fixed RGB levels.  This is the most common modern display. */
	StaticColor = (1<<7), /**< Color display; Fixed color table maps pixel values to colors.  Old color diplays with limited color selections, such as 8-bit personal computers, have this kind of display. */
	StaticGray = (1<<8) /**< Monochrome display; Pixel values map to fixed gray scale.   This includes 1-bit displays (once common) and multi-bit monochrome displays (e.g. NeXT). */
    };

    virtual void MoveToPt(const struct point * Pt);
    /**< Move drawing cursor to given point.
     *   \param Pt The position relative to the graphic context origin.
     */
    virtual void MoveTo(long NewX, long NewY);
    /**< Move drawing cursor to given coordinates.
     *   \param NewX The X coordinate relative to the graphic context origin.
     *   \param NewY The Y coordinate relative to the graphic context origin.
     */
    virtual void Move(long DeltaX, long DeltaY);
    /**< Move drawing cursor.
     *   \param DeltaX The X coordinate relative to the current location.
     *   \param DeltaY The Y coordinate relative to the current location.
     */
    virtual void GetCurrentPt(struct point * Pt);
    /**< Retrieve the current drawing cursor position.
     *   \param Pt Filled with the position relative to the graphic context
     *             origin.
     */
    virtual void DrawLineToPt(const struct point * LineEnd);
    /**< Draw a line to a point.
     *   The line starts at the current drawing cursor, using the current
     *   line drawing styles.  The drawing cursor is also moved to the
     *   end point of the line.
     *   \param LineEnd The end point, relative to the graphic context origin.
     */
    virtual void DrawLineTo(long XEnd, long YEnd);
    /**< Draw a line to given coordinates.
     *   The line starts at the current drawing cursor, using the current
     *   line drawing styles.  The drawing cursor is also moved to the
     *   end point of the line.
     *   \param XEnd The X coordinate of the line end relative to the graphic
     *               context origin.
     *   \param YEnd The Y coordinate of the line end relative to the graphic
     *               context origin.
     */
    virtual void DrawLine(long DeltaX, long DeltaY);
    /**< Draw a line to given offset from cursor.
     *   The line starts at the current drawing cursor, using the current
     *   line drawing styles.  The drawing cursor is also moved to the
     *   end point of the line.
     *   \param DeltaX The X coordinate of the line end relative to the drawing
     *                 cursor.
     *   \param DeltaY The Y coordinate of the line end relative to the drawing
     *                 cursor.
     */

    virtual void DrawString(const char * Text, short Operation);
    /**< Draw text given by C string.
     *   The text is 0-terminated.  The text is displayed left-to-right,
     *   using the current drawing styles.  The drawing cursor is used
     *   as a point of origin, but is not moved after drawing the text.
     *   \param Text The string to display.
     *   \param Operation Where to draw the text relative to the drawing
     *          cursor.  The default vertical position is the baseline,
     *          and the default horizontal position is the left edge.
     *          Use a combination of graphic::position constants.
     */
    virtual void DrawText(const char * Text, long TextLength, short Operation);
    /**< Draw text characters.
     *   The text may contain zeroes.  The text is displayed left-to-right,
     *   using the current drawing styles.  The drawing cursor is used
     *   as a point of origin, but is not moved after drawing the text.
     *   \param Text The characters to display.
     *   \param TextLength How many characters to display.
     *   \param Operation Where to draw the text relative to the drawing
     *          cursor.  The default vertical position is the baseline,
     *          and the default horizontal position is the left edge.
     *          Use a combination of graphic::position constants.
     */
    virtual void SetSpaceShim(short Amount);
    /**< Set the width of spaces in drawn text.
     *   This width is in pixels.  Spaces in text are followed by a skip of
     *   this length before continuing.  This does not apply to tabs (which
     *   are generally not directly supported,
     *   anyway).
     *   \param Amount Additional width of a space.  Default is 0.
     */
    virtual short GetSpaceShim()  ;
    /**< Find out how much additional space is drawn in text.
     *   \return The amount skipped after each space.
     */
    virtual void SetFont(class fontdesc * ChosenFont);
    /**< Set the font for text rendering.
     *   This includes styles, such as bold and italic.
     *   \param ChosenFont The font to use.  The default is andysans12.
     *                     The actual font used depends on display
     *                     characteristics and available font resolutions,
     *                     so font metrics may need updating if these
     *                     characteristics change.
     */
    virtual class fontdesc * GetFont()  ;
    /**< Find out what font is being used for text rendering.
     *   \return the font.
     */

    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawRect(const struct rectangle * Rect);
    /**< Draw a rectangle.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.  Note that line thickness greater than zero
     *   may affect the actual boundaries of the rectangle; the specified
     *   coordinates are the centers of the line end points.  Only the
     *   edges are drawn.
     *   \param Rect The rectangle size and coordinates relative to the
     *               graphic context origin.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawRectSize(long x,long y,long width,long height);
    /**< Draw a rectangle at the given position and size.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.  Note that line thickness greater than zero
     *   may affect the actual boundaries of the rectangle; the specified
     *   coordinates are the centers of the line end points.  Only the
     *   edges are drawn.
     *   \param x The X coordinate of the rectangle's left edge relative
     *            to the graphic context origin.
     *   \param y The Y coordinate of the rectangle's top edge relative
     *            to the graphic context origin.
     *   \param width The nominal width of the rectangle, in pixels.  Negative
     *                values draw nothing.
     *   \param height The nominal height of the rectangle, in pixels.
     *                 Negative values draw nothing.
     */
    // tjm - PointCount should be unsigned.  This should also fail gracefully
    //       if PointArray is NULL or PointCount is 0.
    virtual void DrawPolygon(const struct point * PointArray, short PointCount);
    /**< Draw a set of lines in a closed path.
     *   Starting at the first given point, line segments are drawn as
     *   per DrawLineTo() except that the drawing cursor is not read or
     *   affected, and end points are drawn with the join style rather than
     *   the endcap style.  The first element of the array is both the
     *   starting position of the first line and the end position of the
     *   last line.  The remaining array elements are the end of the previous
     *   line and the start of the next.  Pixels are never drawn more than
     *   once if the line width is more than zero.
     *   \param PointArray The list of points.
     *   \param PointCount The number of elements in \p PointArray.  A
     *                     value less than one produces undefined behavior.
     */
    virtual void DrawTrapezoid(long topX, long topY, long topWidth, 
		long bottomX, long bottomY, long bottomWidth);
    /**< Draw a closed figure with parallel horizontal edges.
     *   This ignores and invalidates the current drawing cursor position.
     *   Negative widths move in the opposite direction.  The "left" edges
     *   of the horizontal lines are joined together, as are the "right" edges.
     *   Only the edges are drawn.
     *   \param topX The X coordinate of the "left" edge of the top horizontal line.
     *   \param topY The Y coordinate of the "left" edge of the top horizontal line.
     *   \param topWidth The width of the top horizontal line.
     *   \param bottomX The X coordinate of the "left" edge of the bottom horizontal line.
     *   \param bottomY The Y coordinate of the "left" edge of the bottom horizontal line.
     *   \param bottomWidth The width of the bottom horizontal line.
     */
    virtual void DrawPath(const struct point * PointArray, short PointCount);
    /**< Draw a set of lines in a path.
     *   Starting at the first given point, line segments are drawn as
     *   per DrawLineTo() except that the drawing cursor is not read or
     *   affected, and intermediate end points are drawn with the join
     *   style rather than the endcap style.  The first element of the
     *   array is only the starting position of the first line.  The
     *   remaining array elements are the end of the previous line and
     *   the start of the next, if present.  Pixels are never drawn more
     *   than once if the line width is more than zero.  If the last point
     *   coincides with the first point, the join style is used there.
     *   Otherwise, the first and last points use the endcap style.
     *   \param PointArray The list of points.
     *   \param PointCount The number of elements in \p PointArray.  A
     *                     value less than two produces undefined behavior.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawOval(const struct rectangle * Rect);
    /**< Draws an oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   ellipse may not look as intended.  Only the edge is drawn.
     *   \param Rect The oval's horizontal axis will be the rectangle's width,
     *               and its vertical axis will be the rectangle's height.
     *               Its center is the rectangle's center.
     *               Negative width and height produce undefined results.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawOvalSize(long x,long y,long width,long height);
    /**< Draws an oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   ellipse may not look as intended.  Only the edge is drawn.
     *   \param x The X coordinate of the center of the ellipse will be
     *            \p x + \p width / 2, relative to the graphic context origin.
     *   \param y The Y coordinate of the center of the ellipse will be
     *            \p y + \p height / 2, relative to the graphic context origin.
     *   \param width The oval's horizontal axis length.  Negative width
     *                produces undefined results.
     *   \param height The oval's vertical axis length.  Negative height
     *                 produces undefined results.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawArc(const struct rectangle * EnclRect, short StartAngle, short OffsetAngle);
    /**< Draws part of an oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   elliptical arc may not look as intended.  Angles begin at 12 o'clock
     *   and go clockwise if positive, and counter-clockwise otherwise.
     *   \param EnclRect The oval's horizontal axis will be the rectangle's width,
     *                   and its vertical axis will be the rectangle's height.
     *                   Its center is the rectangle's center.
     *                   Negative width and height produce undefined results.
     *  \param StartAngle The starting position, in degrees.
     *                    An angle whose magnitude is more than 360 degrees
     *                    will be clamped to 360 (0).
     *  \param OffsetAngle The angle of the end position, relative to the
     *                     start (i.e. end = \p StartAngle + \p OffsetAngle).
     *                     Its sign indicates the direction of travel.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawArcSize(long x,long y,long width,long height, short StartAngle, short OffsetAngle);
    /**< Draws part of an oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   elliptical arc may not look as intended.  Angles begin at 12 o'clock
     *   and go clockwise if positive, and counter-clockwise otherwise.
     *   \param x The X coordinate of the center of the ellipse will be
     *            \p x + \p width / 2, relative to the graphic context origin.
     *   \param y The Y coordinate of the center of the ellipse will be
     *            \p y + \p height / 2, relative to the graphic context origin.
     *   \param width The oval's horizontal axis length.  Negative width
     *                produces undefined results.
     *   \param height The oval's vertical axis length.  Negative height
     *                 produces undefined results.
     *  \param StartAngle The starting position, in degrees.
     *                    An angle whose magnitude is more than 360 degrees
     *                    will be clamped to 360 (0).
     *  \param OffsetAngle The angle of the end position, relative to the
     *                     start (i.e. end = \p StartAngle + \p OffsetAngle).
     *                     Its sign indicates the direction of travel.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawRRect(const struct rectangle * OuterBox, const struct rectangle * InnerBox);
    /**< Draw a rectangle with rounded edges.
     *   The corners of the rectangle are trimmed by the corner size,
     *   and quarter-circle arcs are drawn to connect the edges.  If the
     *   corner size exceeds the edge length in either direction, no
     *   straight edge will be drawn at all, being replaced by a curve.
     *   This will produce an ellipse if both dimensions are exceeded.
     *   \param OuterBox The outer rectangle.
     *   \param InnerBox The location of this rectangle is ignored.  Its
     *                   width is twice the width of the corner, and its height
     *                   is twice the height of the corner.  If either dimension
     *                   is negative, results are undefined.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawRRectSize(long x,long y,long width,long height, long cornerWidth,long cornerHeight);
    /**< Draw a rectangle with rounded edges.
     *   The corners of the rectangle are trimmed by the corner size,
     *   and quarter-circle arcs are drawn to connect the edges.  If the
     *   corner size exceeds the edge length in either direction, no
     *   straight edge will be drawn at all, being replaced by a curve.
     *   This will produce an ellipse if both dimensions are exceeded.
     *   \param x The X coordinate of the left edge of the rectangle.
     *   \param y The Y coordinate of the top edge of the rectangle.
     *   \param width The distance between the left and right edges of the rectangle.
     *                Negative values produce undefined results.
     *   \param height The distance between the top and bottom edges of the rectangle.
     *                Negative values produce undefined results.
     *   \param cornerWidth The width of the rounded corner.  Negative values
     *                      result in undefined behavior.
     *   \param cornerHeight The height of the rounded corner.  Negative values
     *                       result in undefined behavior.
     */

    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRect(const struct rectangle * Rect, const class graphic * Tile);
    /**< Draw a filled rectangle.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.  Note that line thickness greater than zero
     *   may affect the actual boundaries of the rectangle; the specified
     *   coordinates are the centers of the line end points.
     *   \param Rect The rectangle size and coordinates relative to the
     *               graphic context origin.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRectSize(long x,long y,long width,long height, const class graphic * Tile);
    /**< Draw a filled rectangle at the given position and size.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.  Note that line thickness greater than zero
     *   may affect the actual boundaries of the rectangle; the specified
     *   coordinates are the centers of the line end points.
     *   \param x The X coordinate of the rectangle's left edge relative
     *            to the graphic context origin.
     *   \param y The Y coordinate of the rectangle's top edge relative
     *            to the graphic context origin.
     *   \param width The nominal width of the rectangle, in pixels.  Negative
     *                values draw nothing.
     *   \param height The nominal height of the rectangle, in pixels.
     *                 Negative values draw nothing.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    virtual void FillTrapezoid(long topX, long topY, long topWidth, 
		long bottomX, long bottomY, long bottomWidth, const class graphic * Tile);
    /**< Draw a filled, closed figure with parallel horizontal edges.
     *   This ignores and invalidates the current drawing cursor position.
     *   Negative widths move in the opposite direction.  The "left" edges
     *   of the horizontal lines are joined together, as are the "right" edges.
     *   \param topX The X coordinate of the "left" edge of the top horizontal line.
     *   \param topY The Y coordinate of the "left" edge of the top horizontal line.
     *   \param topWidth The width of the top horizontal line.  If
     *                   the sign of this does not match the sign of
     *                   \p bottomWidth, undefined behavior results.
     *   \param bottomX The X coordinate of the "left" edge of the bottom horizontal line.
     *   \param bottomY The Y coordinate of the "left" edge of the bottom horizontal line.
     *   \param bottomWidth The width of the bottom horizontal line.  If
     *                      the size of this does not match the sign of
     *                      \p topWidth, undefined behavior results.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    virtual void FillPolygon(const struct point * PointArray, short PointCount, const class graphic * Tile);
    /**< Draw a set of lines in a filled, closed path.
     *   Starting at the first given point, line segments are drawn as
     *   per DrawLineTo() except that the drawing cursor is not read or
     *   affected, and end points are drawn with the join style rather than
     *   the endcap style.  The first element of the array is both the
     *   starting position of the first line and the end position of the
     *   last line.  The remaining array elements are the end of the previous
     *   line and the start of the next.  Pixels are never drawn more than
     *   once if the line width is more than zero.  Filling the contents
     *   of the polygon when edges intersect is complex; see e.g.
     *   XCreateGC(3) for details (look for EvenOddRule).
     *   \param PointArray The list of points.
     *   \param PointCount The number of elements in \p PointArray.  A
     *                     value less than one produces undefined behavior.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillOval(const struct rectangle * Rect, const class graphic * Tile);
    /**< Draws a filled oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   ellipse may not look as intended.
     *   \param Rect The oval's horizontal axis will be the rectangle's width,
     *               and its vertical axis will be the rectangle's height.
     *               Its center is the rectangle's center.
     *               Negative width and height produce undefined results.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillOvalSize(long x,long y,long width,long height, const class graphic * Tile);
    /**< Draws a filled oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   ellipse may not look as intended.
     *   \param x The X coordinate of the center of the ellipse will be
     *            \p x + \p width / 2, relative to the graphic context origin.
     *   \param y The Y coordinate of the center of the ellipse will be
     *            \p y + \p height / 2, relative to the graphic context origin.
     *   \param width The oval's horizontal axis length.  Negative width
     *                produces undefined results.
     *   \param height The oval's vertical axis length.  Negative height
     *                 produces undefined results.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillArc(const struct rectangle * EnclRect, short StartAngle, short OffsetAngle, const class graphic * Tile);
    /**< Draws part of a filled oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   elliptical arc may not look as intended.  Angles begin at 12 o'clock
     *   and go clockwise if positive, and counter-clockwise otherwise.
     *   \param EnclRect The oval's horizontal axis will be the rectangle's width,
     *                   and its vertical axis will be the rectangle's height.
     *                   Its center is the rectangle's center.
     *                   Negative width and height produce undefined results.
     *  \param StartAngle The starting position, in degrees.
     *                    An angle whose magnitude is more than 360 degrees
     *                    will be clamped to 360 (0).
     *  \param OffsetAngle The angle of the end position, relative to the
     *                     start (i.e. end = \p StartAngle + \p OffsetAngle).
     *                     Its sign indicates the direction of travel.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillArcSize(long x,long y,long width,long height, 
		short StartAngle, short OffsetAngle, const class graphic * Tile);
    /**< Draws part of a filled oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   elliptical arc may not look as intended.  Angles begin at 12 o'clock
     *   and go clockwise if positive, and counter-clockwise otherwise.
     *   \param x The X coordinate of the center of the ellipse will be
     *            \p x + \p width / 2, relative to the graphic context origin.
     *   \param y The Y coordinate of the center of the ellipse will be
     *            \p y + \p height / 2, relative to the graphic context origin.
     *   \param width The oval's horizontal axis length.  Negative width
     *                produces undefined results.
     *   \param height The oval's vertical axis length.  Negative height
     *                 produces undefined results.
     *  \param StartAngle The starting position, in degrees.
     *                    An angle whose magnitude is more than 360 degrees
     *                    will be clamped to 360 (0).
     *  \param OffsetAngle The angle of the end position, relative to the
     *                     start (i.e. end = \p StartAngle + \p OffsetAngle).
     *                     Its sign indicates the direction of travel.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRRect(const struct rectangle * OuterBox, const struct rectangle * InnerBox, const class graphic * Tile);
    /**< Draw a filled rectangle with rounded edges.
     *   The corners of the rectangle are trimmed by the corner size,
     *   and quarter-circle arcs are drawn to connect the edges.  If the
     *   corner size exceeds the edge length in either direction, no
     *   straight edge will be drawn at all, being replaced by a curve.
     *   This will produce an ellipse if both dimensions are exceeded.
     *   \param OuterBox The outer rectangle.
     *   \param InnerBox The location of this rectangle is ignored.  Its
     *                   width is the width of the corner, and its height
     *                   is the height of the corner.  If either dimension
     *                   is negative, results are undefined.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRRectSize(long x, long y, long width, long height, long cornerWidth, long cornerHeight, const class graphic * Tile);
    /**< Draw a filled rectangle with rounded edges.
     *   The corners of the rectangle are trimmed by the corner size,
     *   and quarter-circle arcs are drawn to connect the edges.  If the
     *   corner size exceeds the edge length in either direction, no
     *   straight edge will be drawn at all, being replaced by a curve.
     *   This will produce an ellipse if both dimensions are exceeded.
     *   \param x The X coordinate of the left edge of the rectangle.
     *   \param y The Y coordinate of the top edge of the rectangle.
     *   \param width The distance between the left and right edges of the rectangle.
     *                Negative values produce undefined results.
     *   \param height The distance between the top and bottom edges of the rectangle.
     *                Negative values produce undefined results.
     *   \param cornerWidth The width of the rounded corner.  Negative values
     *                      result in undefined behavior.
     *   \param cornerHeight The height of the rounded corner.  Negative values
     *                       result in undefined behavior.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    virtual void FillRgn(class region * Rgn, const class graphic * Tile);
    /**< Fills in pixels contained in a region.
     *   Any pixel within the given region is drawn as if it was intersected
     *   with a filled rectangle enclosing its boundaries.
     *   \param Rgn The region to fill.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().  NULL is a solid foreground
     *          color fill.  On true monochrome displays, that foreground
     *          color fill is a stipple pattern estimate of the color.
     */
    virtual void EraseRect(const struct rectangle *Rect);
    /**< Draws a filled rectangle of the background color.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.
     *   \param Rect The rectangle size and coordinates relative to the
     *               graphic context origin.
     */
    virtual void EraseRectSize(long x,long y,long width,long height);
    /**< Draw a filled rectangle in the background color at the given
     *   position and size.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.
     *   \param x The X coordinate of the rectangle's left edge relative
     *            to the graphic context origin.
     *   \param y The Y coordinate of the rectangle's top edge relative
     *            to the graphic context origin.
     *   \param width The nominal width of the rectangle, in pixels.  Negative
     *                values draw nothing.
     *   \param height The nominal height of the rectangle, in pixels.
     *                 Negative values draw nothing.
     */
    virtual void EraseVisualRect();
    /**< Draw the background over the entire graphic context region. */

    // tjm - width & height should really be unsigned.
    // tjm - probably meant to have terminated list or something for
    //       ClipRectList given its name, but default implementation of
    //       BitBltSize forces clip to just 1 rectangle.  I say "optional"
    //       because xgraphic ignores this parameter, anyway.  It should
    //       probably just be dropped.  It should probably be replaced by
    //       a mask (for proper blitting) or a region (as a way of giving
    //       a mask).
    virtual void BitBlt(const struct rectangle *SrcRect, 
		class graphic *DestGraphic, const struct point * DstOrigin, 
		const struct rectangle *ClipRectList);
    /**< Copy a rectangular region of pixels.
     *   \param SrcRect The region to copy.  Coordinates are relative to the
     *                  origin of the source graphic context (this).
     *   \param DestGraphic The target graphical context.
     *   \param DstOrigin The location in the target corresponding to the
     *                    upper-left corner of the rectangle.
     *   \param ClipRectList A rectangle to optionally limit drawing to a
     *                       sub-region of the target.  This is currently
     *                       ignored.
     */
    // tjm - width & height should really be unsigned.
    virtual void BitBltSize(long srcX, long srcY, 
		long dstX, long dstY, long Width, long Height, 
		class graphic *DstGraphic, 
		long clipX, long clipY, long clipWidth, long clipHeight);
    /**< Copy a rectangular region of pixels.
     *   Note that the clipping parameters are currently ignored.
     *   \param SrcX The X coordinate of the left edge of the rectangle to copy,
     *               relative to the origin of the source graphic context (this).
     *   \param SrcY The Y coordinate of the top edge of the rectangle to copy,
     *               relative to the origin of the source graphic context (this).
     *   \param Width The width of the rectangle to copy.  Negative widths
     *                result in undefined behavior.
     *   \param Height The height of the rectangle to copy.  Negative widths
     *                 result in undefined behavior.
     *   \param DestGraphic The target graphical context.
     *   \param DstOrigin The location in the target corresponding to the
     *                    upper-left corner of the rectangle.
     *   \param clipX The X coordinate relative to the destination context
     *                origin to optionally limit drawing.
     *   \param clipY The X coordinate relative to the destination context
     *                origin to optionally limit drawing.
     *   \param clipWidth The width of a region in which to optionally limit
     *                    drawing.
     *   \param clipHeight The height of a region in which to optionally
     *                     limit drawing.
     */
    virtual void SetBitAtLoc(long XPos, long YPos, boolean NewValue);
    /**< Draw a single pixel.
     *   While drawing a zero-length line draws a point, that point has
     *   width and end caps.  This function just draws a single pixel.
     *   It invalidates and ignores the current drawing cursor position.
     *   \param XPos The X location, relative to the graphic context origin.
     *   \param YPos The Y location, relative to the graphic context origin.
     */

    virtual void WritePixImage(long DestX, long DestY, 
		const class pixelimage *SrcPixels, long SrcX, long SrcY, 
		long width, long height);
    /**< Copy a \ref pixelimage into the graphic.
     *   Set pixels in the \ref pixelimage are drawn in the foreground
     *   color, and unset pixels are drawn in the background color.
     *   The transfer mode may change this, of course.
     *   \param DestX The X coordinate of the upper-left corner of the
     *          destination, relative to the graphic context origin.
     *   \param DestY The Y coordinate of the upper-left corner of the
     *          destination, relative to the graphic context origin.
     *   \param SrcPixels The image to copy.
     *   \param SrcX The X offset of the start of the pixels to copy,
     *               within the pixel image.  Values outside of the
     *               width of the source image will result in undefined
     *               behavior.  Under X, an protocol error will be given.
     *   \param SrcY The Y offset of the start of the pixels to copy,
     *               within the pixel image.  Values outside of the
     *               height of the source image will result in undefined
     *               behavior.  Under X, a protocol error will be given.
     *   \param width The width of the pixel area to copy.  Zero or
     *                negative values cause this function to do nothing.
     *                Values which, combined with \p SrcX, exceed the
     *                width of the source image result in undefined
     *                behavior.  Under X, a protocol error will be given.
     *   \param height The height of the pixel area to copy.  Zero or
     *                negative values cause this function to do nothing.
     *                Values which, combined with \p SrcY, exceed the
     *                height of the source image result in undefined
     *                behavior.  Under X, a protocol error will be given.
     */
    virtual void ReadPixImage(long SrcX, long SrcY, 
		class pixelimage *DestPixels, long DestX, long DestY, 
		long width, long height);
    /**< Copy pixels from the graphic to a \ref pixelimage.
     *   Behavior on non-monochrome graphic contexts is undefined.
     *   Otherwise, the color interpretation for the pixels is given by
     *   IsImageInverted().
     *   \param SrcX The X coordinate of the upper-left corner of the region
     *               to copy, relative to the graphic context origin.
     *   \param SrcY The Y coordinate of the upper-left corner of the region
     *               to copy, relative to the graphic context origin.
     *   \param DestPixels The destination image.  Bits within the selected
     *                     rectangle will be updated.
     *   \param DestX The X offset of the start of the area to update,
     *                relative to the left edge of \p DestPixels.  Values
     *                outside of the image's width result in undefined
     *                behavior.
     *   \param DestY The Y offset of the start of the area to update,
     *                relative to the top edge of \p DestPixels.  Values
     *                outside of the image's height result in undefined
     *                behavior.
     *   \param width The width of the rectangular area to copy.  If this
     *                plus \p DestX exceeds the width of the target
     *                pixel image or source view window, behavior is undefined.
     *   \param height The height of the rectangular area to copy.  If this
     *                 plus \p DestY exceeds the height of the target
     *                 pixel image or source view window, behavior is undefined.
     */

    virtual void InsertGraphic(class graphic * EnclosingGraphic, const struct rectangle * EnclosedRectangle);
    virtual void InsertGraphicSize(class graphic * EnclosingGraphic, 
		long xOriginInParent, long yOriginInParent, long width, long height);
    virtual void InsertGraphicRegion(class graphic *EnclosingGraphic, const class region *regions);

    virtual void SetVisualRegion(class region *region);
    virtual class region * GetVisualRegion(class region *retRegion)  ;

    virtual void GetLogicalBounds(struct rectangle * Rect);
    virtual long GetLogicalRight()  ;
    virtual long GetLogicalBottom()  ;

    virtual void GetEnclosedBounds(struct rectangle * Rect);
    virtual long GetEnclosedRight()  ;
    virtual long GetEnclosedBottom()  ;

    virtual void GetVisualBounds(struct rectangle * Rect);
    virtual long GetVisualRight()  ;
    virtual long GetVisualBottom()  ;

    virtual void SetClippingRegion(class region *region);
    virtual class region * GetClippingRegion(class region *retRegion)  ;

    virtual void SetClippingRect(struct rectangle * AdditionalRect);
    virtual void SetClippingRectSize(long x, long y, long w, long h);
    virtual void ClearClippingRect();
    virtual void GetClippingRect(struct rectangle * Rect);

    // The argument should really be unsigned.
    virtual void SetLineWidth(short NewLineWidth);
    /**< Set the width of lines in this graphic context.
     *   The line width applies both to lines and to anything drawn with
     *   an arc or line segment, such as rectangles, polygons, arcs, and
     *   ovals.  The special line width 0 is actually a degenerate width
     *   which is usually one pixel wide, has no guarantee of single draw
     *   only, and has no special end caps or joins.  The default is 1.
     *   \param NewLineWidth The new line width.  Negative values have
     *          undefined behavior, as do values above $2^8-1$.
     */
    virtual short GetLineWidth()  ;
    /**< Return the line width used by this context.
     *   \return the line width used by this context.
     */

    // note: xgraphic says NULL disables, but graphic says NULL keeps old
    //   since there is no other graphic implementation, I'll doc as xgraphic
    // note:  xgraphic says pat len of 0 semi-keeps (updates value to blank,
    //         but retains old value in X GC).   Bad code.
    virtual void SetLineDash( const unsigned char *dashPattern, int dashOffset, LineDash dashType );
    /**< Sets line dash pattern.
     *   A dash pattern consists of odd and even segments.  Even segments are
     *   always drawn in the foreground color.  The color of odd semengs is
     *   dependent on \p dashType.
     *   \param dashPattern If NULL, use only solid lines.  Otherwise,
     *                      it is a zero-terminated array of dash lengths,
     *                      containing at least one length.  If the number
     *                      of elements is odd, the entire array is appended
     *                      to itself to make it even.  Each element is the
     *                      length of that segment.  Note that segment lengths
     *                      are only guaranteed to be accurate in vertical and
     *                      horizontal lines.
     *   \param dahsOffset The start element of \p dashPattern.  If this points
     *                     to an odd element, the odd color is used first.
     *   \param dashType The color of odd dashes.  See graphic::LineType.
     */
    virtual void GetLineDash( unsigned char **dashPattern, int *dashOffset, LineDash *dashType );
    /**< Obtain current dash pattern.
     *   See SetLineDash() for interpretation of dash parameters.
     *   \param dashPattern The current pattern string.  This must be freed with
     *                      free(3) when done.
     *   \param dashOffset The current dash offset.
     *   \param dashType The current color of odd dashes.
     */

    virtual void SetLineCap( LineCap NewLineCap );
    /**< Set line end point style.
     *   \param NewLineCap See graphic::LineCap.  The default is CapButt.
     */
    virtual LineCap GetLineCap()  ;
    /**< Obtain current line end point style.
     *   \return the current line end point style.  See graphic::LineCap.
     */

    virtual void SetLineJoin( LineJoin NewLineJoin );
    /**< Set line join point style.
     *   \param NewLineJoin See graphic::LineJoin.  The default is JoinMiter.
     */
    virtual LineJoin GetLineJoin()  ;
    /**< Obtain current line join point style.
     *   \return the current line join point style.  See graphic::LineJoin.
     */

    virtual void SetTransferMode(short NewTransferMode);
    /**< Set default drawing transfer mode.
     *   See the class summary for details on how this works.
     *   \param NewTransferMode A transfer mode bit array.
     */
    virtual inline short GetTransferMode() { return transferMode; }
    /**< Get current default drawing transfer mode.
     *   See the class summary for details on how this works.
     *   \return the current transfer mode bit array.
     */

    // tjm - FIXME: Is this call necessary?  If so, why did they design it
    //              this way?  It seems retarded.
    virtual void RestoreGraphicsState();
    /**< Place this graphic context into the global state.
     *   This is intended to be called before any drawing session.  While
     *   the graphic context was presented as active for all drawing, it
     *   is not guaranteed.  This routine must be called to guarantee that
     *   the local state is also the global state.  It should be called
     *   any time some other graphic context may have drawn something.
     */
    virtual void FlushGraphics();
    /**< Force immediate drawing.
     *   Drawing commands may be buffered for efficiency.  This forces the
     *   buffer out to the display.
     */

    virtual class graphic * WhitePattern()  ;
    /**< Returns a graphic context for filling with the background color.
     *   \return A graphic for the Tile argument for various filled objects.
     */
    virtual class graphic * BlackPattern()  ;
    /**< Returns a graphic context for filling with the foreground color.
     *   \return A graphic for the Tile argument for various filled objects.
     */
    virtual class graphic * GrayPattern(short IntensityNum, short IntensityDenom)  ;
    /**< Returns a graphic context for filling with a dither pattern.
     *   The pattern is as close as possible to \p IntensityNum * 100 /
     *   \p InensityDenom % foreground and the rest background.
     *   \return A graphic for the Tile argument for various filled objects.
     */
    // tjm - should support other patterns as well.
    virtual void SetPatternOrigin(long xpos, long ypos);
    /**< Set tiling pattern offset.
     *   If an object with a tiling pattern has been moved, and additional
     *   fills are to made adjacent to that, the tiling pattern origin should
     *   be adjusted by the amount that the object moved in order to line the
     *   patterns up.
     *   \param xpos The new tiling pattern origin X offset.
     *   \param ypos The new tiling pattern origin Y offset.
     */
    virtual void GetPatternOrigin(long *xpos, long *ypos);
    /**< Get tiling pattern offset.
     *   See SetPatternOrigin() for how these values are used.  Generally,
     *   after an object move, this is called, the values are adjusted, and
     *   then SetPatternOrigin() is called.
     *   \param xpos The current tiling pattern origin X offset.
     *   \param ypos The current tiling pattern origin Y offset.
     */

    /* If colorName is NULL, use the RGB. */
    virtual void SetForegroundColor(const char *colorName, long red, long green, long blue);
    virtual void GetForegroundColor(const char **colorName, long *red, long *green, long *blue);
    virtual void SetBackgroundColor(const char *colorName, long red, long green, long blue);
    virtual void GetBackgroundColor(const char **colorName, long *red, long *green, long *blue);
    virtual void SetForegroundColor(color *c);
    virtual void SetBackgroundColor(color *c);

    void SetFGColorCell(color *c);
    void SetBGColorCell(color *c);
    
    virtual void SetFGColor( double red, double green, double blue );
    virtual void GetFGColor( double *red, double *green, double *blue );
    virtual void SetBGColor( double red, double green, double blue );
    virtual void GetBGColor( double *red, double *green, double *blue );

    virtual long GetHorizontalResolution()  ;
    /**< Obtain the graphic context resolution, in dots per inch.
     *   \return the horizontal resolution.
     */
    virtual long GetVerticalResolution()  ;
    /**< Obtain the graphic context resolution, in dots per inch.
     *   \return the vertical resolution.
     */
    virtual const char * GetWindowManagerType()  ;
    /**< Obtain a name for the low-level device driver.
     *   This is referred to often as the window manager, or the windowing
     *   system, or simply the display interface driver.  Since WM is no
     *   longer supported, the only value you will ever get here is
     *   "XV11R1".
     *   \return A name for the "window manager" type.
     */
    virtual long GetDevice()  ;
    /**< Obtain the window system-specific device handle.
     *   The only thing that can be done with this value is to compare it
     *   with another context's value to determine if it is on the same
     *   device.  This can also be used to avoid re-checking display
     *   resolution or font metrics.
     *   \return an opaque device handle for this graphic context.
     */
    virtual long DisplayClass()  ;
    /**< Find the current display type.
     *   Symbols of the \ref DisplayClass enumeration are ored together to
     *   form the return value.
     *   \return the type of display this graphic is on.
     */
    virtual boolean IsImageInverted()  ;
    /**< Find interpretation of colors in ReadPixImage() results.
     *   This interpretation of "inverted" is with respect to white
     *   backgrounds (such as paper, or the default ATK background).
     *   \return TRUE is 1 is white and 0 black; FALSE if vice-versa.
     */
    virtual void WriteImage(long DestX, long DestY, 
		class image *image, long SrcX, long SrcY, 
		long width, long height);
    virtual void ReadImage(long SrcX, long SrcY, 
		class image *DestImage, long DestX, long DestY, 
		long width, long height);
    virtual void WriteImage(long DestX, long DestY, 
		ddimage &image, long SrcX, long SrcY, 
		long width, long height);
    virtual void SetFGToShadow(int shadow);  /* see installed shadows.h */
    


    inline long GetLogicalWidth() { return ((this)->localBounds.width); }
    inline long GetLogicalHeight() { return ((this)->localBounds.height); }
    inline long GetLogicalLeft() { return ((this)->localBounds.left); }
    inline long GetLogicalTop() { return ((this)->localBounds.top); }

    inline long GetVisualWidth() { return ((this)->visualBounds.width); }
    inline long GetVisualHeight() { return ((this)->visualBounds.height); }
    inline long GetVisualLeft() { return ((this)->visualBounds.left); }
    inline long GetVisualTop() { return ((this)->visualBounds.top); }

    inline long GetEnclosedWidth() { return ((this)->localBounds.width); }
    inline long GetEnclosedHeight() { return ((this)->localBounds.height); }
    inline long GetEnclosedLeft() { return (point_X(&((this)->enclosedOrigin))); }
    inline long GetEnclosedTop() { return (point_Y(&((this)->enclosedOrigin))); }

    inline class colormap  ** GetColormap() { return ((this)->colormap); }
    inline void SetColormap(class colormap **cmap) { ((this)->colormap = (cmap)); }
    inline class colormap **GetInheritedColormap() { return ((this)->inheritedColormap ? (this)->inheritedColormap : NULL); }
    inline void SetInheritedColormap(class colormap **cmap) { ((this)->inheritedColormap = (cmap)); }
    inline void ClearInheritedColormap() { ((this)->inheritedColormap = NULL); }
    inline class colormap  ** CurrentColormap() { return ((this->colormap && *(this->colormap)) ? this->colormap : this->inheritedColormap); }

    inline color *GetForegroundColor() {
	return fore;
    }
    inline color *GetBackgroundColor() {
	return back;
    }

    static void SetDefaultColors(const char *foreground, const char *background);
    static void GetDefaultColors(const char **foreground, const char **background);
    static class graphic * CreateGraphic(class view *v)  ;
    
    
	graphic();
    static void ComputeShadow(long  br , long  bg , long  bb, long  *rr , long  *rg , long  *rb, int  shadow);
    static void ComputeShadowDouble(double br, double bg, double bb, double *rr, double *rg, double *rb, int shadow);
    

  protected:
    struct rectangle localBounds;    /* What is size of rectangle */
    friend class view; // tjm - FIXME: just for localBounds; sets it to 0.
    struct rectangle visualBounds;   /* How much of rect is seen */
    friend class xim; // adjusts many of these variables
    class region *localRegion;
    class region *visualRegion;
    struct point savedOrigin;	     /* Logical offsets of origin */
				     /* Saved across insertions */
    struct point enclosedOrigin;     /* local rect in parent's coord*/
    class fontdesc * currentFont;   /* Current font for text drawing */
    short  spaceShim;
    short  transferMode;	    /* ALU function for bitblit */
    short  lineWidth;		    /* How thick to make outlines */
    LineDash lineDashType;	    /* Dash style for lines */
    int	   lineDashOffset;	    /* Dash offset (phase) for lines */
    unsigned char *lineDashPattern;	    /* Dash pattern for lines */
    LineCap  lineCap;		    /* Cap style for lines */
    LineJoin lineJoin;		    /* Join style for lines */
    class region * clippingRegion; /* Rects for clipping */
    struct point    currentPoint;	    /* Start of drawing operations */
    class colormap **colormap,**inheritedColormap;
/* private data */
    struct point physicalOrigin;
    friend class physical;
    struct point patternOrigin;
    icolormap colors;
    color *fore, *back;
};
/** @} */
#endif /* _graphic_H_ */
