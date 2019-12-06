#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    int m, n;
    FILE *fp = fopen("matrix", "r");
    if (fp == NULL) {
        printf("Malloc error\n");
        return 1;
    }
    // fread(&m, sizeof(int), 1, fp);
    // fread(&n, sizeof(int), 1, fp);
    fscanf(fp, "%d\n", &n);
    m = n;
    // fscanf(fp, "%d ", &n);

    if (m != n) {
        printf("Matrix not square\n");
        return 1;
    }
    printf("matrix is of size %dx%d\n", n, n);

    int *mat = malloc(n * n * sizeof(int));
    if (mat == NULL) {
        printf("Malloc error\n");
        return 1;
    }
    // fread(mat, sizeof(int), n*n, fp);
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            fscanf(fp, "%d ", &mat[i*n+j]);
            printf("%d ", mat[i*n+j]);
        }
        printf("\n");
    }

    fclose(fp);
    return 0;
}
