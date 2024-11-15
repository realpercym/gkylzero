#pragma once

#include <gkyl_array.h>
#include <gkyl_elem_type.h>
#include <gkyl_evalf_def.h>
#include <gkyl_range.h>

GKYL_CU_DH
static inline void*
flat_fetch(void *data, size_t loc)
{
  return ((char*) data) + loc;
}

// Struct used to pass function pointer and context to various buffer
// copy operators
struct gkyl_array_copy_func {
  array_copy_func_t func;
  void *ctx;
  uint32_t flags;

  void *ctx_on_dev; // pointer to on-device context (or itself)
  struct gkyl_array_copy_func *on_dev; // pointer to itself or device data
};

/**
 * Check if array_copy_func is on device.
 *
 * @param bc BC function to check
 * @return true if eqn on device, false otherwise
 */
bool
gkyl_array_copy_func_is_cu_dev(const struct gkyl_array_copy_func *bc);

/**
 * Clear out = val. Returns out.
 *
 * @param out Output array
 * @param val Factor to set 
 * @return out array
 */
struct gkyl_array* gkyl_array_clear(struct gkyl_array *out, double val);

/**
 * Compute out = out + a*inp. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @return out array
 */
struct gkyl_array* gkyl_array_accumulate(struct gkyl_array *out,
  double a, const struct gkyl_array *inp);

/**
 * Compute out = out + a*inp[coff] where coff is a component-offset if
 * out->ncomp < inp->ncomp, or out[coff] = out[coff]+ a*inp if
 * out->ncomp > inp->ncomp. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @param coff Component offset
 * @return out array
 */
struct gkyl_array* gkyl_array_accumulate_offset(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, int coff);

/**
 * Set out = a*inp. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @return out array
 */
struct gkyl_array* gkyl_array_set(struct gkyl_array *out,
  double a, const struct gkyl_array *inp);

/**
 * Set out = a*inp[coff] where coff is a component-offset if
 * out->ncomp < inp->ncomp, or out[coff] = a*inp if
 * out->ncomp > inp->ncomp. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @param coff Component offset
 * @return out array
 */
struct gkyl_array* gkyl_array_set_offset(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, int coff);

/**
 * Scale out = a*out. Returns out.
 *
 * @param out Output array
 * @param a Factor to scale
 * @return out array
 */
struct gkyl_array* gkyl_array_scale(struct gkyl_array *out, double a);

/**
 * Scale out = a*out. Returns out.
 *
 * @param out Output array
 * @param a Factor to scale that varies by cell
 * @return out array
 */
struct gkyl_array* gkyl_array_scale_by_cell(struct gkyl_array *out, const struct gkyl_array *a);

/**
 * Shift the k-th coefficient in every cell, out_k = a+out_k. Returns out.
 *
 * @param out Output array.
 * @param a Factor to shift k-th coefficient by.
 * @param k Coefficient to be shifted.
 * @return out array.
 */
struct gkyl_array* gkyl_array_shiftc(struct gkyl_array *out, double a, unsigned k);

/**
 * Clear out = val. Returns out.
 *
 * @param out Output array
 * @param val Factor to set 
 * @return out array
 */
struct gkyl_array* gkyl_array_clear_range(struct gkyl_array *out, double val,
  const struct gkyl_range *range);

/**
 * Compute out = out + a*inp over a range of indices.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @param range Range specifying region to accumulate
 * @return out array
 */
struct gkyl_array* gkyl_array_accumulate_range(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, const struct gkyl_range *range);

/**
 * Compute out = out + a*inp[coff] where coff is a component-offset if
 * out->ncomp < inp->ncomp, or out[coff] = out[coff]+ a*inp if
 * out->ncomp > inp->ncomp, over a range of indices. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @param coff Component offset
 * @return out array
 */
struct gkyl_array* gkyl_array_accumulate_offset_range(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, int coff, const struct gkyl_range *range);

