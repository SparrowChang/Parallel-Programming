#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#define RANK_FIRST 0

int main(int argc, char *argv[]) {
    MPI_Status status;
    MPI_Request sendreq, recvreq1, recvreq2;
    int size, rank, seed;
    int balance = 0, final_balance = 0;
    int count = 0;
    int start_reduce = 0;
    int istart, iend, before_start, before_end, irange, iremain;
    int i, j;
    int finish;
    int buf_ID_now = 0;
    int buf_ID_next = 1;
    
    MPI_Init(&argc, &argv);

    int N = atoi(argv[1]);
    int real_boundary = N + 2;
    int virtual_boundary = N + 1;
    int buf[2][real_boundary][real_boundary];
    
    seed = atoi(argv[2]);
    srand(seed);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rank_last = size - 1;
    int rank_prev = rank - 1;
    int rank_next = rank + 1;
    
    irange = N / size;
    iremain = N % size;
    
    if (rank < iremain) {
        istart = irange * rank + 1 + rank;
        iend = istart + irange + 1;
    }
    else {
        istart = irange * rank + 1 + iremain;
        iend = istart + irange;
    }
    
    before_start = istart - 1;
    before_end = iend - 1;
    
    for (i = 1; i < N + 1; i++) {
        for (j = 1; j < N + 1; j++) {
            buf[buf_ID_now][i][j] = random() >> 3; 
        }
    }

    for (i = istart; i < iend; i++) {
        memcpy(&buf[buf_ID_now][i][0], &buf[buf_ID_now][i][1], sizeof(int));
        memcpy(&buf[buf_ID_now][i][virtual_boundary], &buf[buf_ID_now][i][N], sizeof(int));
    }
    
    if (rank == RANK_FIRST) {
        memcpy(&buf[buf_ID_now][0][1], &buf[buf_ID_now][1][1], N * sizeof(int));
    }
    
    if (rank == rank_last) {
        memcpy(&buf[buf_ID_now][virtual_boundary][1], &buf[buf_ID_now][N][1], N * sizeof(int));
    }

    while (1) {
        count++;

        for (i = istart; i < iend; i++) {
            for (j = 1; j < virtual_boundary; j++) {
                buf[buf_ID_next][i][j] = (buf[buf_ID_now][i][j] + buf[buf_ID_now][i - 1][j] + buf[buf_ID_now][i + 1][j] + buf[buf_ID_now][i][j - 1] + buf[buf_ID_now][i][j + 1]) / 5;
            }
        }

        if (rank != rank_last) {
            MPI_Isend(&buf[buf_ID_next][before_end][1], N, MPI_INT, rank_next, 0, MPI_COMM_WORLD, &sendreq);
        }
        
        if (rank != RANK_FIRST) {
            MPI_Irecv(&(buf[buf_ID_next][before_start][1]), N, MPI_INT, rank_prev, 0, MPI_COMM_WORLD, &recvreq1);
            MPI_Isend(&(buf[buf_ID_next][istart][1]), N, MPI_INT, rank_prev, 0, MPI_COMM_WORLD, &sendreq);
        }
        
        if (rank != rank_last) {
            MPI_Irecv(&(buf[buf_ID_next][iend][1]), N, MPI_INT, rank_next, 0, MPI_COMM_WORLD, &recvreq2);
        }
        
        for (i = istart; i < iend; i++) {
            memcpy(&buf[buf_ID_next][i][0], &buf[buf_ID_next][i][1], sizeof(int));
            memcpy(&buf[buf_ID_next][i][virtual_boundary], &buf[buf_ID_next][i][N], sizeof(int));
        }

        if (rank == RANK_FIRST) {
            memcpy(&buf[buf_ID_next][0][1], &buf[buf_ID_next][1][1], N * sizeof(int));
        }

        if (rank == rank_last) {
            memcpy(&buf[buf_ID_next][virtual_boundary][1], &buf[buf_ID_next][N][1], N * sizeof(int));
        }
        
        if (0 != memcmp(&buf[buf_ID_next][istart][1],
                        &buf[buf_ID_now][istart][1],
                        real_boundary * (iend - istart) * sizeof(int) - 2 * sizeof(int))) {
            balance = 0;
        }
        else {
            balance = 1;
        }
        
        start_reduce = balance;
        MPI_Bcast(&start_reduce, 1, MPI_INT, RANK_FIRST, MPI_COMM_WORLD);
        
        if (1 == start_reduce) {
            MPI_Allreduce(&balance, &final_balance, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            if (final_balance == size) {
                if (RANK_FIRST == rank) {
                    printf("Size: %d*%d, Seed: %d, ", N, N, seed);
                    printf("Iteration: %d, Temp: %d\n", count, buf[buf_ID_next][1][1]);
                }
                break;
            }
        }
        
        if (1 == buf_ID_now) {
            buf_ID_now = 0;
            buf_ID_next = 1;
        }
        else {
            buf_ID_now = 1;
            buf_ID_next = 0;
        }
        
        if (rank != RANK_FIRST) {
            MPI_Wait(&recvreq1, MPI_STATUS_IGNORE);
        }
        
        if (rank != rank_last) {
            MPI_Wait(&recvreq2, MPI_STATUS_IGNORE);
        }
    }
    
    MPI_Finalize();
    return 0;
}

