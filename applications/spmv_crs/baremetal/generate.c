#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "spmv.h"
#include "494_bus_full.h"

#define ROW 0
#define COL 1

// Row first, then column comparison
int compar(const void *v_lhs, const void *v_rhs)
{
  int row_lhs = ((int *)v_lhs)[ROW];
  int row_rhs = ((int *)v_rhs)[ROW];
  int col_lhs = ((int *)v_lhs)[COL];
  int col_rhs = ((int *)v_rhs)[COL];

  if( row_lhs==row_rhs ) {
    if( col_lhs==col_rhs )
      return 0;
    else if( col_lhs<col_rhs )
      return -1;
    else
      return 1;
  } else if( row_lhs<row_rhs ) {
    return -1;
  } else {
    return 1;
  }
}

void generate_data(struct bench_args_float_t* data)
{
  char *current, *next, *buffer;
  int status, i, nbytes;
  int coords[NNZ][2]; // row, col
  struct prng_rand_t state;

  // Parse data.
  nbytes = strlen(bus_494_full);
  buffer = (char*) malloc(nbytes);
  strncpy(buffer, bus_494_full, nbytes);
  current = buffer;
  next = strchr(current, '\n');// skip first two lines
  *next = (char)0;
  current = next+1;
  for(i=0; i<NNZ; i++) {
    next = strchr(current, '\n');
    *next = (char)0;
    current = next+1;
#if TYPE == float
    status = sscanf(current, "%d %d %f", &coords[i][ROW], &coords[i][COL], &data->val[i]);
#elif TYPE == double
    status = sscanf(current, "%d %d %lf", &coords[i][ROW], &coords[i][COL], &data->val[i]);
#else
#error "Bad value of TYPE"
#endif
    assert(status==3 && "Parse error in matrix file");
  }

  // Sort by row
  qsort(coords, NNZ, 2*sizeof(int), &compar);

  // Fill data structure
  for(i=0; i<NNZ; i++)
    data->cols[i] = (TYPE)coords[i][COL]-1;
  memset(data->rowDelimiters, 0, (N+1)*sizeof(int));
  for(i=0; i<NNZ; i++)
    data->rowDelimiters[coords[i][ROW]-1+1] += (TYPE)1; // count
    // (-1 because matrix is 1-indexed, +1 because it's counting cells before it)
  for(i=1; i<N+1; i++)
    data->rowDelimiters[i] += (TYPE)(data->rowDelimiters[i-1]); // scan

  // Set vector
  prng_srand(1,&state);
  for( i=0; i<N; i++ )
    data->vec[i] = ((TYPE)prng_rand(&state))/((TYPE)PRNG_RAND_MAX);

}
