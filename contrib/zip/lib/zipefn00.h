#ifndef _zipefn00_h_
#define _zipefn00_h_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* zipefn00.h	Zip Subroutine Library Symbolic Facility-names		      */
/* Author	TC Peters						      */
/* Information Technology Center		   Carnegie-Mellon University */

/**  SPECIFICATION -- External Facility Suite  *********************************

TITLE	Zip Subroutine Library Symbolic Facility-names

MODULE	zipefn00.h

NOTICE	IBM Internal Use Only

DESCRIPTION
	This file is to be included in the compilation of both client-programs
	and the Zip Subroutine Library modules. It defines the symbolc "Facility-
	names of Zip Subroutine Procedures/Functions.

HISTORY
  05/01/85	Created (TCP)
  05/16/86	Added names (RML)

END-SPECIFICATION  ************************************************************/

/* Symbolic External Facility Names (EFN) */


#define zip_lowest_figure_facility_name			0	/* Lowest Base-facility name defined */

#define zip_Unspecified_facility_Name_EFN	0
#define zip_Initialize_EFN			0
#define zip_Terminate_EFN			0
#define zip_Set_general_Exception_Handler_EFN	0
#define zip_Reset_general_Exception_Handler_EFN	0

#define zip_figure_EFN				0
#define zip_Image_Figure_EFN			0
#define zip_Figure_Image_EFN			0
#define zip_Figure_Name_EFN			0
#define zip_Figure_Type_EFN			0 /* ???? */
#define zip_01_EFN				0

#define zip_Containing_Stream_Figure_EFN	0
#define zip_Containing_Image_EFN		0
#define zip_Set_Figure_Exception_Handler_EFN	0
#define zip_Reset_Figure_Exception_Handler_EFN	0  
#define zip_Set_Figure_Text_EFN			0
#define zip_Set_Figure_Pattern_EFN		0
#define zip_Set_Figure_Font_EFN			0
#define zip_Display_Figure_EFN			0
#define zip_Draw_Figure_EFN			0
#define zip_Clear_Figure_EFN			0
#define zip_Which_Figure_EFN			0
#define zip_Highlight_Figure_Points_EFN		0
#define zip_Normalize_Figure_Points_EFN		0
#define zip_Hide_Figure_Points_EFN		0
#define zip_Expose_Figure_Points_EFN		0
#define zip_Change_Figure_Point_EFN		0
#define zip_Remove_Figure_Point_EFN		0
#define zip_Add_Figure_Point_EFN		0

#define zip_Acknowledge_Image_Status_Message_EFN 0 /* ???
#define zip_Issue_Image_Status_Message		0  ??? */


#define zip_highest_figure_facility_name	0	/* Highest Figure-facility name defined */

#define zip_lowest_image_facility_name		0	/* Lowest Image-facility name defined */

#define zip_Image_EFN				0
#define zip_Stream_Image_EFN			0
#define zip_Image_Name_EFN			0
#define zip_Image_01_EFN			0

#define zip_Containing_Stream_EFN		0
#define zip_Containing_Image_Image_EFN		0
#define zip_Containing_Image_EFN		0
#define zip_Set_Image_Exception_Handler_EFN	0
#define zip_Reset_Image_Exception_Handler_EFN	0 
#define zip_Set_Image_Text_EFN			0
#define zip_Set_Image_Pattern_EFN		0
#define zip_Set_Image_Font_EFN			0
#define zip_Display_Image_EFN			0
#define zip_Draw_Image_EFN			0
#define zip_Clear_Image_EFN			0
#define zip_Which_Image_EFN			0
#define zip_Highlight_Image_Points_EFN		0
#define zip_Normalize_Image_Points_EFN		0
#define zip_Hide_Image_Points_EFN		0
#define zip_Expose_Image_Points_EFN		0

#define zip_highest_image_facility_name		0	/* Highest Image-facility name defined */

#define zip_lowest_pane_facility_name		0	/* Lowest Pane-facility name defined */

#define zip_Pane_EFN				0
#define zip_Pane_Name_EFN			0
#define zip_Pane_Stream_EFN			0

#define zip_Create_Window_Pane_EFN		0
#define zip_Create_Panel_Layout_EFN		0
#define zip_Create_Layout_Pane_EFN		0
#define zip_Destroy_Pane_EFN			0
#define zip_Redisplay_Panes_EFN			0

#define zip_Set_Pane_Exception_Handler_EFN	0
#define zip_Reset_Pane_Exception_Handler_EFN	0 
#define zip_Set_Pane_Coordinates_EFN		0
#define zip_Set_Pane_Border_EFN			0
#define zip_Set_Pane_Stream_EFN			0
#define zip_Set_Pane_Image_EFN			0
#define zip_Set_Pane_Figure_EFN			0
#define zip_Set_Pane_Zoom_Factor_EFN		0


#define zip_Set_Pane_Text_EFN			0
#define zip_Set_Pane_Pattern_EFN		0
#define zip_Set_Pane_Font_EFN			0
#define zip_Draw_Pane_EFN			0
#define zip_Which_Pane_EFN			0
#define zip_Highlight_Pane_Points_EFN		0
#define zip_Normalize_Pane_Points_EFN		0
#define zip_Hide_Pane_Points_EFN		0
#define zip_Expose_Pane_Points_EFN		0

#define zip_Display_Pane_EFN			0
#define zip_Print_Pane_EFN			0
#define zip_Clear_Pane_EFN			0
#define zip_Invert_Pane_EFN			0
#define zip_Zoom_Pane_EFN			0
#define zip_Scale_Pane_EFN			0
#define zip_Handle_Planning_EFN			0
#define zip_Pan_Pane_EFN			0
#define zip_Pan_Pane_To_Edge_EFN		0
#define zip_Flip_Pane_EFN			0
#define zip_Flop_Pane_EFN			0
#define zip_Balance_Pane_EFN			0
#define zip_Hide_Pane_EFN			0
#define zip_Expose_Pane_EFN			0
#define zip_Hide Pane_Points_EFN		0
#define zip_Expose_Pane_Points_EFN		0
#define zip_Hide_Pane_Coordinates_EFN		0
#define zip_Expose_Pane_Coordinates_EFN		0
#define zip_Which_Pane_EFN			0

#define zip_highest_pane_facility_name		0	/* Highest Pane-facility name defined */

#define zip_lowest_stream_facility_name		0	/* Lowest Stream-facility name defined */

#define zip_Stream_EFN				0
#define zip_Stream_Name_EFN			0
#define zip_Create_Stream_EFN			0
#define zip_Destroy_Stream_EFN			0
#define zip_Open_Stream_EFN			0
#define zip_Close_Stream_EFN			0
#define zip_Read_Stream_EFN			0
#define zip_Write_Stream_EFN			0

#define zip_Set_Stream_Exception_Handler_EFN	0
#define zip_Reset_Stream_Exception_Handler_EFN	0  
#define zip_Set_Stream_Text_EFN			0
#define zip_Set_Stream_Pattern_EFN		0
#define zip_Set_Stream_Font_EFN			0
#define zip_Set_Stream_Source_EFN		0

#define zip_Display_Stream_EFN			0
#define zip_Draw_Stream_EFN			0
#define zip_Which_Stream_EFN			0
#define zip_Copy_Stream_EFN			0


#define zip_highest_stream_facility_name		80	/* Highest Stream-facility name defined */


#endif /* _zipefn00_h_ */
