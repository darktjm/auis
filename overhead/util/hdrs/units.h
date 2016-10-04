#ifndef UNITS_H
#define UNITS_H

#include <math.h>
/** \addtogroup libutil
 * @{ */
/** \addtogroup units Unit Conversions
 *  Various physical unit conversion macros, in floating point.
 * @{ */

#define units_PICASperINCH	(6.0) /**< Conversion factor */
#define units_POINTSperINCH	(72.0) /**< Conversion factor */
#define units_MILSperINCH	(1000.0) /**< Conversion factor */
#define units_INCHESperFOOT	(12.0) /**< Conversion factor */
#define units_INCHESperYARD	(36.0) /**< Conversion factor */
#define units_INCHESperROD	(5.5 * units_INCHESperYARD) /**< Conversion factor */
#define units_INCHESperFURLONG	(40 * units_INCHESperROD) /**< Conversion factor */
#define units_INCHESperMILE	(5280 * units_INCHESperFOOT) /**< Conversion factor */
#define units_INCHESperLINK	(7.92) /**< Conversion factor */
#define units_INCHESperCHAIN	(100 * units_INCHESperLINK) /**< Conversion factor */
#define units_INCHESperFATHOM	(6 * units_INCHESperFOOT) /**< Conversion factor */
#define units_INCHESperNAUTMILE	(6O76.1155 * units_INCHESperFOOT) /**< Conversion factor */
#define units_INCHESperLEAGUE	(3 * units_INCHESperNAUTMILE) /**< Conversion factor */
#define units_INCHESperCUBIT	(18.0) /**< Conversion factor */
#define units_INCHESperLIGHTYEAR (5878510000000.0 * units_INCHESperMILE) /**< Conversion factor */
#define units_INCHESperPARSEC	(3.262 * units_INCHESperLIGHTYEAR) /**< Conversion factor */

#define units_INCHESperMETER	(39.37007262) /**< Conversion factor */

#define units_ANGSTROMSperMETER	(10000000000.0) /**< Conversion factor */
#define units_MICRONSperMETER	(1000000.0) /**< Conversion factor */
#define units_MILLIMETERSperMETER (1000.0) /**< Conversion factor */
#define units_CENTIMETERSperMETER (100.0) /**< Conversion factor */
#define units_METERSperKILOMETER  (1000.0) /**< Conversion factor */

#define units_RADIANSperDEGREE	(M_PI / 180.0) /**< Conversion factor */

#define units_CELSIUSperFAHRENHEIT (5.0 / 9.0) /**< Conversion factor */
#define units_FAHRENHEITperCELSIUS (9.0 / 5.0) /**< Conversion factor */

#define units_CELSIUSinFAHRENHEIT(cel) \
	(units_FAHRENHEITperCELSIUS * (cel) + 32.0)  /**< Convert Celsius to Fahrenheit, including offset */
#define units_FAHRENHEITinCELSIUS(fah) \
	(units_CELSIUSperFAHRENHEIT * ((fah) - 32.0))  /**< Convert Fahrenheit to Celsius, including offset */

/** @} */
/** @} */

#endif
