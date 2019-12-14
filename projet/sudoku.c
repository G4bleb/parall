#include <mpi.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define TAG_SUDOKU 0
#define TAG_CELL 1
#define TAG_WAITING 2
#define TAG_SOLVED 3

bool Finalized;

typedef struct Cell {
    bool *possibilities;
    bool solved;
    int possibCount;
} Cell;

int Id;
int P;

int N_input;
int Size;
Cell ***sdkPool;
int sdkPoolSize;
int sdkMaxPoolSize;

int **initializeSudoku() {
    printf("Waiting for size input...\n");
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

void printIntSudoku(int **sdk) {
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

int findPossibilities(int lineNum, int colNum, bool *ret, int **sdk) {
    int num;
    int possibCount = 0;
    for (num = 1; num <= Size; num++) {
        if (canBePlaced(num, lineNum, colNum, sdk)) {
            ret[num - 1] = true;
            possibCount++;
        } else {
            ret[num - 1] = false;
        }
    }
    return possibCount;
}

bool firstCellWithKPossibilities(Cell **cellSdk, int k, int out[2]) {
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            if (cellSdk[i][j].possibCount == k && !cellSdk[i][j].solved) {
                out[0] = i;
                out[1] = j;
                return true;
            }
        }
    }
    return false;
}

Cell **CellSudokuFromIntSudoku(int **sdk) {
    Cell **cellSdk = malloc(Size * sizeof(Cell *));
    int i, j, num;

    for (i = 0; i < Size; i++) {
        cellSdk[i] = malloc(Size * sizeof(Cell));
        for (j = 0; j < Size; j++) {
            cellSdk[i][j].possibilities = malloc(Size * sizeof(bool));
            if (sdk[i][j] == 0) {
                cellSdk[i][j].possibCount =
                    findPossibilities(i, j, cellSdk[i][j].possibilities, sdk);
                cellSdk[i][j].solved = false;

            } else {
                for (num = 1; num <= Size; num++) {
                    cellSdk[i][j].possibilities[num - 1] = false;
                }
                cellSdk[i][j].possibilities[sdk[i][j] - 1] = true;
                cellSdk[i][j].possibCount = 1;
                cellSdk[i][j].solved = true;
            }
        }
    }
    return cellSdk;
}

void destroyCellSdk(Cell **cellSdk) {
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            free(cellSdk[i][j].possibilities);
        }
        free(cellSdk[i]);
    }
    free(cellSdk);
}

void setValue(Cell **cellSdk, int lineNum, int colNum, int val) {
    // Remove this possibility from other cells
    // In line, in column; also remove all possibilities for cell where we will
    // put a new value
    int i;
    // #pragma omp parallel fors
    for (i = 0; i < Size; i++) {
        if (cellSdk[lineNum][i].possibilities[val - 1]) {
            cellSdk[lineNum][i].possibilities[val - 1] = false;
            cellSdk[lineNum][i].possibCount--;
        }
        if (cellSdk[i][colNum].possibilities[val - 1]) {
            cellSdk[i][colNum].possibilities[val - 1] = false;
            cellSdk[i][colNum].possibCount--;
        }
        cellSdk[lineNum][colNum].possibilities[i] = false;
    }
    int j;
    // In box
    int iRelativeToBox = lineNum % N_input;
    int jRelativeToBox = colNum % N_input;
    int maxI = lineNum - iRelativeToBox + N_input;
    int maxJ;
    // #pragma omp parallel for private(i, maxJ, j)
    for (i = lineNum - iRelativeToBox; i < maxI; i++) {
        maxJ = colNum - jRelativeToBox + N_input;
        for (j = colNum - jRelativeToBox; j < maxJ; j++) {
            if (cellSdk[i][j].possibilities[val - 1]) {
                cellSdk[i][j].possibilities[val - 1] = false;
                cellSdk[i][j].possibCount--;
            }
        }
    }

    cellSdk[lineNum][colNum].possibilities[val - 1] = true;
    cellSdk[lineNum][colNum].possibCount = 1;
    cellSdk[lineNum][colNum].solved = true;
}

bool isCellSdkSolved(Cell **cellSdk) {
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            if (!cellSdk[i][j].solved) {
                return false;
            }
        }
    }
    return true;
}

void printSudoku(Cell **cellSdk) {
    printf("printSudoku...\n");
    int a, b;
    int k;
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            if (b % N_input == 0) {
                printf("\t");
            }
            if (cellSdk[a][b].solved) {
                for (k = 0; k < Size; k++) {
                    if (cellSdk[a][b].possibilities[k]) {
                        printf("%c", k + 'A');
                        break; // TODO meh
                    }
                }
            } else {
                printf("%c", '@');
            }
        }
        if ((a + 1) % N_input == 0) {
            printf("\n");
        }
        printf("\n");
    }
}

bool isCellSdkSolvable(Cell **cellSdk) {
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            if (cellSdk[i][j].possibCount == 0) {
                return false;
            }
        }
    }
    return true;
}

