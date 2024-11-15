#pragma once

#include <gkyl_array.h>
#include <gkyl_array_ops.h>
#include <gkyl_elem_type.h>
#include <gkyl_rect_grid.h>
#include <gkyl_ref_count.h>
#include <gkyl_rect_decomp.h>

// The return value of the functions is an error code. Success is
// denoted by 0 and failure by other values.

// Forward declaration
struct gkyl_comm;

struct gkyl_comm_state;

// Get local "rank"
typedef int (*get_rank_t)(struct gkyl_comm *comm, int *rank);

// Get number of ranks
typedef int (*get_size_t)(struct gkyl_comm *comm, int *sz);

// Blocking send @a array to @a dest process using @a tag.
typedef int (*gkyl_array_send_t)(struct gkyl_array *array, int dest, int tag,
  struct gkyl_comm *comm);

// Nonblocking send @a array to @a dest process using @a tag, and.
// store the status of this comm in @a state.
typedef int (*gkyl_array_isend_t)(struct gkyl_array *array, int dest, int tag,
  struct gkyl_comm *comm, struct gkyl_comm_state *state);

// Blocking receive @a array from @a src process using @a tag.
typedef int (*gkyl_array_recv_t)(struct gkyl_array *array, int src, int tag,
  struct gkyl_comm *comm);

// Nonblocking receive @a array from @a src process using @a tag, and
// store the status of this comm in @a state.
typedef int (*gkyl_array_irecv_t)(struct gkyl_array *array, int src, int tag,
  struct gkyl_comm *comm, struct gkyl_comm_state *state);

// "Reduce" all elements of @a type in array @a data and store output in @a out
typedef int (*all_reduce_t)(struct gkyl_comm *comm, enum gkyl_elem_type type,
  enum gkyl_array_op op, int nelem, const void *inp, void *out);

// "Synchronize" @a array across the regions or blocks.
typedef int (*gkyl_array_sync_t)(struct gkyl_comm *comm,
  const struct gkyl_range *local, const struct gkyl_range *local_ext,
  struct gkyl_array *array);

// "Synchronize" @a array across the periodic directions
typedef int (*gkyl_array_per_sync_t)(struct gkyl_comm *comm,
  const struct gkyl_range *local, const struct gkyl_range *local_ext,
  int nper_dirs, const int *per_dirs,
  struct gkyl_array *array);

// Write array to specified file
typedef int (*gkyl_array_write_t)(struct gkyl_comm *comm,
  const struct gkyl_rect_grid *grid, const struct gkyl_range *range,
  const struct gkyl_array *arr, const char *fname);

// Create a new communicator that extends the communicator to work on a
// extended domain specified by erange
typedef struct gkyl_comm* (*extend_comm_t)(const struct gkyl_comm *comm,
  const struct gkyl_range *erange);

// Create a new communicator by splitting a comm, and choosing members
// of new communicator according to the color rank. It can be used with
// a new decomp object, or the same one used for the parent comm, depending
// of the use case.
typedef struct gkyl_comm* (*split_comm_t)(const struct gkyl_comm *comm,
  int color, struct gkyl_rect_decomp *new_decomp);

// Barrier
typedef int (*barrier_t)(struct gkyl_comm *comm);

// Allocate/free state objects.
typedef struct gkyl_comm_state* (*comm_state_new_t)(struct gkyl_comm *comm);
typedef void (*comm_state_release_t)(struct gkyl_comm_state *state);

// Wait for a request.
typedef void (*comm_state_wait_t)(struct gkyl_comm_state *state);

// Start and end a group call (e.g. in NCCL).
typedef void (*comm_group_call_start_t)();
typedef void (*comm_group_call_end_t)();

// Structure holding data and function pointers to communicate various
// Gkeyll objects across multi-region or multi-block domains
struct gkyl_comm {

  get_rank_t get_rank; // get local rank function
  get_size_t get_size; // get number of ranks
  gkyl_array_send_t gkyl_array_send; // blocking send array.
  gkyl_array_isend_t gkyl_array_isend; // nonblocking send array.
  gkyl_array_recv_t gkyl_array_recv; // blocking recv array.
  gkyl_array_irecv_t gkyl_array_irecv; // nonblocking recv array.
  all_reduce_t all_reduce; // all reduce function
  gkyl_array_sync_t gkyl_array_sync; // sync array
  gkyl_array_per_sync_t gkyl_array_per_sync; // sync array in periodic dirs
  barrier_t barrier; // barrier

