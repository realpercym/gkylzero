#include <acutest.h>
#include <gkyl_alloc.h>
#include <stddef.h>

void
test_aligned_alloc()
{
  int *d1 = gkyl_aligned_alloc(8, 100*sizeof(int));
  TEST_CHECK( (ptrdiff_t) d1 % 8 == 0 );
  gkyl_aligned_free(d1);

  int *d2 = gkyl_aligned_alloc(16, 100*sizeof(int));
  TEST_CHECK( (ptrdiff_t) d2 % 16 == 0 );
  gkyl_aligned_free(d2);

  int *d3 = gkyl_aligned_alloc(32, 100*sizeof(int));
  TEST_CHECK( (ptrdiff_t) d3 % 32 == 0 );
  gkyl_aligned_free(d3);

  int *d4 = gkyl_aligned_alloc(64, 100*sizeof(int));
  TEST_CHECK( (ptrdiff_t) d4 % 64 == 0 );
  gkyl_aligned_free(d4);
}

void
test_aligned_realloc()
{
  int n = 10;
  int *d = gkyl_aligned_alloc(16, n*sizeof(int));

  for (int i=0; i<n; ++i)
    d[i] = 2*i;

  int *rd = gkyl_aligned_realloc(d, 16, n*sizeof(int), 2*n*sizeof(int));

  TEST_CHECK( (ptrdiff_t) rd % 16 == 0 );

  for (int i=0; i<n; ++i)
    TEST_CHECK( rd[i] == 2*i );

  gkyl_aligned_free(rd);
}

void test_mem_buff(bool use_gpu)
{
  // Create a new memory buffer.
  const int nelem = 7;
  gkyl_mem_buff mbuff = use_gpu? gkyl_mem_buff_cu_new(nelem * sizeof(double))
                               : gkyl_mem_buff_new(nelem * sizeof(double));

  // Check its size.
  TEST_CHECK(gkyl_mem_buff_size(mbuff) == nelem*sizeof(double));

  // Assign some values.
  double test_vals[nelem];
  for (int i=0; i<nelem; i++)
    test_vals[i] = 2.0*i + i*i/2.0;
  
  double *mbuff_p = (double *) gkyl_mem_buff_data(mbuff);
  if (use_gpu)
    gkyl_cu_memcpy(mbuff_p, test_vals, nelem*sizeof(double), GKYL_CU_MEMCPY_H2D);
  else {
    for (int i=0; i<nelem; i++)
      mbuff_p[i] = test_vals[i];
  }

  // Check assigned values.
  double *mbuff_p2_ho = (double *) gkyl_mem_buff_data(mbuff);
  if (use_gpu) {
    double *mbuff_p2 = (double *) gkyl_mem_buff_data(mbuff);
    size_t sz = gkyl_mem_buff_size(mbuff);
    mbuff_p2_ho = gkyl_malloc(sz);
    gkyl_cu_memcpy(mbuff_p2_ho, mbuff_p2, sz, GKYL_CU_MEMCPY_D2H);
  }
  for (int i=0; i<nelem; i++)
    TEST_CHECK(mbuff_p2_ho[i] == test_vals[i]);

  if (use_gpu)
    gkyl_free(mbuff_p2_ho);

  // Resize.
  const int nelem_new = nelem*3;
  mbuff = gkyl_mem_buff_resize(mbuff, nelem_new * sizeof(double));

  // Assign new values.
  double test_vals_new[nelem_new-nelem];
  for (int i=nelem; i<nelem_new; i++)
    test_vals_new[i-nelem] = 3.0*i + i*i/3.0;

  mbuff_p = (double *) gkyl_mem_buff_data(mbuff);
  if (use_gpu)
    gkyl_cu_memcpy(mbuff_p+nelem, test_vals_new, (nelem_new-nelem)*sizeof(double), GKYL_CU_MEMCPY_H2D);
  else {
    for (int i=nelem; i<nelem_new; i++)
      mbuff_p[i] = test_vals_new[i-nelem];
  }

  // Check all assigned values.
  mbuff_p2_ho = (double *) gkyl_mem_buff_data(mbuff);
  if (use_gpu) {
    double *mbuff_p2 = (double *) gkyl_mem_buff_data(mbuff);
    size_t sz = gkyl_mem_buff_size(mbuff);
    mbuff_p2_ho = gkyl_malloc(sz);
    gkyl_cu_memcpy(mbuff_p2_ho, mbuff_p2, sz, GKYL_CU_MEMCPY_D2H);
  }
  for (int i=0; i<nelem; i++)
    TEST_CHECK( mbuff_p2_ho[i] == test_vals[i] );
  for (int i=nelem; i<nelem_new; i++)
    TEST_CHECK( mbuff_p2_ho[i] = test_vals_new[i-nelem] );

  if (use_gpu)
    gkyl_free(mbuff_p2_ho);

  gkyl_mem_buff_release(mbuff);
}

void test_mem_buff_ho(){ test_mem_buff(false); }

#ifdef GKYL_HAVE_CUDA
void test_mem_buff_dev(){ test_mem_buff(true); }
#endif

TEST_LIST = {
  { "aligned_alloc", test_aligned_alloc },
  { "aligned_realloc", test_aligned_realloc },
  { "mem_buff_ho", test_mem_buff_ho },
#ifdef GKYL_HAVE_CUDA
  { "mem_buff_dev", test_mem_buff_dev },
#endif
  { NULL, NULL },
};