Cell **popFromSdkPool() {
    if (sdkPoolSize != 0) {
        sdkPoolSize--;
        return sdkPool[sdkPoolSize];
    } else {
        printf("Could not find any Sdk from pool\n");
        return NULL;
    }
}

void pushToSdkPool(Cell **cellSdk) {
    // printf("Sdk with [%d][%d] = %d is now in pool\n", x, y, tmpSdk[x][y]);
    sdkPoolSize++;
    if(sdkPoolSize > sdkMaxPoolSize){
        sdkPool = realloc(sdkPool, (sdkPoolSize) * sizeof(int **));
    }
    sdkPool[sdkPoolSize - 1] = cellSdk;
    // printf("SdkPoolSize = %d\n", sdkPoolSize);
}

Cell **copyCellSdk(Cell **cellSdk) {
    Cell **tmpCellSdk;
    tmpCellSdk = malloc(Size * sizeof(Cell *));
    int i, j, k;
    for (i = 0; i < Size; i++) {
        tmpCellSdk[i] = malloc(Size * sizeof(Cell));
        for (j = 0; j < Size; j++) {
            tmpCellSdk[i][j] = cellSdk[i][j];
            tmpCellSdk[i][j].possibilities = malloc(Size * sizeof(bool));
            for (k = 0; k < Size; k++) {
                tmpCellSdk[i][j].possibilities[k] =
                    cellSdk[i][j].possibilities[k];
            }
        }
    }
    return tmpCellSdk;
}

void mpiSendSdk(Cell **cellSdk, int destination){
    // bool over = !cellSdk;
    // MPI_Send(&over, 1, MPI_C_BOOL, destination, TAG_SUDOKU, MPI_COMM_WORLD);
    // if (over) {
    //     return;
    // }
    int i, j;
    for (i = 0; i < Size; i++) {
        for (j = 0; j < Size; j++) {
            MPI_Send(cellSdk[i][j].possibilities, Size, MPI_C_BOOL, destination, TAG_CELL, MPI_COMM_WORLD);
            MPI_Send(&cellSdk[i][j].possibCount, 1, MPI_INT, destination, TAG_CELL, MPI_COMM_WORLD);
            MPI_Send(&cellSdk[i][j].solved, 1, MPI_C_BOOL, destination, TAG_CELL, MPI_COMM_WORLD);
        }
    }
}