/**
 * Set out = a*inp. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @return out array
 * @param range Range specifying region to set
 */
struct gkyl_array* gkyl_array_set_range(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, const struct gkyl_range *range);

/**
 * Set out = a*inp[coff] where coff is a component-offset if
 * out->ncomp < inp->ncomp, or out[coff] = a*inp if
 * out->ncomp > inp->ncomp, over a range of indices. Returns out.
 *
 * @param out Output array
 * @param a Factor to multiply input array
 * @param inp Input array
 * @return out array
 * @param range Range specifying region to set
 */
struct gkyl_array* gkyl_array_set_offset_range(struct gkyl_array *out,
  double a, const struct gkyl_array *inp, int coff, const struct gkyl_range *range);

/**
 * Scale out = a*ut. Returns out.
 *
 * @param out Output array
 * @param a Factor to scale by
 * @return out array
 * @param range Range specifying region to scale
 */
struct gkyl_array* gkyl_array_scale_range(struct gkyl_array *out,
  double a, const struct gkyl_range *range);

/**
 * Shift the k-th coefficient in every cell, out_k = a+out_k within
 * a given range. Returns out.
 *
 * @param out Output array.
 * @param a Factor to shift k-th coefficient by.
 * @param k Coefficient to be shifted.
 * @param range Range to shift coefficient k in.
 * @return out array.
 */
struct gkyl_array* gkyl_array_shiftc_range(struct gkyl_array *out, double a,
  unsigned k, const struct gkyl_range *range);

/**
 * Copy out inp. Returns out.
 *
 * @param out Output array
 * @param inp Input array
 * @param range Range specifying region to copy
 * @return out array
 */
struct gkyl_array* gkyl_array_copy_range(struct gkyl_array *out,
  const struct gkyl_array *inp, const struct gkyl_range *range);

/**
 * Copy out inp over specified ranges. Returns out.
 * input and output ranges must have the same volume.
 *
 * @param out Output array
 * @param inp Input array
 * @param out_range Range specifying region to copy to in out array
 * @param inp_range Range specifying region to copy to from in inp array
 * @return out array
 */
struct gkyl_array* gkyl_array_copy_range_to_range(struct gkyl_array *out,
  const struct gkyl_array *inp, struct gkyl_range *out_range, struct gkyl_range *inp_range);

/**
 * Perform an "reduce" operation of data in the array.
 *
 * @param res On output, reduces values (ncomp size)
 * @param arr Array to perform reduction on.
 * @param op Reduction operators
 */
void gkyl_array_reduce(double *res, const struct gkyl_array *arr, enum gkyl_array_op op);

/**
 * Perform an "reduce" operation of data in the array. Data is reduced
 * component-wise.
 *
 * @param res On output, reduced values (ncomp size)
 * @param arr Array to perform reduction on.
 * @param op Reduction operators
 * @param range Range specifying region
 */
void gkyl_array_reduce_range(double *res,
  const struct gkyl_array *arr, enum gkyl_array_op op, const struct gkyl_range *range);

/**
 * Copy region of array into a buffer. The buffer must be preallocated
 * and at least of size arr->size*arr->elemSz bytes.
 *
 * @param data Output data buffer.
 * @param arr Array to copy from
 * @param range Range specifying region to copy from
 */
void gkyl_array_copy_to_buffer(void *data, const struct gkyl_array *arr,
  const struct gkyl_range *range);

/**
 * Copy buffer into region of array. The array must be preallocated.
 *
 * @param arr Array to copy into
 * @param data Input data buffer.
 * @param range Range specifying region to copy into
 */
void gkyl_array_copy_from_buffer(struct gkyl_array *arr, const void *data,
  const struct gkyl_range *range);

/**
 * Copy region of array into a buffer, calling user-specified function
 * as the copying is done. The buffer must be preallocated and at
 * least of size arr->size*arr->elemSz bytes.
 *
 * @param data Output data buffer.
 * @param arr Array to copy from
 * @param range Range specifying region to copy from
 * @param cf Function pointer and context
 */
