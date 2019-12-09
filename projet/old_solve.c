#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Sudoku.h"

Sudoku solveSdk(Sudoku sdk){
    int i, j, val;
    for (i = 0; i < sdk.size; i++) {
        for (j = 0; j < sdk.size; j++) {
            while (!isValueValidForLine(val, sdk.mat[i], sdk.size) || !isValueValidForColumn(val, sdk, j)) {
                val = (rand() % sdk.size) + 1; // TODO a number pool
            }
            sdk.mat[i][j] = val;
            printf("sdk.mat[%d][%d] = %d\n", i, j, val);
        }
    }

    return sdk;
}

int main(){
    srand(time(NULL));
    Sudoku sdk = loadSdk();
    sdk = solveSdk(sdk);
    logSdk(sdk);
    //Fill 0's
    return 0;
}