/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#define cursor_width 18
#define cursor_height 13
#define cursor_x_hot 16
#define cursor_y_hot 6
static char cursor_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3c, 0x00,
   0x00, 0x78, 0x00, 0xfe, 0xff, 0x00, 0xfe, 0xff, 0x01, 0xfe, 0xff, 0x00,
   0x00, 0x78, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x0c, 0x00,
   0x00, 0x00, 0x00};

#define cursormask_width 18
#define cursormask_height 13
#define cursormask_x_hot 16
#define cursormask_y_hot 6

static char cursormask_bits[] = {
   0x00, 0x0e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x7e, 0x00,
   0xff, 0xff, 0x00, 0xff, 0xff, 0x01, 0xff, 0xff, 0x03, 0xff, 0xff, 0x01,
   0xff, 0xff, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x0e, 0x00};