void gkyl_array_copy_to_buffer_fn(void *data, const struct gkyl_array *arr,
  const struct gkyl_range *range, struct gkyl_array_copy_func *cf);

/**
 * Copy region of array into a buffer, calling user-specified function
 * as the copying is done. While the copying is being performed the
 * index in @a dir is "flipped". (TODO: WHAT DOES THIS MEAN?). The
 * buffer must be preallocated and at least of size
 * arr->size*arr->elemSz bytes.
 *
 * @param data Output data buffer.
 * @param arr Array to copy from
 * @dir Direction to apply index flip
 * @param range Range specifying region to copy from
 * @param cf Function pointer and context
 */
void gkyl_array_flip_copy_to_buffer_fn(void *data, const struct gkyl_array *arr,
  int dir, const struct gkyl_range *range, struct gkyl_array_copy_func *cf);

/**
 * Host-side wrappers for array operations
 */
void gkyl_array_clear_cu(struct gkyl_array* out, double val);

void gkyl_array_accumulate_cu(struct gkyl_array* out, double a, const struct gkyl_array* inp);

void gkyl_array_accumulate_offset_cu(struct gkyl_array* out, double a, const struct gkyl_array* inp, int coff);

void gkyl_array_set_cu(struct gkyl_array* out, double a, const struct gkyl_array* inp);

void gkyl_array_set_offset_cu(struct gkyl_array* out, double a, const struct gkyl_array* inp, int coff);

void gkyl_array_scale_cu(struct gkyl_array* out, double a);

void gkyl_array_scale_by_cell_cu(struct gkyl_array* out, const struct gkyl_array* a);

void gkyl_array_shiftc_cu(struct gkyl_array* out, double a, unsigned k);

void gkyl_array_shiftc_range_cu(struct gkyl_array *out, double a, unsigned k, const struct gkyl_range *range);

/**
 * Host-side wrappers for range-based array operations
 */
void gkyl_array_clear_range_cu(struct gkyl_array *out, double val, const struct gkyl_range *range);

void gkyl_array_accumulate_range_cu(struct gkyl_array *out,
  double a, const struct gkyl_array* inp, const struct gkyl_range *range);

void gkyl_array_accumulate_offset_range_cu(struct gkyl_array *out,
  double a, const struct gkyl_array* inp, int coff, const struct gkyl_range *range);

void gkyl_array_set_range_cu(struct gkyl_array *out,
  double a, const struct gkyl_array* inp, const struct gkyl_range *range);

void gkyl_array_set_offset_range_cu(struct gkyl_array *out,
  double a, const struct gkyl_array* inp, int coff, const struct gkyl_range *range);

void gkyl_array_scale_range_cu(struct gkyl_array *out,
  double a, const struct gkyl_range *range);

void gkyl_array_copy_range_cu(struct gkyl_array *out, const struct gkyl_array* inp, 
  const struct gkyl_range *range);

void gkyl_array_copy_range_to_range_cu(struct gkyl_array *out, const struct gkyl_array* inp,
  struct gkyl_range *out_range, struct gkyl_range *inp_range);

void gkyl_array_copy_to_buffer_cu(void *data, const struct gkyl_array *arr, 
  const struct gkyl_range *range);

void gkyl_array_copy_from_buffer_cu(struct gkyl_array *arr, const void *data, 
  const struct gkyl_range *range);

void gkyl_array_copy_to_buffer_fn_cu(void *data, const struct gkyl_array *arr,
  const struct gkyl_range *range, struct gkyl_array_copy_func *cf);

void gkyl_array_flip_copy_to_buffer_fn_cu(void *data, const struct gkyl_array *arr,
  int dir, const struct gkyl_range *range, struct gkyl_array_copy_func *cf);
