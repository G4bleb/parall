#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void fusion(int U[], int V[], int T[], int n, int m) {
    
    int i = 0, j = 0, k;
    for (k = 0; k < m + n; k++) {
        if ((U[i] < V[j] || j == m) && i != n) {
            T[k] = U[i++];
        } else {
            T[k] = V[j++];
        }
    }
}

int printTab(int T[], int n) {
    printf("[");
    int i;
    for (i = 0; i < n; i++) {
        printf("%d ", T[i]);
    }
    printf("]\n");
}


int main() {
    //printf("Please enter n followed by two sorted arrays of size n : ");
    int n;
    scanf("%d", &n);
    n = 2 * n;
    int *T = malloc(n * sizeof(int));
    int i;
    for (i = 0; i < n; i++) {
        scanf("%d", &T[i]);
    }

    double start;
    double end;
    int sizeU = n / 2;
    if (sizeU * 2 != n) {
        sizeU += 1;
    }
    int *U = malloc(sizeU * sizeof(int));
    for (i = 0; i < sizeU; i++) {
        U[i] = T[i];
    }
    int *V = malloc(n / 2 * sizeof(int));
    for (i = sizeU; i < n; i++) {
        V[i - sizeU] = T[i];
    }
    fusion(U, V, T, sizeU, n / 2);
    start = omp_get_wtime();
    fusion(U, V, T, sizeU, n / 2);
    end = omp_get_wtime();
    //printTab(T, n);
    printf("%g\n", end - start);
    free(T);
    free(U);
    free(V);
    return 0;
}