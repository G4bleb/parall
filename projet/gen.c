#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Sudoku.h"

void fillSdk(Sudoku sdk) {
    int val;
    int i, j;
    for (i = 0; i < sdk.size; i++) {
        for (j = 0; j < sdk.size; j++) {
            while (!isValueValidForLine(val, sdk.mat[i], sdk.size)) {
                val = (rand() % sdk.size) + 1; // TODO a number pool
            }
            sdk.mat[i][j] = val;
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Need argument n\n");
        return 1;
    }
    srand(time(NULL));

    int n = atoi(argv[1]);
    Sudoku sdk = initializeSdk(n);
    fillSdk(sdk);
    logSdk(sdk);
    // int * numberPool = malloc(n*sizeof(int));
    
    return 0;
}