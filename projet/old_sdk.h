#include <stdio.h>
#include <stdlib.h>

typedef struct Sudoku {
    int **mat;
    int n;
    int size;
} Sudoku;

Sudoku initializeSdk(int n);
Sudoku loadSdk();
char isValueValidForLine(int val, int *line, int length);
char isValueValidForColumn(int val, Sudoku sdk, int colIndex);
char isValueValidForSquare(int val, Sudoku sdk, int squarenum);
void logSdk(Sudoku sdk);