#pragma once

#include <gkyl_app.h>
#include <gkyl_comm.h>
#include <gkyl_moment_braginskii.h>
#include <gkyl_mp_scheme.h>
#include <gkyl_util.h>
#include <gkyl_wave_prop.h>
#include <gkyl_wv_eqn.h>

#include <time.h>

// number of components that various applied functions should return
enum {
  GKYL_MOM_APP_NUM_APPLIED_CURRENT = 3,
  GKYL_MOM_APP_NUM_EXT_EM = 6,
  GKYL_MOM_APP_NUM_APPLIED_ACCELERATION = 3,
  GKYL_MOM_APP_NUM_NT_SOURCE = 2
};

// Parameters for moment species
struct gkyl_moment_species {
  char name[128]; // species name
  double charge, mass; // charge and mass
 
  struct gkyl_wv_eqn *equation; // equation object
  enum gkyl_wave_limiter limiter; // limiter to use
  enum gkyl_wave_split_type split_type; // edge splitting to use

  enum gkyl_braginskii_type type_brag; // which Braginskii equations
  bool has_grad_closure; // has gradient-based closure (only for 10 moment) 

  bool has_friction; // Run with frictional sources.
  bool use_explicit_friction; // Use an explicit (SSP-RK3) solver for integrating frictional sources.
  double friction_Z; // Ionization number for frictional sources.
  double friction_T_elc; // Electron temperature for frictional sources.
  double friction_Lambda_ee; // Electron-electron collisional term for frictional sources.

  bool has_volume_sources; // Run with volume-based geometrical sources.
  double volume_gas_gamma; // Adiabatic index for volume-based geometrical sources.
  double volume_U0; // Initial comoving plasma velocity for volume-based geometrical sources.
  double volume_R0; // Initial radial distance from expansion/contraction center for volume-based geometrical sources.

  bool has_reactivity; // Run with reactive sources.
  double reactivity_gas_gamma; // Adiabatic index for reactive sources.
  double reactivity_specific_heat_capacity; // Specific heat capacity for reactive sources.
  double reactivity_energy_of_formation; // Energy of formation for reactive sources.
  double reactivity_ignition_temperature; // Ignition temperature for reactive sources.
  double reactivity_reaction_rate; // Reaction rate for reactive sources.

  bool has_einstein_medium; // Run with coupled fluid-Einstein sources in plane-symmetric spacetimes.
  double medium_gas_gamma; // Adiabatic index for coupled fluid-Einstein sources in plane-symmetric spacetimes.
  double medium_kappa; // Stress-energy prefactor for coupled fluid-Einstein sources in plane-symmetric spacetimes.

  int evolve; // evolve species? 1-yes, 0-no
  bool force_low_order_flux; // should  we force low-order flux?

