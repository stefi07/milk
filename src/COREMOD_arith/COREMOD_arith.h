#ifndef _ARITH_H
#define _ARITH_H

//void __attribute__ ((constructor)) libinit_COREMOD_arith();

//int init_COREMOD_arith();

#include "COREMOD_arith/execute_arith.h"
#include "COREMOD_arith/image_arith__Cim_Cim__Cim.h"
#include "COREMOD_arith/image_arith__im__im.h"
#include "COREMOD_arith/image_arith__im_f__im.h"
#include "COREMOD_arith/image_arith__im_f_f__im.h"
#include "COREMOD_arith/image_arith__im_im__im.h"
#include "COREMOD_arith/image_crop.h"
#include "COREMOD_arith/image_crop2D.h"
#include "COREMOD_arith/image_multicrop2D.h"
#include "COREMOD_arith/image_dxdy.h"
#include "COREMOD_arith/image_merge3D.h"
#include "COREMOD_arith/image_stats.h"
#include "COREMOD_arith/image_total.h"
#include "COREMOD_arith/imfunctions.h"

#include "COREMOD_arith/image_setzero.h"
#include "COREMOD_arith/image_set_col.h"
#include "COREMOD_arith/image_set_row.h"
#include "COREMOD_arith/image_set_1Dpixrange.h"
#include "COREMOD_arith/image_set_2Dpix.h"


//imageID arith_make_slopexy(const char *ID_name, long l1,long l2, double sx, double sy);

#endif
