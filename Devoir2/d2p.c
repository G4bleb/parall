#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 1

void swap(int *a, int *b) {
    *b = *b + *a;
    *a = *b - *a;
    *b = *b - *a;
}

int recherche_dichotomique(int key, int T[], int lo, int hi) {
    hi++;
    int index = lo;
    char found = 0;
    while (!found && lo < hi) {
        int mid = lo + ((hi - lo) / 2);
        if (T[mid] == key) {
            found = 1;
            index = mid;
        } else if (T[mid] > key) {
            hi = mid;
            index = hi;
        } else {
            lo = mid + 1;
            index = lo;
        }
    }
    return index;
}

void P_Fusion(int T[], int p1, int r1, int p2, int r2, int A[], int p3) {
    int q1, q2, q3;
    int n1 = r1 - p1 + 1;
    int n2 = r2 - p2 + 1;
    if (n1 < n2) {
        swap(&n1, &n2);
        swap(&p1, &p2);
        swap(&r1, &r2);
    }
    if (n1 > 0) {
        q1 = (p1 + r1) / 2;
        q2 = recherche_dichotomique(T[q1], T, p2, r2);
        q3 = p3 + (q1 - p1) + (q2 - p2);
        A[q3] = T[q1];
        #pragma omp task
        P_Fusion(T, p1, q1 - 1, p2, q2 - 1, A, p3);
        #pragma omp task
        P_Fusion(T, q1 + 1, r1, q2, r2, A, q3 + 1);
    }
}

int printTab(int T[], int n) {
    printf("[");
    int i;
    for (i = 0; i < n; i++) {
        printf(" %d", T[i]);
    }
    printf(" ]\n");
}

int main() {
    // printf("Please enter n followed by two sorted arrays of size n : ");
    int n;
    scanf("%d", &n);
    n = 2 * n;
    int *T = malloc(n*sizeof(int));
    int i;
    for (i = 0; i < n; i++) {
        scanf("%d", &T[i]);
    }
    int *B =  malloc(n*sizeof(int));

    omp_set_num_threads(NUM_THREADS);
    double start;
    double end;
    start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
        P_Fusion(T, 0, n / 2 - 1, n / 2, n - 1, B, 0);
        }
        #pragma omp taskwait
    }
    
    end = omp_get_wtime();
    // printTab(B, n);
    printf("%g\n", end - start);
    free(T);
    free(B);

    return 0;
}