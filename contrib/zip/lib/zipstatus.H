#ifndef _zipstatus_H_
#define _zipstatus_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libzip
 * @{ */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*
zipstatus.H

  04/15/88	Created for ATK (TCP)
*/

#define  zipstatus_VERSION		    1

struct zipstatus_options
  {
  unsigned int				    xxxx		: 1;
  };

struct zipstatus_states
  {
  unsigned int				    xxxx		: 1;
  };


#include <zipview.H>
class zipstatus : public  zipview
  {
public:
	TRACED_CLASS(zipstatus);





  virtual long Issue_Message( char *msg );
  virtual long Acknowledge_Message( char *msg );
  virtual long Clear_Message();
  virtual long Issue_Status_Message( long facility, long status );
  virtual long Acknowledge_Status_Message( long facility, long status );
  virtual long Issue_Figure_Status_Message( zip_type_figure figure );
  virtual long Acknowledge_Figure_Status_Message( zip_type_figure figure );
  virtual long Issue_Image_Status_Message( zip_type_image image );
  virtual long Acknowledge_Image_Status_Message( zip_type_image image );
  virtual long Issue_Stream_Status_Message( zip_type_stream stream );
  virtual long Acknowledge_Stream_Status_Message( zip_type_stream stream );
  virtual long Issue_Pane_Status_Message( zip_type_pane pane );
  virtual long Acknowledge_Pane_Status_Message( zip_type_pane pane );




  
	zipstatus();
  




  struct zipstatus_options	 options;
  struct zipstatus_states	 states;
  };
/** @} */
#endif /* _zipstatus_H_ */
