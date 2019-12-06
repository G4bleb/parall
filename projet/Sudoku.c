#include "Sudoku.h"

Sudoku initializeSdk(int n) {
    Sudoku sdk;
    sdk.n = n;
    sdk.size = n*n;
    sdk.mat = malloc(sdk.size * sizeof(int *));
    int j;
    int *line;
    for (j = 0; j < sdk.size; j++) {
        sdk.mat[j] = calloc(sdk.size, sizeof(int));
    }
    return sdk;
}

Sudoku loadSdk() {
    Sudoku sdk;
    scanf("%d\n", &sdk.n);
    sdk.size = sdk.n * sdk.n;
    int i, j;
    sdk.mat = malloc(sdk.size * sizeof(int *));
    for (i = 0; i < sdk.size; i++) {
        sdk.mat[i] = malloc(sdk.size * sizeof(int));
        for (j = 0; j < sdk.size - 1; j++) {
            scanf("%d ", &sdk.mat[i][j]);
        }
        scanf("%d \n", &sdk.mat[i][sdk.size - 1]);
    }
    return sdk;
}

char isValueValidForLine(int val, int *line, int length) {
    int i;
    for (i = 0; i < length; i++) {
        if (val == line[i]) {
            return 0;
        }
    }
    return 1;
}

char isValueValidForColumn(int val, Sudoku sdk, int colIndex){
    int i;
    for (i = 0; i < sdk.size; i++) {
        if (val == sdk.mat[i][colIndex]) {
            return 0;
        }
    }
    return 1;
}

char isValueValidForSquare(int val, Sudoku sdk, int squarenum){
    
}

void logSdk(Sudoku sdk) {
    printf("%d\n", sdk.n);
    int val;
    int i, j;
    for (j = 0; j < sdk.size; j++) {
        for (i = 0; i < sdk.size; i++) {
            printf("%d ", sdk.mat[j][i]);
        }
        printf("\n");
    }
}