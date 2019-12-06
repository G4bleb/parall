#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage : %s n\n", argv[0]);
        return 1;
    }
    srand(time(NULL));
    int n = atoi(argv[1]);
    if (n < 1) {
        printf("n can't be negative\n");
        return 1;
    }
    int *mat = malloc(n * n * sizeof(int));
    if (mat == NULL) {
        printf("Malloc error\n");
        return 1;
    }
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j) {
                mat[i * n + j] = 0;
            } else {
                mat[i * n + j] = (rand() % n) + 1; // Chiffre entre 1 et n
            }
        }
    }
    printf("matrix is of size %dx%d, first value is %d\n", n, n, mat[0]);
    printf("%d\n" , n);
    // fprintf(fp, "%d\n" , n);

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", mat[i * n + j]);
        }
    }
    printf("\n");
    // fwrite(&n, sizeof(int), 1, fp);
    // fwrite(&n, sizeof(int), 1, fp);
    // fwrite(mat, sizeof(int), n * n, fp);

    return 0;
}
