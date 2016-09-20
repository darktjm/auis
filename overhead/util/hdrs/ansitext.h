#ifndef ANSITEXT_H
#define ANSITEXT_H

/** \addtogroup libutil
 * @{ */

#define ansitext_BaseFactor		(2.0)
#define ansitext_AscenderFactor		(1.0)
#define ansitext_DescenderFactor	(1.0)

#define ansitext_CapitalFactor \
	(ansitext_BaseFactor + ansitext_AscenderFactor)
#define ansitext_DescentRatio \
	(ansitext_DescenderFactor / ansitext_CapitalFactor)

#define ansitext_ComputeAscent(fontsize)  (fontsize)
#define ansitext_ComputeDescent(fontsize) ((fontsize) * ansitext_DescentRatio)

#define ansitext_ComputeDelta(fontsize, spacing) \
	((((spacing) / 4.0) - (1.0 / 6.0)) * (fontsize))

#define ansitext_SlantFontSlope		(2.5)

/** @} */

#endif
