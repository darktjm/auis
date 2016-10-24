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
 *  all things are drawn.  This is mostly an abstract class; children
 *  are created specific to the type of graphics device (e.g. \ref xgraphic).
 *
 *  The graphic context contains the following elements:
 *  -  A drawing cursor.  This is an invisible point used by other
 *     drawing functions as a point of origin.
 *  -  A drawing color.  The interpretation of this color is dependent on
 *     the display hardware.
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
 *     Some common combinations are provided as well.
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
	CapProjecting /**< Like CapButt, but the rectangle extends beyond the end points by 1/2 the line width. */
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
	Monochrome = (1<<1), /**< Psudeo-class used for matching against monochrome classes (GrayScale, StaticGray). */
	Color = (1<<2), /**< Pseudo-class used for class matching against color classes (PseudoColor, StaticColor, TrueColor, DirectColor). */
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
    virtual void DrawLineToPt(struct point * LineEnd);
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
     *          and the default horizontal position os the left edge.
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
     *          and the default horizontal position os the left edge.
     *          Use a combination of graphic::position constants.
     */
    virtual void SetSpaceShim(short Amount);
    /**< Set the width of spaces in drawn text.
     *   This width is in pixels.  When a shim is active, spaces in text
     *   are not drawn; instead, the number of pixels given is simply skipped
     *   before starting to draw the text after the space(s).
     *   \param Amount The width of a space.  Set to zero to disable, and draw
     *          spaces as part of the text.
     */
    virtual short GetSpaceShim()  ;
    /**< Find out how spaces are drawn in text.
     *   \return If zero, spaces are drawn as part of the text font.
     *           Otherwise, spaces are "drawn" by displaying text before
     *           and after the space, with the given number of pixels between.
     */
    virtual void SetFont(class fontdesc * ChosenFont);
    /**< Set the font for text rendering.
     *   This includes styles, such as bold and italic.
     *   \param ChosenFont The font to use.  The default is andysans12.
     */
    virtual class fontdesc * GetFont()  ;
    /**< Find out what font is being used for text rendering.
     *   \return the font.
     */

    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void DrawRect(struct rectangle * Rect);
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
    virtual void DrawPolygon(struct point * PointArray, short PointCount);
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
    virtual void DrawPath(struct point * PointArray, short PointCount);
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
    virtual void DrawOval(struct rectangle * Rect);
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
    virtual void DrawArc(struct rectangle * EnclRect, short StartAngle, short OffsetAngle);
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
    virtual void DrawRRect(struct rectangle * OuterBox, struct rectangle * InnerBox);
    /**< Draw a rectangle with rounded edges.
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
    virtual void FillRect(struct rectangle * Rect, class graphic * Tile);
    /**< Draw a filled rectangle.
     *   The current drawing cursor position is ignored and invalidated
     *   by this function.  Note that line thickness greater than zero
     *   may affect the actual boundaries of the rectangle; the specified
     *   coordinates are the centers of the line end points.
     *   \param Rect The rectangle size and coordinates relative to the
     *               graphic context origin.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRectSize(long x,long y,long width,long height, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    virtual void FillTrapezoid(long topX, long topY, long topWidth, 
		long bottomX, long bottomY, long bottomWidth, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    virtual void FillPolygon(struct point * PointArray, short PointCount, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillOval(struct rectangle * Rect, class graphic * Tile);
    /**< Draws a filled oval with horizontal and vertical axes.
     *   Note that display pixel aspect is not taken into account, so the
     *   ellipse may not look as intended.
     *   \param Rect The oval's horizontal axis will be the rectangle's width,
     *               and its vertical axis will be the rectangle's height.
     *               Its center is the rectangle's center.
     *               Negative width and height produce undefined results.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillOvalSize(long x,long y,long width,long height, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillArc(struct rectangle * EnclRect, short StartAngle, short OffsetAngle, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillArcSize(long x,long y,long width,long height, 
		short StartAngle, short OffsetAngle, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRRect(struct rectangle * OuterBox, struct rectangle * InnerBox, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    // tjm - width & height should really be unsigned.  Also, the X
    //       routine should deal w/ >16-bit width/height.
    virtual void FillRRectSize(long x, long y, long width, long height, long cornerWidth, long cornerHeight, class graphic * Tile);
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
     *          BlackPattern(), or GrayPattern().
     */
    virtual void FillRgn(class region * Rgn, class graphic * Tile);
    /**< Fills in pixels contained in a region.
     *   Any pixel within the given region is drawn as if it was intersected
     *   with a filled rectangle enclosing its boundaries.
     *   \param Rgn The region to fill.
     *   \param Tile A fill pattern.  Must be generated with WhitePattern(),
     *          BlackPattern(), or GrayPattern().
     */
    virtual void EraseRect(struct rectangle *Rect);
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
    virtual void BitBlt(struct rectangle *SrcRect, 
		class graphic *DestGraphic, struct point * DstOrigin, 
		struct rectangle *ClipRectList);
    /**< Copy a rectangular region of pixels.
     *   \param SrcRect The region to copy.  Coordinates are relative to the
     *                  origin of the source graphic context (this).
     *   \param DestGraphic The target graphical context.
     *   \param DstOrigin The location in the target corresponding to the
     *                    upper-left corner of the rectangle.
     *   \param ClipRectList A rectangle to optionally limit drawing to a
     *                       sub-region of the target.
     */
    // tjm - width & height should really be unsigned.
    virtual void BitBltSize(long srcX, long srcY, 
		long dstX, long dstY, long Width, long Height, 
		class graphic *DstGraphic, 
		long clipX, long clipY, long clipWidth, long clipHeight);
    /**< Copy a rectangular region of pixels.
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
		class pixelimage *SrcPixels, long SrcX, long SrcY, 
		long width, long height);
    virtual void ReadPixImage(long SrcX, long SrcY, 
		class pixelimage *DestPixels, long DestX, long DestY, 
		long width, long height);

    virtual void InsertGraphic(class graphic * EnclosingGraphic, const struct rectangle * EnclosedRectangle);
    virtual void InsertGraphicSize(class graphic * EnclosingGraphic, 
		long xOriginInParent, long yOriginInParent, long width, long height);
    virtual void InsertGraphicRegion(class graphic *EnclosingGraphic, const class region *regions);

    virtual void SetVisualRegion(class region *region);
    virtual class region * GetVisualRegion(class region *retRegion)  ;

    virtual void MoveLogicalOrigin(long DeltaX, long DeltaY);
    virtual void SetLogicalOrigin(long NewX, long NewY);
    virtual void SetLogicalOriginPt(struct point * Pt);
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
     *          undefined behavior.
     */
    virtual short GetLineWidth()  ;
    /**< Return the line width used by this context.
     *   \return the line width used by this context.
     */

    virtual void SetLineDash( const char *dashPattern, int dashOffset, short dashType );
    virtual void GetLineDash( char **dashPattern, int *dashOffset, short *dashType );

    virtual void SetLineCap( short NewLineCap );
    virtual short GetLineCap()  ;

    virtual void SetLineJoin( short NewLineJoin );
    virtual short GetLineJoin()  ;

    virtual void SetTransferMode(short NewTransferMode);
    virtual inline short GetTransferMode() { return transferMode; }  ;

    virtual void RestoreGraphicsState();
    virtual void FlushGraphics();

    virtual class graphic * WhitePattern()  ;
    virtual class graphic * BlackPattern()  ;
    virtual class graphic * GrayPattern(short IntensityNum, short IntensityDenom)  ;
    // tjm - should support other patterns as well.
    virtual void SetPatternOrigin(long xpos, long ypos);
    virtual void GetPatternOrigin(long *xpos, long *ypos);

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
    virtual long GetDevice()  ;
    virtual long DisplayClass()  ;
    virtual boolean IsImageInverted()  ;
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
    short  lineDashType;	    /* Dash style for lines */
    int	   lineDashOffset;	    /* Dash offset (phase) for lines */
    char   *lineDashPattern;	    /* Dash pattern for lines */
    short  lineCap;		    /* Cap style for lines */
    short  lineJoin;		    /* Join style for lines */
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
