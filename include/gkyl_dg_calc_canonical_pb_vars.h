#pragma once

#include <gkyl_array.h>
#include <gkyl_basis.h>
#include <gkyl_eqn_type.h>
#include <gkyl_range.h>
#include <gkyl_rect_grid.h>

// Object type
typedef struct gkyl_dg_calc_canonical_pb_vars gkyl_dg_calc_canonical_pb_vars;

/**
 * Create new updater to compute canonical_pb variables needed in 
 * updates for general geometry. Methods compute:
 * alpha_surf : Surface expansion of phase space flux alpha = {z, H}
 * 
 * @param phase_grid Phase space grid (for getting cell spacing and cell center)
 * @param conf_basis Configuration space basis functions
 * @param phase_basis Phase space basis functions
 * @param use_gpu bool to determine if on GPU
 * @return New updater pointer.
 */
struct gkyl_dg_calc_canonical_pb_vars* 
gkyl_dg_calc_canonical_pb_vars_new(const struct gkyl_rect_grid *phase_grid, 
  const struct gkyl_basis *conf_basis, const struct gkyl_basis *phase_basis,  bool use_gpu);

/**
 * Create new updater to compute canonical_pb general geometry variables on
 * NV-GPU. See new() method for documentation.
 */
struct gkyl_dg_calc_canonical_pb_vars* 
gkyl_dg_calc_canonical_pb_vars_cu_dev_new(const struct gkyl_rect_grid *phase_grid, 
  const struct gkyl_basis *conf_basis, const struct gkyl_basis *phase_basis);

/**
 * Compute surface expansion of phase space flux alpha = {z, H}
 * 
 * Note: Each cell stores the surface expansion on the *lower* edge of the cell
 * @param up Updater for computing general geometry canonical_pb variables 
 * @param conf_range Configuration space range (should only be local range because geometry only defined on local range)
 * @param phase_range Phase space range 
 * @param phase_ext_range Extended Phase space range (so we obtain alpha_surf at all the needed surfaces)
 * @param hamil Hamiltonian expansion in a cell
 * @param alpha_surf Output surface expansion in a cell on the *lower* edge in each direction 
 * @param sgn_alpha_surf Output sign(alpha) at quadrature points along a surface 
 * @param const_sgn_alpha Output boolean array for if sign(alpha) is a constant on the surface
 *                        If sign(alpha) is a constant, kernels are simpler and we exploit this fact.
 */
void gkyl_dg_calc_canonical_pb_vars_alpha_surf(struct gkyl_dg_calc_canonical_pb_vars *up, 
  const struct gkyl_range *conf_range, const struct gkyl_range *phase_range, const struct gkyl_range *phase_ext_range, 
  struct gkyl_array* hamil,
  struct gkyl_array* alpha_surf, struct gkyl_array* sgn_alpha_surf, struct gkyl_array* const_sgn_alpha);

/**
 * Host-side wrappers for canonical_pb general geometry variable operations on device
 */
void gkyl_dg_calc_canonical_pb_vars_alpha_surf_cu(struct gkyl_dg_calc_canonical_pb_vars *up, 
  const struct gkyl_range *conf_range, const struct gkyl_range *phase_range, const struct gkyl_range *phase_ext_range, 
  struct gkyl_array* hamil,
  struct gkyl_array* alpha_surf, struct gkyl_array* sgn_alpha_surf, struct gkyl_array* const_sgn_alpha);

/**
 * Compute surface expansion of phase space flux alpha = {z, H}
 * 
 * Note: Each cell stores the surface expansion on the *lower* edge of the cell
 * @param up Updater for computing general geometry canonical_pb variables 
 * @param conf_range Configuration space range 
 * @param h_ij_inv Inverse of the metric tensor expansion
 * @param M2ij M2ij moment expansion: int(wiwjfd^3w)
 * @param V_drift Drift velocity moment expansion
 * @param M1i Drift velocity times density moment expansion
 * @param pressure Output, scalar pressure expansion
 */
void gkyl_canonical_pb_pressure(struct gkyl_dg_calc_canonical_pb_vars *up, const struct gkyl_range *conf_range,
 const struct gkyl_array *h_ij_inv, 
 const struct gkyl_array *M2ij, const struct gkyl_array *V_drift, const struct gkyl_array *M1i,
 struct gkyl_array *pressure);

 /**
 * Host-side wrappers for canonical_pb pressure operations on device
 */
void gkyl_canonical_pb_pressure_cu(struct gkyl_dg_calc_canonical_pb_vars *up, const struct gkyl_range *conf_range,
 const struct gkyl_array *h_ij_inv, 
 const struct gkyl_array *M2ij, const struct gkyl_array *V_drift, const struct gkyl_array *M1i,
 struct gkyl_array *pressure);

/**
 * Delete pointer to updater to compute canonical_pb general geometry variables.
 *
 * @param up Updater to delete.
 */
void gkyl_dg_calc_canonical_pb_vars_release(struct gkyl_dg_calc_canonical_pb_vars *up);