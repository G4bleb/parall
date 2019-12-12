#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int N_input;
int Size;
int ***sdkPool;
int sdkPoolSize;

int ** initialize() {
    printf("Waiting for input...\n");
    scanf("%d ", &N_input);
    Size = N_input * N_input;
    int **sdk = malloc(Size * sizeof(int *));
    int i;
    for (i = 0; i < Size; i++) {
        sdk[i] = malloc(Size * sizeof(int));
    }
    return sdk;
}

void loadFromInput(int **sdk) {
    printf("Loading sudoku...\n");
    int a, b;
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            scanf("%d ", &sdk[a][b]);
        }
    }
}

void printSudoku(int **sdk) {
    printf("Printing sudoku...\n");
    int a, b;
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            if (b % N_input == 0) {
                printf("\t");
            }
            printf("%c", sdk[a][b] + 'A' - 1);
        }
        if ((a + 1) % N_input == 0) {
            printf("\n");
        }
        printf("\n");
    }
}

bool inLine(int num, int lineNum, int **sdk) {
    int j;
    for (j = 0; j < Size; j++) {
        if (sdk[lineNum][j] == num) {
            return true;
        }
    }
    return false;
}

bool inColumn(int num, int colNum, int **sdk) {
    int i;
    for (i = 0; i < Size; i++) {
        if (sdk[i][colNum] == num) {
            return true;
        }
    }
    return false;
}

// TODO A optimiser pour ne pas repasser sur la ligne et la colonne
bool inBox(int num, int lineNum, int colNum, int **sdk) {
    int i, j;
    int iRelativeToBox = lineNum % N_input;
    int jRelativeToBox = colNum % N_input;
    int maxJ;
    int maxI = lineNum - iRelativeToBox + N_input;
    for (i = lineNum - iRelativeToBox; i < maxI; i++) {
        maxJ = colNum - jRelativeToBox + N_input;
        for (j = colNum - jRelativeToBox; j < maxJ; j++) {
            if (sdk[i][j] == num) {
                return true;
            }
        }
    }
    return false;
}

bool canBePlaced(int num, int lineNum, int colNum, int **sdk) {
    if (!inLine(num, lineNum, sdk) && !inColumn(num, colNum, sdk) &&
        !inBox(num, lineNum, colNum, sdk)) {
        return true;
    }
    return false;
}

int findPossibilities(int lineNum, int colNum, int *ret, int **sdk) {
    int possibCount = 0;
    int num;
    for (num = 1; num <= Size; num++) {
        if (canBePlaced(num, lineNum, colNum, sdk)) {
            ret[possibCount] = num;
            possibCount++;
        }
    }
    return possibCount;
}

bool isSdkSolved(int **sdk) {
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            if (sdk[i][j] == 0) {
                return false;
            }
        }
    }
    return true;
}

int **getNewSdkFromPool(int **sdk) {
    int i;
    if (sdkPoolSize != 0) {
        if (sdk) {
            for (i = 0; i < Size; i++) {
                free(sdk[i]);
            }
            free(sdk);
        }
        sdkPoolSize--;
        return sdkPool[sdkPoolSize];
    } else {
        printf("Could not find any answers\n");
        return NULL;
    }
}

void addSdkWithPossibilityToSdkPool(int **sdk, int x, int y, int *possibilities,
                                    int possibility) {
    int **tmpSdk = malloc(Size * sizeof(int *));
    int i, j;

    for (i = 0; i < Size; i++) {
        tmpSdk[i] = malloc(Size * sizeof(int));
        for (j = 0; j < Size; j++) {
            tmpSdk[i][j] = sdk[i][j];
        }
    }

    tmpSdk[x][y] = possibilities[possibility];
    printf("Sdk with [%d][%d] = %d is now in pool\n", x, y, tmpSdk[x][y]);
    sdkPoolSize++;
    sdkPool = realloc(sdkPool, (sdkPoolSize) * sizeof(int **));
    sdkPool[sdkPoolSize - 1] = tmpSdk;
    // printf("SdkPoolSize = %d\n", sdkPoolSize);
}

int **solveSdk(int **sdk) {
    int possibilities[Size];
    int tmp[2];
    bool finished = false;
    bool foundOne;
    int i, j, k, pool, possibCount;
    while (!finished) {
        for (pool = 1; pool <= Size; pool++) {
            foundOne = false;
            for (i = 0; i < Size; i++) {
                for (j = 0; j < Size; j++) {
                    if (sdk[i][j] == 0) {
                        possibCount = findPossibilities(i, j, possibilities, sdk);
                        if (possibCount) {
                            if (possibCount <= pool) {
                                foundOne = true;
                                if (possibCount == 1) {
                                    sdk[i][j] = possibilities[0];
                                    // printf("Written %d at %d, %d\n",
                                    //        possibilities[0], i, j);
                                } else { // C'est plus
                                    printf("More than 1 possibility in %d %d\n", i, j);
                                    // Créer possibCount sudokus
                                    for (k = 1; k < possibCount; k++) {
                                        addSdkWithPossibilityToSdkPool(
                                            sdk, i, j, possibilities, k);
                                    }
                                    sdk[i][j] = possibilities[0];
                                    i = Size;
                                    j = Size;
                                }
                            }
                        } else {
                            sdk = getNewSdkFromPool(sdk);
                            if (!sdk) {
                                printf("COULD NOT GET A NEW SUDOKU\n");
                                i = Size;
                                j = Size;
                                return NULL;
                            }
                        }

                        // printf("There are %d possibilites for %d, %d\n",
                        //        possibCount, i, j);
                    }
                }
            }
            if (foundOne) {
                pool = 0;
            }
        }
        if (!isSdkSolved(sdk)) {
            sdk = getNewSdkFromPool(sdk);
            if (!sdk) {
                printf("SUDOKU NOT SOLVED\n");
                finished = true;
            }
        } else {
            printf("---------------Sudoku solved\n");
            finished = true;
        }
    }
    return sdk;
}

int main(void) {
    int **sdk = initialize();
    loadFromInput(sdk);
    printf("Is 1 in line 3 ? : %d\n", inLine(1, 3, sdk));
    printf("Is 4 in line 3 ? : %d\n", inLine(4, 3, sdk));
    printf("Is 4 in col  3 ? : %d\n", inColumn(4, 3, sdk));
    printf("Is 4 in col  4 ? : %d\n", inColumn(4, 4, sdk));
    printf("Is 1 in box where 4x4 is ? : %d\n", inBox(1, 4, 4, sdk));
    printf("Is 4 in col where 4x4 is ? : %d\n", inBox(4, 4, 4, sdk));
    printf("Can I place a 1 at 0 0 ? : %d\n", canBePlaced(1, 0, 0, sdk));
    printf("Can I place a 1 at 3 0 ? : %d\n", canBePlaced(1, 3, 0, sdk));
    printSudoku(sdk);
    sdkPool = NULL, sdkPoolSize = 0;
    sdk = solveSdk(sdk);
    if(sdk){
        printSudoku(sdk);
        int i;
        for (i = 0; i < Size; i++) {
            free(sdk[i]);
        }
        free(sdk);
    }
    
    return 0;
}