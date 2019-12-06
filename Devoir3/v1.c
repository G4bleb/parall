/*
 *   Floyd's all-pairs shortest path
 *
 *   Given an NxN matrix of distances between pairs of
 *   vertices, this MPI program computes the shortest path
 *   between every pair of vertices.
 *
 *   Initially programmed by Michael J. Quinn
 *   Edited by Gabriel Lebis
 *   Last modification: 24 nov 2019
 */

#include "MyMPI.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

typedef int dtype;
#define MPI_TYPE MPI_INT

void devoir3_read_row_striped_matrix(char *s,        /* IN - File name */
                             int **mat,         /*OUT - full matrix (for one process)*/
                             dtype **storage, /* OUT - Submatrix stored here */
                             int *m,             /* OUT - Matrix rows */
                             int *n,             /* OUT - Matrix cols */
                             MPI_Comm comm,     /* IN - Communicator */
                             MPI_Win *winptr     /* OUT - window*/
                             )

{
    int datum_size; /* Size of matrix element */
    int i;
    int id;            /* Process rank */
    FILE *infileptr;   /* Input file pointer */
    int local_rows;    /* Rows on this proc */
    int p;             /* Number of processes */

    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &id);
    datum_size = sizeof(dtype);

    /* Process p-1 opens file, reads size of matrix,
       and broadcasts matrix dimensions to other procs */

    if (id == (p - 1)) {
        infileptr = fopen(s, "r");
        if (infileptr == NULL)
            *m = 0;
        else {
            // fread(m, (size_t)datum_size, 1, infileptr);
            // fread(n, (size_t)datum_size, 1, infileptr);
            fscanf(infileptr, "%d\n", n);
            *m = *n;
        }
    }
    MPI_Bcast(m, 1, MPI_INT, p - 1, comm);

    if (!(*m))
        MPI_Abort(MPI_COMM_WORLD, OPEN_FILE_ERROR);

    MPI_Bcast(n, 1, MPI_INT, p - 1, comm);

    local_rows = BLOCK_SIZE(id, p, *m);

    /* Allocate local part of matrix.*/

    *storage = (void *)malloc((size_t)(local_rows * *n * datum_size));
    if (*storage == NULL) {
        printf("Error: Malloc failed for process %d\n", id);
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }

    /* Process p-1 reads blocks of rows from file and
       creates a window */

    MPI_Alloc_mem(*n * *n * datum_size, MPI_INFO_NULL, mat);
    MPI_Win_create(*mat, *n * *n, datum_size, MPI_INFO_NULL, comm, winptr);

    int j;
    if (id == (p - 1)) {
        // fread(*mat, (size_t)datum_size, (size_t)(*n * *n), infileptr);
        for (i = 0; i < *n; i++) {
            for (j = 0; j < *m; j++) {
                fscanf(infileptr, "%d ", &((*mat)[i * *n + j]));
                fflush(stdout);
            }
        }
        fclose(infileptr);
    }

    MPI_Win_fence(0, *winptr);

    if (id == (p - 1)) {
        //Last process gets its rows
        for (i = 0; i < local_rows * *n; i++) {
            (*storage)[i] = (*mat)[*n * *n - local_rows * *n + i];
        }
    }else{
        //Every other process gets their rows
        MPI_Get(*storage, local_rows * *n, MPI_TYPE, p - 1,
                local_rows * *n * id, *n * *n, MPI_TYPE, *winptr);
    }

    MPI_Win_fence(0, *winptr);
}

//-----------------------------------------------------------
void devoir3_compute_shortest_paths(int id, int p, dtype *mat, dtype *storage,
                                    int n, MPI_Win win) {
    int i, j, k;
    int local_rows = BLOCK_SIZE(id, p, n); // Nb of rows
    int *tmp; /* Holds the gotten row */

    tmp = (dtype *)malloc((size_t)(n * ((int)sizeof(dtype)))); // Holds one row
    for (k = 0; k < n; k++){
        if(id == p-1){
            // Last process gets its row
            for (i = 0; i < n; i++) {
                tmp[i] = mat[k * n + i];
            }
        }else{
            // Every other process gets its row
            MPI_Get(tmp, n, MPI_TYPE, p - 1, k * n, n * n, MPI_TYPE, win);
        }
        MPI_Win_fence(0, win);
        for (i = 0; i < local_rows; i++) {
            for (j = 0; j < n; j++) {
                storage[i * n + j] = MIN(storage[i * n + j],
                                         storage[i * n + k] + tmp[j]);
            }
        }
    }
    
    
    if (id == p - 1) {
        for (i = 0; i < local_rows * n; i++) {
            mat[n * n - local_rows * n + i] = storage[i];
        }
    } else {
        MPI_Put(storage, local_rows * n, MPI_TYPE, p - 1, local_rows * n * id,
                n * n, MPI_TYPE, win);
    }
    MPI_Win_fence(0, win);
}

//--------------------------------------------------
int main(int argc, char *argv[]) {
    dtype *storage; /* Local portion of array elements */
    int i, j;
    int id; /* Process rank */
    int m;  /* Rows in matrix */
    int n;  /* Columns in matrix */
    int p;  /* Number of processes */
    double time, max_time;
    MPI_Win win;
    int *mat = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    devoir3_read_row_striped_matrix(argv[1], &mat, &storage,
                                    &m, &n, MPI_COMM_WORLD, &win);

    if (m != n){
        printf("Matrix must be square\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    time = -MPI_Wtime();
    devoir3_compute_shortest_paths(id, p, mat, storage, n, win);
    time += MPI_Wtime();
    MPI_Reduce(&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (id == p-1) {
        printf("Floyd, matrix size %d, %d processes: %10.6f seconds\n", n, p,
               max_time);
        for (i = 0; i < n; i++){
            for (j = 0; j < n; j++){
                printf("%d\t", mat[i*n+j]);
            }
            printf("\n");
        }
        
    }
    MPI_Win_free(&win);
    MPI_Finalize();
    return 0;
}

