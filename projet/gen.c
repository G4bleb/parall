#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage : %s n\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int i, j;
    printf("%d\n", n);
    for (i = 0; i < n*n; i++){
        for (j = 0; j < n * n; j++) {
            printf("%d ", 0);
        }
        printf("\n");
    }

    
    return 0;
}