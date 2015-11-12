/***********************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   © 2004 Polycom, Inc.
**
**   All rights reserved.
**
***********************************************************************/

#define REGION_POWER_STEPSIZE_DB 3.010299957
#define ABS_REGION_POWER_LEVELS  32
#define DIFF_REGION_POWER_LEVELS 24

#define DRP_DIFF_MIN -12
#define DRP_DIFF_MAX 11

#define MAX_NUM_BINS 16
#define MAX_VECTOR_INDICES 625
#define MAX_VECTOR_DIMENSION 5
#include "huff_tab.h"


