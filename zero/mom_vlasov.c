#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <gkyl_alloc.h>
#include <gkyl_alloc_flags_priv.h>
#include <gkyl_mom_vlasov.h>
#include <gkyl_mom_vlasov_priv.h>
#include <gkyl_util.h>

void
gkyl_mom_free(const struct gkyl_ref_count *ref)
{
  struct gkyl_mom_type *momt = container_of(ref, struct gkyl_mom_type, ref_count);
  if (GKYL_IS_CU_ALLOC(momt->flags))
    gkyl_cu_free(momt->on_dev);
  gkyl_free(momt);
}

struct gkyl_mom_type*
gkyl_mom_vlasov_new(const struct gkyl_basis* cbasis,
  const struct gkyl_basis* pbasis, const char *mom, bool use_gpu)
{
  assert(cbasis->poly_order == pbasis->poly_order);

#ifdef GKYL_HAVE_CUDA
  if(use_gpu) {
    return gkyl_mom_vlasov_cu_dev_new(cbasis, pbasis, mom);
  } 
#endif
  struct mom_type_vlasov *mom_vm = gkyl_malloc(sizeof(struct mom_type_vlasov));
  int cdim = cbasis->ndim, pdim = pbasis->ndim, vdim = pdim-cdim;
  int poly_order = cbasis->poly_order;

  mom_vm->momt.cdim = cdim;
  mom_vm->momt.pdim = pdim;
  mom_vm->momt.poly_order = poly_order;
  mom_vm->momt.num_config = cbasis->num_basis;
  mom_vm->momt.num_phase = pbasis->num_basis;
  mom_vm->momt.kernel = kernel;

  // choose kernel tables based on basis-function type
  const gkyl_mom_kern_list *m0_kernels, *m1i_kernels,
    *m2_kernels, *m2ij_kernels, *m3i_kernels, *m3ijk_kernels, *five_moments_kernels;

  switch (cbasis->b_type) {
    case GKYL_BASIS_MODAL_SERENDIPITY:
      m0_kernels = ser_m0_kernels;
      m1i_kernels = ser_m1i_kernels;
      m2_kernels = ser_m2_kernels;
      m2ij_kernels = ser_m2ij_kernels;
      m3i_kernels = ser_m3i_kernels;
      m3ijk_kernels = ser_m3ijk_kernels;
      five_moments_kernels = ser_five_moments_kernels;
      break;

    case GKYL_BASIS_MODAL_TENSOR:
      m0_kernels = tensor_m0_kernels;
      m1i_kernels = tensor_m1i_kernels;
      m2_kernels = tensor_m2_kernels;
      m2ij_kernels = tensor_m2ij_kernels;
      m3i_kernels = tensor_m3i_kernels;
      m3ijk_kernels = tensor_m3ijk_kernels;
      five_moments_kernels = tensor_five_moments_kernels;
      break;

    default:
      assert(false);
      break;    
  }

  if (strcmp(mom, "M0") == 0) { // density
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m0_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m0_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = 1;
  }
  else if (strcmp(mom, "M1i") == 0) { // momentum
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m1i_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m1i_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = vdim;
  }
  else if (strcmp(mom, "M2") == 0) { // energy
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m2_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m2_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = 1;
  }
  else if (strcmp(mom, "M2ij") == 0) { // pressure tensor in lab-frame
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m2ij_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m2ij_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = vdim*(vdim+1)/2;
  }
  else if (strcmp(mom, "M3i") == 0) { // heat-flux vector in lab-frame
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m3i_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m3i_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = vdim;
  }
  else if (strcmp(mom, "M3ijk") == 0) { // heat-flux tensor in lab-frame
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != m3ijk_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = m3ijk_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];

    int m3ijk_count[] = { 1, 4, 10 };
    mom_vm->momt.num_mom = m3ijk_count[vdim-1];
  }
  else if (strcmp(mom, "FiveMoments") == 0) { // Zeroth, First, and Second moment computed together
    assert(cv_index[cdim].vdim[vdim] != -1);
    assert(NULL != five_moments_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
    
    mom_vm->kernel = five_moments_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];
    mom_vm->momt.num_mom = 2+vdim;
  }
  else {
    // string not recognized
    gkyl_exit("gkyl_mom_type_vlasov: Unrecognized moment requested!");
  }

  mom_vm->momt.flags = 0;
  GKYL_CLEAR_CU_ALLOC(mom_vm->momt.flags);
  mom_vm->momt.ref_count = gkyl_ref_count_init(gkyl_mom_free);
  
  mom_vm->momt.on_dev = &mom_vm->momt; // on host, self-reference
    
  return &mom_vm->momt;
}

struct gkyl_mom_type *
gkyl_int_mom_vlasov_new(const struct gkyl_basis* cbasis, const struct gkyl_basis* pbasis, bool use_gpu)
{
  assert(cbasis->poly_order == pbasis->poly_order);

#ifdef GKYL_HAVE_CUDA
  if(use_gpu) {
    return gkyl_int_mom_vlasov_cu_dev_new(cbasis, pbasis);
  } 
#endif
  struct mom_type_vlasov *mom_vm = gkyl_malloc(sizeof(struct mom_type_vlasov));
  int cdim = cbasis->ndim, pdim = pbasis->ndim, vdim = pdim-cdim;
  int poly_order = cbasis->poly_order;

  mom_vm->momt.cdim = cdim;
  mom_vm->momt.pdim = pdim;
  mom_vm->momt.poly_order = poly_order;
  mom_vm->momt.num_config = cbasis->num_basis;
  mom_vm->momt.num_phase = pbasis->num_basis;
  mom_vm->momt.kernel = kernel;
  mom_vm->momt.num_mom = 2+vdim;
  
  // set kernel pointer
  switch (cbasis->b_type) {
    case GKYL_BASIS_MODAL_SERENDIPITY:
      assert(cv_index[cdim].vdim[vdim] != -1);
      assert(NULL != ser_int_mom_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
      
      mom_vm->kernel = ser_int_mom_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];

      break;

    case GKYL_BASIS_MODAL_TENSOR:
      assert(cv_index[cdim].vdim[vdim] != -1);
      assert(NULL != tensor_int_mom_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order]);
      
      mom_vm->kernel = tensor_int_mom_kernels[cv_index[cdim].vdim[vdim]].kernels[poly_order];

      break;


    default:
      assert(false);
      break;    
  }  

  mom_vm->momt.flags = 0;
  GKYL_CLEAR_CU_ALLOC(mom_vm->momt.flags);
  mom_vm->momt.ref_count = gkyl_ref_count_init(gkyl_mom_free);
  
  mom_vm->momt.on_dev = &mom_vm->momt; // on host, self-reference
    
  return &mom_vm->momt;  
}

#ifndef GKYL_HAVE_CUDA

struct gkyl_mom_type*
gkyl_mom_vlasov_cu_dev_new(const struct gkyl_basis* cbasis,
  const struct gkyl_basis* pbasis, const char *mom)
{
  assert(false);
  return 0;
}

struct gkyl_mom_type *
gkyl_int_mom_vlasov_cu_dev_new(const struct gkyl_basis* cbasis,
  const struct gkyl_basis* pbasis)
{
  assert(false);
  return 0;  
}

#endif
