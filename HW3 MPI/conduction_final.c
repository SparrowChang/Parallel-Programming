#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char **argv) {
  int N, seed;
  N = atoi(argv[1]);
  seed = atoi(argv[2]);
  srand(seed);
  int temp[N][N];

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      temp[i][j] = random() >> 3; // avoid overflow
    }
  }
  int count = 0, balance = 0;
  int next[N][N];
  while (!balance) {
    count++;
    balance = 1;
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        int up = i - 1 < 0 ? 0 : i - 1;
        int down = i + 1 >= N ? i : i + 1;
        int left = j - 1 < 0 ? 0 : j - 1;
        int right = j + 1 >= N ? j : j + 1;
        next[i][j] = (temp[i][j] + temp[up][j] + temp[down][j] +
            temp[i][left] + temp[i][right]) / 5;
        if (next[i][j] != temp[i][j]) {
          balance = 0;
        }
      }
    }
    int main ( int argc , char *argv [ ] )
{
	// Get the number of processes, world_size; Get the rank of the process, world_rank
	int world_size, world_rank , next , last , buf[2];
	MPI_Request reqs[2];
	MPI_Status stats[2];
	MPI_Init( &argc , &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	
	last = world_rank-1;
	next = world_rank+1;
	
	if ( world_rank == 0 ) last = world_size-1;
	if ( world_rank == world_size-1 ) next = 0;
	
	MPI_Irecv(&buf[1] , 1 , MPI_INT , last , 0 , MPI_COMM_WORLD, &reqs [1] );
	MPI_Isend(&world_rank , 1 , MPI_INT , next , 0 , MPI_COMM_WORLD, &reqs [2] );
	
	MPI_Waitall ( 2 , reqs , stats );
	
	printf("Task %d communicated with tasks %d & %d\n" , world_rank , last , next );
	MPI_Finalize();
	return 0;
}
    memcpy(temp, next, N * N * sizeof(int));
  }
  printf("Size: %d*%d, Seed: %d, ", N, N, seed);
  printf("Iteration: %d, Temp: %d\n", count, temp[0][0]);
  return 0;
}

