#pragma once

// Private header, not for direct use in user code

#include <gkyl_array.h>
#include <gkyl_mom_type.h>
#include <gkyl_range.h>
#include <gkyl_ref_count.h>
#include <gkyl_mom_bcorr_lbo_gyrokinetic_kernels.h>

typedef void (*lbo_gyrokinetic_momf_t)(const int *idx, enum gkyl_vel_edge edge, const double *vBoundary,
  const double *dxv, double _m, const double *fIn, double* GKYL_RESTRICT out);

// The cv_index[cd].vdim[vd] is used to index the various list of
// kernels below
static struct { int vdim[3]; } cv_index[] = {
  {-1, -1, -1}, // 0x makes no sense
  {-1,  0,  1}, // 1x kernel indices
  {-1, -1,  2}, // 2x kernel indices
  {-1, -1,  3}, // 3x kernel indices  
};

// for use in kernel tables
typedef struct { lbo_gyrokinetic_momf_t kernels[3]; } gkyl_mom_bcorr_lbo_gyrokinetic_kern_list;

//
// Serendipity basis kernels
//

// boundary integral moment correction kernel lists (both momentum and energy)
GKYL_CU_D
static const gkyl_mom_bcorr_lbo_gyrokinetic_kern_list ser_mom_bcorr_lbo_gyrokinetic_kernels[] = {
  // 1x kernels
  { NULL, mom_bcorr_lbo_gyrokinetic_1x1v_ser_p1, mom_bcorr_lbo_gyrokinetic_1x1v_ser_p2 }, // 0
  { NULL, mom_bcorr_lbo_gyrokinetic_1x2v_ser_p1, mom_bcorr_lbo_gyrokinetic_1x2v_ser_p2 }, // 1
  // 2x kernels
  { NULL, mom_bcorr_lbo_gyrokinetic_2x2v_ser_p1, mom_bcorr_lbo_gyrokinetic_2x2v_ser_p2 }, // 2
  // 3x kernels
  { NULL, mom_bcorr_lbo_gyrokinetic_3x2v_ser_p1, mom_bcorr_lbo_gyrokinetic_3x2v_ser_p2 }, // 3
};

//
// Tensor-product basis kernels
//
// // boundary integral moment correction kernel lists (both momentum and energy)
// GKYL_CU_D
// static const gkyl_mom_bcorr_lbo_gyrokinetic_kern_list ten_mom_bcorr_lbo_gyrokinetic_kernels[] = {
//   // 1x kernels
//   { NULL, NULL, mom_bcorr_lbo_gyrokinetic_1x1v_tensor_p2 }, // 0
//   { NULL, NULL, mom_bcorr_lbo_gyrokinetic_1x2v_tensor_p2 }, // 1
//   // 2x kernels
//   { NULL, NULL, mom_bcorr_lbo_gyrokinetic_2x2v_tensor_p2 }, // 2
//   // 3x kernels
//   { NULL, NULL, NULL }, // 3
// };

struct mom_type_bcorr_lbo_gyrokinetic {
  struct gkyl_mom_type momt;
  lbo_gyrokinetic_momf_t kernel; // moment calculation kernel
  double _m; // mass of species
  double vBoundary[2*GKYL_MAX_DIM];
};

void gk_mom_free(const struct gkyl_ref_count *ref);

GKYL_CU_D
static void
kernel(const struct gkyl_mom_type *momt, const double *xc, const double *dx,
  const int *idx, const double *f, double* out, void *param)
{
  struct mom_type_bcorr_lbo_gyrokinetic *mom_bcorr = container_of(momt, struct mom_type_bcorr_lbo_gyrokinetic, momt);
  enum gkyl_vel_edge edge = *(enum gkyl_vel_edge *)param;
  
  return mom_bcorr->kernel(idx, edge, mom_bcorr->vBoundary, dx, mom_bcorr->_m, f, out);
}