  gkyl_array_write_t gkyl_array_write; // array output
  extend_comm_t extend_comm; // extend communicator
  split_comm_t split_comm; // split communicator.

  comm_state_new_t comm_state_new; // Allocate a new state object.
  comm_state_release_t comm_state_release; // Free a state object.
  comm_state_wait_t comm_state_wait; // Wait for a request to complete.

  comm_group_call_start_t comm_group_call_start; // Start a group call.
  comm_group_call_end_t comm_group_call_end; // End a group call.

  struct gkyl_ref_count ref_count; // reference count
};

/**
 * Get rank of communicator.
 *
 * @param comm Communicator
 * @param rank On output, the rank
 * @return error code: 0 for success
 */
static int
gkyl_comm_get_rank(struct gkyl_comm *comm, int *rank)
{
  return comm->get_rank(comm, rank);
}

/**
 * Get number of ranks in communicator
 *
 * @param comm Communicator
 * @param rank On output, the rank
 * @return error code: 0 for success
 */
static int
gkyl_comm_get_size(struct gkyl_comm *comm, int *sz)
{
  return comm->get_size(comm, sz);
}

/**
 * Blocking send a gkyl array to another process.
 * @param comm Communicator.
 * @param array Array to send.
 * @param dest MPI rank we are sending to.
 * @param tag MPI tag.
 * @return error code: 0 for success
 */
static int
gkyl_comm_array_send(struct gkyl_comm *comm, struct gkyl_array *array,
  int dest, int tag)
{
  return comm->gkyl_array_send(array, dest, tag, comm);
}

/**
 * Nonblocking send a gkyl array to another process.
 * @param comm Communicator.
 * @param array Array to send.
 * @param dest MPI rank we are sending to.
 * @param tag MPI tag.
 * @param state Comm state object to check (e.g. if comm finished).
 * @return error code: 0 for success
 */
static int
gkyl_comm_array_isend(struct gkyl_comm *comm, struct gkyl_array *array,
  int dest, int tag, struct gkyl_comm_state *state)
{
  return comm->gkyl_array_isend(array, dest, tag, comm, state);
}

/**
 * Blocking recv a gkyl array from another process.
 * @param comm Communicator.
 * @param array Array to receive into.
 * @param src MPI rank we are receiving from. 
 * @param tag MPI tag.
 * @return error code: 0 for success
 */
static int
gkyl_comm_array_recv(struct gkyl_comm *comm, struct gkyl_array *array,
  int src, int tag)
{
  return comm->gkyl_array_recv(array, src, tag, comm);
}

/**
 * Nonblocking recv a gkyl array from another process.
 * @param comm Communicator.
 * @param array Array to receive into.
 * @param src MPI rank we are receiving from. 
 * @param tag MPI tag.
 * @param state Comm state object to check (e.g. if comm finished).
 * @return error code: 0 for success
 */
static int
gkyl_comm_array_irecv(struct gkyl_comm *comm, struct gkyl_array *array,
  int src, int tag, struct gkyl_comm_state *state)
{
  return comm->gkyl_array_irecv(array, src, tag, comm, state);
}

/**
 * All reduce values across domains.
 *
 * @param comm Communicator
 * @param type Data-type of element
 * @param op Operator to use in reduction
 * @param nelem Number of elemets in inp and out
 * @param inp Local values on domain
 * @param out Reduced values
 * @return error code: 0 for success
 */
static int
gkyl_comm_all_reduce(struct gkyl_comm *comm, enum gkyl_elem_type type,
  enum gkyl_array_op op, int nelem, const void *inp, void *out)
{
  return comm->all_reduce(comm, type, op, nelem, inp, out);
}

/**
 * Synchronize array across domain.
 *
 * @param comm Communicator
 * @param local Local range for array: sub-range of local_ext
 * @param local_ext Extended range, i.e. range over which array is defined
 * @param array Array to synchronize
 * @return error code: 0 for success
 */
static int gkyl_comm_array_sync(struct gkyl_comm *comm,
  const struct gkyl_range *local,
  const struct gkyl_range *local_ext,
  struct gkyl_array *array)
{
  comm->barrier(comm);
  return comm->gkyl_array_sync(comm, local, local_ext, array);
}

