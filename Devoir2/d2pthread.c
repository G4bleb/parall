#include <pthread.h>
#include <stdio.h>

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

typedef struct P_FusionArgs
{
    int *T, p1, r1, p2, r2, *A, p3;
} P_FusionArgs;

void * P_Fusion(void * ptargs) {
    P_FusionArgs * args = ptargs;
    int *T = args->T;
    int p1 = args->p1;
    int r1 = args->r1;
    int p2 = args->p2;
    int r2 = args->r2;
    int *A = args->A;
    int p3 = args->p3;

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

        pthread_t thread1;
        P_FusionArgs args1 = {T, p1, q1-1, p2, q2-1, A, p3};
        pthread_create(&thread1, NULL, P_Fusion, &args1);

        pthread_t thread2;
        P_FusionArgs args2 = {T, q1+1, r1, q2, r2, A, q3+1};
        pthread_create(&thread1, NULL, P_Fusion, &args2);

        pthread_join(&thread2, NULL);
        pthread_join(&thread1, NULL);
    }
}

int printTab(int T[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf(" %d", T[i]);
    }
    printf(" ]\n");
}

int main() {
    printf("Please enter n followed by two sorted arrays of size n : ");
    int n;
    scanf("%d", &n);
    n = 2 * n;
    int T[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &T[i]);
    }
    int B[n];
    printTab(T, n);
    P_FusionArgs args = {T, 0, n / 2 - 1, n / 2, n - 1, B, 0};
    P_Fusion(&args);
    printTab(B, n);
    return 0;
}