  void *ctx; // context for initial condition init function (and potentially other functions)
  // pointer to initialization function
  void (*init)(double t, const double *xn, double *fout, void *ctx);
  bool is_app_accel_static; // flag to indicate if applied acceleration is static
  void *app_accel_ctx; // context for applied acceleration
  // pointer to applied acceleration/forces function
  void (*app_accel_func)(double t, const double *xn, double *fout, void *ctx);
  // pointer to user-defined number density and temperature sources
  void (*nT_source_func)(double t, const double *xn, double *fout, void *ctx);
  bool nT_source_set_only_once;
  // boundary conditions
  enum gkyl_species_bc_type bcx[2], bcy[2], bcz[2];
  // pointer to boundary condition functions along x
  void (*bcx_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
  void (*bcx_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
  // pointer to boundary condition functions along y
  void (*bcy_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
  void (*bcy_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
  // pointer to boundary condition functions along z
  void (*bcz_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
  void (*bcz_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double *skin, double * GKYL_RESTRICT ghost, void *ctx);
};

// Parameter for EM field
struct gkyl_moment_field {
  double epsilon0, mu0;
  double elc_error_speed_fact, mag_error_speed_fact;

  enum gkyl_wave_limiter limiter; // limiter to use

  int evolve; // evolve field? 1-yes, 0-no

  void *ctx; // context for initial condition init function (and potentially other functions)
  // pointer to initialization function
  void (*init)(double t, const double *xn, double *fout, void *ctx);
  // pointer to applied current function
  void (*app_current_func)(double t, const double *xn, double *fout, void *ctx);
  double t_ramp_curr; // linear ramp for turning on applied currents
  
  bool is_ext_em_static; // flag to indicate if external field is time-independent
  // pointer to external fields
  void (*ext_em_func)(double t, const double *xn, double *fout, void *ctx);
  double t_ramp_E; // linear ramp for turning on external E field
  bool use_explicit_em_coupling; // flag to indicate if using explicit em-coupling

  bool has_volume_sources; // Run with volume-based geometrical sources.
  double volume_gas_gamma; // Adiabatic index for volume-based geometrical sources.
  double volume_U0; // Initial comoving plasma velocity for volume-based geometrical sources.
  double volume_R0; // Initial radial distance from expansion/contraction center for volume-based geometrical sources.

  // boundary conditions
  enum gkyl_field_bc_type bcx[2], bcy[2], bcz[2];
  // pointer to boundary condition functions along x
  void (*bcx_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
  void (*bcx_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
  // pointer to boundary condition functions along y
  void (*bcy_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
  void (*bcy_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
  // pointer to boundary condition functions along z
  void (*bcz_lower_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
  void (*bcz_upper_func)(const struct gkyl_wv_eqn* eqn, double t, int nc, const double* skin, double* GKYL_RESTRICT ghost, void* ctx);
};

// Choices of schemes to use in the fluid solver
enum gkyl_moment_scheme {
  GKYL_MOMENT_WAVE_PROP = 0, // default, 2nd-order FV
  GKYL_MOMENT_MP, // monotonicity-preserving Suresh-Huynh scheme
  GKYL_MOMENT_KEP // Kinetic-energy preserving scheme
};

// Lower-level inputs: in general this does not need to be set by the
// user. It is needed when the App is being created on a sub-range of
// the global range, and is meant for use in higher-level drivers that
// use MPI or other parallel mechanism.
struct gkyl_moment_low_inp {
  // local range over which App operates
  struct gkyl_range local_range;
  // communicator to used
  struct gkyl_comm *comm;
};

// Top-level app parameters
struct gkyl_moment {
  char name[128]; // name of app: used as output prefix

  int ndim; // space dimensions
  double lower[3], upper[3]; // lower, upper bounds
  int cells[3]; // config-space cells

  void *c2p_ctx; // context for mapc2p function
  // pointer to mapc2p function: xc are the computational space
  // coordinates and on output xp are the corresponding physical space
  // coordinates.
  void (*mapc2p)(double t, const double *xc, double *xp, void *ctx);

  double cfl_frac; // CFL fraction to use

  enum gkyl_moment_scheme scheme_type; // scheme to update fluid and moment eqns
  
  enum gkyl_mp_recon mp_recon; // reconstruction scheme to use
  bool skip_mp_limiter; // should MP limiter be skipped?
  bool use_hybrid_flux_kep; // should shock-hybrid scheme be used when using KEP?

  int num_periodic_dir; // number of periodic directions
  int periodic_dirs[3]; // list of periodic directions

  int num_skip_dirs; // number of directions to skip
  int skip_dirs[3]; // directions to skip

  int num_species; // number of species
  struct gkyl_moment_species species[GKYL_MAX_SPECIES]; // species objects
  struct gkyl_moment_field field; // field object

  bool has_collision; // has collisions
  // scaling factors for collision frequencies so that nu_sr=nu_base_sr/rho_s
  // nu_rs=nu_base_rs/rho_r, and nu_base_sr=nu_base_rs
  double nu_base[GKYL_MAX_SPECIES][GKYL_MAX_SPECIES];

  bool has_nT_sources;

  bool has_braginskii; // has Braginskii transport
  double coll_fac; // multiplicative collisionality factor for Braginskii  

  // this should not be set by typical user-facing code but only by
  // higher-level drivers
  bool has_low_inp; // should one use low-level inputs?
  struct gkyl_moment_low_inp low_inp; // low-level inputs
};

// Simulation statistics
struct gkyl_moment_stat {
  long nup; // calls to update
  double total_tm; // time for simulation (not including ICs)
  
  long nfail; // number of failed time-steps

  //// wave_prop stuff
  double species_tm; // time to compute species updates
  double field_tm; // time to compute field updates
  double sources_tm; // time to compute source terms

  //// stuff for MP-XX/SSP-RK schemes
  long nfeuler; // calls to forward-Euler method
    
  long nstage_2_fail; // number of failed RK stage-2s
  long nstage_3_fail; // number of failed RK stage-3s

  double stage_2_dt_diff[2]; // [min,max] rel-diff for stage-2 failure
  double stage_3_dt_diff[2]; // [min,max] rel-diff for stage-3 failure
  
  double init_species_tm; // time to initialize all species
  double init_field_tm; // time to initialize fields

  double species_rhs_tm; // time to compute species collisionless RHS
  double species_bc_tm; // time to apply BCs

  double field_rhs_tm; // time to compute field RHS
  double field_bc_tm; // time to apply BCs
};

// Object representing moments app
typedef struct gkyl_moment_app gkyl_moment_app;

/**
 * Construct a new moments app.
 *
 * @param vm App inputs. See struct docs.
 * @return New moment app object.
 */
gkyl_moment_app* gkyl_moment_app_new(struct gkyl_moment *mom);

/**
 * Compute maximum estimated stable dt wtih current app state. Call
 * after app initialized and after initial conditions set.
 *
 * @param app App object.
 * @retuen maximum estimated stable dt
 */
double gkyl_moment_app_max_dt(gkyl_moment_app* app);

/**
 * Initialize species and field.
 *
 * @param app App object.
 * @param t0 Time for initial conditions.
 */
void gkyl_moment_app_apply_ic(gkyl_moment_app* app, double t0);

/**
 * Initialize field.
 *
 * @param app App object.
 * @param t0 Time for initial conditions
 */
void gkyl_moment_app_apply_ic_field(gkyl_moment_app* app, double t0);

/**
 * Initialize species.
 *
 * @param app App object.
 * @param sidx Index of species to initialize.
 * @param t0 Time for initial conditions
 */
void gkyl_moment_app_apply_ic_species(gkyl_moment_app* app, int sidx, double t0);

/**
 * Write field and species data to file.
 * 
 * @param app App object.
 * @param tm Time-stamp
 * @param frame Frame number
 */
void gkyl_moment_app_write(const gkyl_moment_app* app, double tm, int frame);

/**
 * Write field data to file.
 * 
 * @param app App object.
 * @param tm Time-stamp
 * @param frame Frame number
 */
void gkyl_moment_app_write_field(const gkyl_moment_app *app, double tm, int frame);

/**
 * Write species data to file.
 * 
 * @param app App object.
 * @param sidx Index of species to write
 * @param tm Time-stamp
 * @param frame Frame number
 */
void gkyl_moment_app_write_species(const gkyl_moment_app* app, int sidx, double tm, int frame);

/**
 * Write field energy to file.
 *
 * @param app App object.
 */
void gkyl_moment_app_write_field_energy(gkyl_moment_app *app);

/**
 * Write integrated moments to file.
 *
 * @param app App object.
 */
void gkyl_moment_app_write_integrated_mom(gkyl_moment_app *app);

/**
 * Write stats to file. Data is written in json format.
 *
 * @param app App object.
 */
void gkyl_moment_app_stat_write(const gkyl_moment_app* app);

/**
 * Write output to console: this is mainly for diagnostic messages the
 * driver code wants to write to console. It accounts for parallel
 * output by not messing up the console with messages from each rank.
 *
 * @param app App object
 * @param fp File pointer for open file for output
 * @param fmt Format string for console output
 * @param argp Objects to write
 */
void gkyl_moment_app_cout(const gkyl_moment_app* app, FILE *fp, const char *fmt, ...);

/**
 * Advance simulation by a suggested time-step 'dt'. The dt may be too
 * large in which case method will attempt to take a smaller time-step
 * and also return it as the 'dt_actual' field of the status
 * object. If the suggested time-step 'dt' is smaller than the largest
 * stable time-step the method will use the smaller value instead,
 * returning the larger time-step in the 'dt_suggested' field of the
 * status object. If the method fails to find any stable time-step
 * then the 'success' flag will be set to 0. At that point the calling
 * code must abort the simulation as this signals a catastrophic
 * failure and the simulation can't be safely continued.
 * 
 * @param app App object.
 * @param dt Suggested time-step to advance simulation
 * @return Status of update.
 */
struct gkyl_update_status gkyl_moment_update(gkyl_moment_app *app, double dt);

/**
 * Calculate integrated field energy
 *
 * @param tm Time at which integrated diagnostic are to be computed
 * @param app App object.
 */
void gkyl_moment_app_calc_field_energy(gkyl_moment_app *app, double tm);

/**
 * Calculate integrated moments
 *
 * @param app App object.
 * @param tm Time at which integrated diagnostic are to be computed
 */
void gkyl_moment_app_calc_integrated_mom(gkyl_moment_app *app, double tm);

/**
 * Return simulation statistics.
 * 
 * @return Return statistics.
 */
struct gkyl_moment_stat gkyl_moment_app_stat(gkyl_moment_app *app);

/**
 * Free moment app.
 *
 * @param app App to release.
 */
void gkyl_moment_app_release(gkyl_moment_app* app);
