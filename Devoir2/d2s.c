#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void swap(int *a, int *b) {
    *b = *b + *a;
    *a = *b - *a;
    *b = *b - *a;
}

void adhoc(int T[]) {
    if (T[1] < T[0]) {
        swap(&T[0], &T[1]);
    }
}

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

void tri_fusion(int T[], int n) {
    if (n <= 2) {
        //printf("n petit, n = %d\n", n);
        if (n == 2) {
            adhoc(T);
        }
    } else {
        int sizeU = n / 2;
        if (sizeU * 2 != n) {
            sizeU += 1;
        }
        int *U = malloc(sizeU * sizeof(int));
        int i;
        for (i = 0; i < sizeU; i++) {
            U[i] = T[i];
        }
        int *V = malloc(n / 2 * sizeof(int));
        for (i = sizeU; i < n; i++) {
            V[i - sizeU] = T[i];
        }
        tri_fusion(U, sizeU);
        tri_fusion(V, n / 2);
        fusion(U, V, T, sizeU, n / 2);
        free(U);
        free(V);
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
    start = omp_get_wtime();
    tri_fusion(T, n);
    end = omp_get_wtime();
    // printTab(T, n);
    printf("%g\n", end - start);
    free(T);
    return 0;
}