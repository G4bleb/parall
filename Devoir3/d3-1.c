#include "MyMPI.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define DEVOIR3_MPI_TAG_COUPLE 1
#define DEVOIR3_MPI_TAG_OVER 2
#define DEVOIR3_MPI_TAG_FIRST_PRIME 3

int main(int argc, char *argv[]) {
    int id, p, n, low_value, high_value, size, proc0_size, i, index, prime,
        first, count, global_count;
    double elapsed_time;
    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 2) {
        if (!id)
            printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }
    n = atoi(argv[1]);
    low_value = 2 + BLOCK_LOW(id, p, n - 1);
    // printf("Process %d starts at %d\n", id, low_value);
    high_value = 2 + BLOCK_HIGH(id, p, n - 1);
    size = BLOCK_SIZE(id, p, n - 1);
    proc0_size = (n - 1) / p;
    if ((2 + proc0_size) < (int)sqrt((double)n) || size < 2) {
        if (!id)
            printf("Too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    char *marked = (char *)malloc((size_t)size);
    if (marked == NULL) {
        printf("Cannot allocate enough memory for marked\n");
        MPI_Finalize();
        exit(1);
    }

    for (i = 0; i < size; i++)
        marked[i] = 0;
    if (!id)
        index = 0;
    prime = 2;
    do {
        if (prime * prime > low_value)
            first = prime * prime - low_value;
        else {
            if (!(low_value % prime))
                first = 0;
            else
                first = prime - (low_value % prime);
        }
        for (i = first; i < size; i += prime)
            marked[i] = 1;
        if (!id) {
            while (marked[++index])
                ;
            prime = index + 2;
        }
        MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } while (prime * prime <= n);
    count = 0;
    for (i = 0; i < size; i++)
        if (!marked[i])
            count++;

    //Construction d'un tableau des premiers locaux
    int *localPrimes = malloc(count * sizeof(int));
    if (localPrimes == NULL) {
        printf("Cannot allocate enough memory for localPrimes\n");
        MPI_Finalize();
        exit(1);
    }
    int j = 0;
    for (i = 0; i < size; i++) {
        if (!marked[i]) {
            localPrimes[j] = i + low_value;
            j++;
        }
    }

    
    int *globalTwins = NULL;
    if (!id) {
        globalTwins = malloc(size * sizeof(int));
        if (globalTwins == NULL) {
            printf("Cannot allocate enough memory for globalTwins\n");
            MPI_Finalize();
            exit(1);
        }
    }

    int twins[2];
    int twinsIndex = 0;
    i = 0;
    // Recherche des jumeaux
    for (j = 0; j < count - 1; j++) {
        if (localPrimes[j] + 2 == localPrimes[j + 1]) {
            // printf("Process %d Found couple : (%d, %d)\n", id,
            // localPrimes[j],localPrimes[j + 1]);
            twins[0] = localPrimes[j];
            twins[1] = localPrimes[j + 1];
            if (id) {
                //Envoi des jumeaux au ps 0
                MPI_Send(twins, 2, MPI_INT, 0, DEVOIR3_MPI_TAG_COUPLE,
                         MPI_COMM_WORLD);
            } else {
                globalTwins[twinsIndex++] = twins[0];
                globalTwins[twinsIndex++] = twins[1];
            }
        }
    }

    //Vérification des jumeaux aux extrémités des blocs:

    // On reçoit le premier premier du bloc suivant, qu'on compare avec notre dernier premier
    if (id < p - 1) {
        MPI_Recv(&i, 1, MPI_INT, id + 1, DEVOIR3_MPI_TAG_FIRST_PRIME,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (localPrimes[count - 1] + 2 == i) {
            twins[0] = localPrimes[count - 1];
            twins[1] = i;
            if (id) {
                MPI_Send(twins, 2, MPI_INT, 0, DEVOIR3_MPI_TAG_COUPLE,
                         MPI_COMM_WORLD);
            } else {
                globalTwins[twinsIndex++] = twins[0];
                globalTwins[twinsIndex++] = twins[1];
            }
        }
    }
    // On envoie les premier premier d'un bloc aux blocs précédents
    if (id) {
        MPI_Send(&localPrimes[0], 1, MPI_INT, id - 1,
                 DEVOIR3_MPI_TAG_FIRST_PRIME, MPI_COMM_WORLD);
    }

    //Compte des premiers
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (id) {
        //On envoie 0 pour signaler au ps 0 que l'envoi est terminé
        //On envoie twinsIndex car on sait qu'il est égal à 0 pour tout id > 0
        MPI_Send(&twinsIndex, 1, MPI_INT, 0, DEVOIR3_MPI_TAG_OVER,
                 MPI_COMM_WORLD);
    }

    //Réception des jumeaux
    if (!id) {
        globalTwins = realloc(globalTwins, global_count * sizeof(int));
        int number_amount;
        int buffer[2];
        for (i = 1; i < p; i++) {
            do {
                MPI_Recv(buffer, 2, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD,
                         MPI_STATUS_IGNORE);
                if (buffer[0]) {
                    globalTwins[twinsIndex++] = buffer[0];
                    globalTwins[twinsIndex++] = buffer[1];
                }
            } while (buffer[0] != 0);
        }
    }

    elapsed_time += MPI_Wtime();
    if (!id) {
        for (i = 0; i < twinsIndex; i += 2) {
            printf("(%d, %d) ", globalTwins[i], globalTwins[i + 1]);
        }
        printf("\n");
        // printf("%d primes are less than or equal to %d\n", global_count, n);
        printf("Total elapsed time: %10.6f\n", elapsed_time);
        free(globalTwins);
    }
    free(localPrimes);
    free(marked);
    MPI_Finalize();
    return 0;
}