/**
 * Synchronize array across domain in periodic directions.
 *
 * @param comm Communicator
 * @param local Local range for array: sub-range of local_ext
 * @param local_ext Extended range, i.e. range over which array is defined
 * @param nper_dirs Number of periodic directions
 * @param per_dirs Directions that are periodic
 * @param array Array to synchronize
 * @return error code: 0 for success
 */
static int gkyl_comm_array_per_sync(struct gkyl_comm *comm,
  const struct gkyl_range *local,
  const struct gkyl_range *local_ext,
  int nper_dirs, const int *per_dirs,
  struct gkyl_array *array)
{
  comm->barrier(comm);
  return comm->gkyl_array_per_sync(comm, local, local_ext,
    nper_dirs, per_dirs, array);
}

/**
 * Barrier across domains
 *
 * @param comm Communcator
 * @return error code: 0 for success
 */
static int
gkyl_comm_barrier(struct gkyl_comm *comm)
{
  return comm->barrier(comm);
}

/**
 * Create a new comm request/status pair.
 * @return request/status struct.
 */
static struct gkyl_comm_state*
gkyl_comm_state_new(struct gkyl_comm *comm)
{
  return comm->comm_state_new(comm);
}

/**
 * Free memory associate with a comm state object.
 */
static void
gkyl_comm_state_release(struct gkyl_comm *comm, struct gkyl_comm_state *state)
{
  comm->comm_state_release(state);
}

/**
 * Wait for a request to complete.
 */
static void
gkyl_comm_state_wait(struct gkyl_comm *comm, struct gkyl_comm_state *state)
{
  comm->comm_state_wait(state);
}

/**
 * Start and end a group call (e.g. in NCCL).
 */
static void
gkyl_comm_group_call_start(struct gkyl_comm *comm)
{
  comm->comm_group_call_start();
}
static void
gkyl_comm_group_call_end(struct gkyl_comm *comm)
{
  comm->comm_group_call_end();
}

/**
 * Write out grid and array data to file in .gkyl format so postgkyl
 * can understand it.
 *
 * @param comm Communicator
 * @param grid Grid object to write
 * @param range Range describing portion of the array to output.
 * @param arr Array object to write
 * @param fname Name of output file (include .gkyl extension)
 * @return Status flag: 0 if write succeeded, 'errno' otherwise
 */
static int
gkyl_comm_array_write(struct gkyl_comm *comm,
  const struct gkyl_rect_grid *grid, const struct gkyl_range *range,
  const struct gkyl_array *arr, const char *fname)
{
  return comm->gkyl_array_write(comm, grid, range, arr, fname);
}

/**
 * Create a new communicator that extends the communicator to work on a
 * extended domain specified by erange. (Each range handled by the
 * communicator is extended by a tensor-product with erange). The
 * returned communicator must be freed by calling gkyl_comm_release.
 *
 * @param comm Communicator
 * @param erange Range to extend by
 * @return Newly created communicator
 */
static struct gkyl_comm*
gkyl_comm_extend_comm(const struct gkyl_comm *comm,
  const struct gkyl_range *erange)
{
  return comm->extend_comm(comm, erange);
}

/**
 * Create a new communicator that extends the communicator to work on a
 * extended domain specified by erange. (Each range handled by the
 * communicator is extended by a tensor-product with erange). The
 * returned communicator must be freed by calling gkyl_comm_release.
 *
 * @param comm Communicator.
 * @param color All ranks of same color will share a communicator.
 * @param new_decomp Decomp object to associate with the new communicator.
 * @return Newly created communicator
 */
static struct gkyl_comm*
gkyl_comm_split_comm(const struct gkyl_comm *comm, int color,
  struct gkyl_rect_decomp *new_decomp)
{
  return comm->split_comm(comm, color, new_decomp);
}

/**
 * Acquire pointer to communicator
 *
 * @param comm Communicator to to get acquire
 * @return Acquired comm obj pointer
 */
struct gkyl_comm* gkyl_comm_acquire(const struct gkyl_comm *comm);

/**
 * Release communicator memory.
 *
 * @param comm Communicator to release
 */
void gkyl_comm_release(const struct gkyl_comm *comm);
