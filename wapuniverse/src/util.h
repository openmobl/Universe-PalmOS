#ifndef _util_h_
#define _util_h_

#include 	<PalmOS.h>
#include	"WAPUniverse.h"

#define	SafeMemPtrNew		Malloc
#define	SafeMemPtrFree(x)	if (x != NULL) Free(x)

#endif