Cell ** mpiReceiveSdk(){
    // MPI_Status status;
    // bool dummy;
    // MPI_Recv(&dummy, 1, MPI_C_BOOL, 0, TAG_SUDOKU, MPI_COMM_WORLD, &status);

    Cell **tmpCellSdk;
    tmpCellSdk = malloc(Size * sizeof(Cell *));
    int i, j;
    for (i = 0; i < Size; i++) {
        tmpCellSdk[i] = malloc(Size * sizeof(Cell));
        for (j = 0; j < Size; j++) {
            tmpCellSdk[i][j].possibilities = malloc(Size * sizeof(bool));
            MPI_Recv(tmpCellSdk[i][j].possibilities, Size, MPI_C_BOOL, 0, TAG_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&tmpCellSdk[i][j].possibCount, 1, MPI_INT, 0, TAG_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&tmpCellSdk[i][j].solved, 1, MPI_C_BOOL, 0, TAG_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    return tmpCellSdk;
}

void mpiISendSolved(int destination) {
    printf("ps %d Isending Solved to ps %d\n", Id, destination);
    MPI_Request req;
    bool tmp = true;
    MPI_Isend(&tmp, 1, MPI_C_BOOL, destination, TAG_SOLVED, MPI_COMM_WORLD, &req);
    MPI_Request_free(&req);
}
void mpiSendSolved(int destination) {
    printf("ps %d sending Solved to ps %d\n", Id, destination);
    bool tmp = true;
    MPI_Send(&tmp, 1, MPI_C_BOOL, destination, TAG_SOLVED, MPI_COMM_WORLD);
}

Cell **solveSudoku(Cell **cellSdk, MPI_Request * solvedReq) {
    int flag = false, send_flag = false;
    MPI_Test(solvedReq, &flag, MPI_STATUS_IGNORE);
    if (flag) {
        Finalized = true;
        printf("Finalizing process %d\n", Id);
        // if (!Finalized)
        MPI_Finalize();
        // Finalized = true;
        exit(0);
    }
    MPI_Request req;
    Cell **tmpCellSdk;
    int k, pIndex, i, count;
    int num;
    int tmp[2];
    for (num = 1; num <= Size; num++) {
        while (firstCellWithKPossibilities(cellSdk, num, tmp)) {
            pIndex = -1;
            for (k = 0; k < num; k++) {
                tmpCellSdk = copyCellSdk(cellSdk);
                for (pIndex++; cellSdk[tmp[0]][tmp[1]].possibilities[pIndex] != true; pIndex++);
                setValue(tmpCellSdk, tmp[0], tmp[1], pIndex + 1);
                if (isCellSdkSolvable(tmpCellSdk)) {
                    // Le dérivé est résolvable
                    #pragma omp critical
                    pushToSdkPool(tmpCellSdk);
                } else {
                    destroyCellSdk(tmpCellSdk);
                }
            }
            count = sdkPoolSize;
            if(!Id){
                int idtoSendTo;
                for (i = 1; i < count; i++) {
                    tmpCellSdk = popFromSdkPool();
                    // printf("i = %d, Preparing to get asked for a Sudoku\n", i);
                    MPI_Irecv(&idtoSendTo, 1, MPI_INT, MPI_ANY_SOURCE, TAG_WAITING, MPI_COMM_WORLD, &req);
                    while (!flag && !send_flag){
                        MPI_Test(&req, &send_flag, MPI_STATUS_IGNORE);
                        MPI_Test(solvedReq, &flag, MPI_STATUS_IGNORE);
                    }
                    if (flag) {tmpCellSdk = solveSudoku(tmpCellSdk, solvedReq);}
                    // printf("It was send_flag (= %d), rank %d\n", send_flag, idtoSendTo);
                    mpiSendSdk(tmpCellSdk, idtoSendTo);
                }
                tmpCellSdk = solveSudoku(popFromSdkPool(), solvedReq);
                // if (tmpCellSdk) {
                //     for (k = 1; k < P; k++) {
                //         mpiSendSolved(k);
                //     }
                //     // if (!Finalized) MPI_Finalize();
                //     Finalized = true;
                //     exit(0);
                //     // printf("mpiReceiveSdk Finalizing for process %d\n", Id);
                //     // Finalize = true;
                // }
            }else{
                //shared(Finalize)
                // #pragma omp parallel for private(i, tmpCellSdk) shared(Finalized)
                for (i = 0; i < count; i++) {
                    #pragma omp critical
                    tmpCellSdk = popFromSdkPool();
                    tmpCellSdk = solveSudoku(tmpCellSdk, solvedReq);
                    // if (tmpCellSdk && Finalized == false) {
                    //     for (k = 0; k < P; k++){
                    //         // mpiISendSolved(k);
                    //         if (k != Id) mpiSendSolved(k);
                    //     }
                    //     // MPI_Finalize();
                    //     Finalized = true;
                    //     exit(0);
                    // }
                    MPI_Test(solvedReq, &flag, MPI_STATUS_IGNORE);
                    if (!Finalized && flag) {
                        Finalized = true;
                        printf("Finalizing process %d\n", Id);
                        // MPI_Finalize();
                        exit(0);
                    }
                }
            }
            // MPI_Reduce(&Finalize, &Finalize, 1, MPI_C_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
            // MPI_Bcast(&Finalize, 1, MPI_INT, 0, MPI_COMM_WORLD);
            
            // if(Finalize){
            //     MPI_Finalize();
            //     exit(0);
            // }
            
            return tmpCellSdk;
        }
    }
    // MPI_Reduce(&Finalized, &Finalized, 1, MPI_C_BOOL, MPI_LOR, 0,
    // MPI_COMM_WORLD); MPI_Bcast(&Finalized, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(!Finalized){
        Finalized = true;
        printf("Process %d print :\n", Id);
        printSudoku(cellSdk);
        for (k = 0; k < P; k++) {
            if (k != Id)
                mpiISendSolved(k);
            //     mpiSendSolved(k);
        }
        printf("Finalizing process %d\n", Id);
        MPI_Finalize();
        exit(0);
    }
    
    // MPI_Reduce(&Finalize, &Finalize, 1, MPI_C_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&Finalize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    return cellSdk;
}


int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &Id);
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    int **sdk = NULL;
    if(!Id){
        sdk = initializeSudoku();
    }

    MPI_Bcast(&N_input, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    sdkPool = NULL, sdkPoolSize = 0;
    Finalized = false;
    bool dummy = false;
    MPI_Request req;
    MPI_Irecv(&dummy, 1, MPI_C_BOOL, MPI_ANY_SOURCE, TAG_SOLVED, MPI_COMM_WORLD, &req);
    if (!Id) {
        loadFromInput(sdk);
        Cell **cellSdk = CellSudokuFromIntSudoku(sdk);
        int i;
        for (i = 0; i < Size; i++) {
            free(sdk[i]);
        }
        free(sdk);
        printSudoku(cellSdk);
        solveSudoku(cellSdk, &req);
        MPI_Finalize();
        //destroyCellSdk(cellSdk);
    }else{
        // int flag;
        while(true){
            // MPI_Test(&req, &flag, MPI_STATUS_IGNORE);
            // if (flag) {
            //     printf("Finalizing process %d\n", Id);
            //     MPI_Finalize();
            //     exit(0);
            // }
            MPI_Send(&Id, 1, MPI_INT, 0, TAG_WAITING, MPI_COMM_WORLD);
            solveSudoku(mpiReceiveSdk(), &req);
        }
    }
    //printIntSudoku(sdk);
    
    
    
    // sdk = solveSdk(sdk);
    // if(sdk){
    //     printIntSudoku(sdk);

    // }
    return 0;
}