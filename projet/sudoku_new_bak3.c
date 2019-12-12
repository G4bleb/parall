#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum possibilitiesState {FOUND, NOT_FOUND, SUDOKU_WRONG};

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
            // printf("%d ", sdk[a][b]);
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
    int maxI = lineNum - iRelativeToBox + N_input;
    for (i = lineNum - iRelativeToBox; i < maxI; i++) {
        int maxJ = colNum - jRelativeToBox + N_input;
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
        printf("Could not find any NewSdkFromPool\n");
        return NULL;
    }
}

void addCopyOfSdkToSdkPool(int **sdk) {
    int **tmpSdk = malloc(Size * sizeof(int *));
    int i, j;

    for (i = 0; i < Size; i++) {
        tmpSdk[i] = malloc(Size * sizeof(int));
        for (j = 0; j < Size; j++) {
            tmpSdk[i][j] = sdk[i][j];
        }
    }

    // printf("Sdk with [%d][%d] = %d is now in pool\n", x, y, tmpSdk[x][y]);
    sdkPoolSize++;
    sdkPool = realloc(sdkPool, (sdkPoolSize) * sizeof(int **));
    sdkPool[sdkPoolSize - 1] = tmpSdk;
    // printf("SdkPoolSize = %d\n", sdkPoolSize);
}

enum possibilitiesState lookForPossibilities(int **sdk, int pool, int *possibilities, int *tmp){
    int i, j, possibCount;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            if (sdk[i][j] == 0) {
                possibCount = findPossibilities(i, j, possibilities, sdk);
                if (!possibCount) return SUDOKU_WRONG;
                if (possibCount == pool) {
                    tmp[0] = i;
                    tmp[1] = j;
                    return FOUND;
                }
            }
        }
    }
    return NOT_FOUND;
}

int **solveSdk(int **sdk) {
    int possibilities[Size];
    int tmp[2];
    int k, pool;
    enum possibilitiesState state;
    for (pool = 1; pool <= Size; pool++) {
        state = lookForPossibilities(sdk, pool, possibilities, tmp);
        if(state == FOUND){
            if(pool == 1){
                // printf("One possibility for [%d][%d] : %d\n", tmp[0], tmp[1],possibilities[0]);
                sdk[tmp[0]][tmp[1]] = possibilities[0];
            }else{
                // printf("%d possibilities for [%d][%d]\n", pool, tmp[0], tmp[1]);
                for (k = 0; k < pool; k++) {
                    sdk[tmp[0]][tmp[1]] = possibilities[k];
                    addCopyOfSdkToSdkPool(sdk);
                }
                for (k = 0; k < pool; k++) {
                    #pragma omp task
                    solveSdk(getNewSdkFromPool(NULL));
                }
            }
            pool = 0;
        }else if(state == SUDOKU_WRONG){
            #pragma omp taskwait
            // printf("Sudokus wrong : %d\n", ++wrongCounter);
            // sdk = getNewSdkFromPool(sdk);
            // printSudoku(sdk);
            if (!sdk) {
                printf("COULD NOT GET A NEW SUDOKU\n");
                return NULL;
            }
            pool = 0;
            return NULL;
        }else{
            if(pool == Size){
                printf("---------------Sudoku solved-----------\n");
                printSudoku(sdk);
                exit(0);
            }
        }
    }
    return NULL;
}

int main(void) {
    int **sdk = initialize();
    loadFromInput(sdk);
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