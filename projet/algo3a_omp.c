#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Cell {
    bool *possibilities; // Chiffres possibles dans une cellule, taille n*n
    bool solved;         // Définitivement résolue ou pas
    int possibCount;     // Nombre de possibilités restantes
} Cell;

int N_input;        // n
int Size;           // Taille du sudoku (n*n)
Cell ***sdkPool;    // Tableau de sudokus
int sdkPoolSize;    // Taille de ^
int sdkMaxPoolSize; // Taille maximum jamais atteinte de ^

// Récupère la taille du sudoku et alloue de la mémoire pour lui
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

// Charge le sudoku dans une matrice d'entiers
void loadFromInput(int **sdk) {
    printf("Loading sudoku...\n");
    int a, b;
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            scanf("%d ", &sdk[a][b]);
        }
    }
}

// Imprime un sudoku d'entiers
void printIntSudoku(int **sdk) {
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

// Vérifie si la valeur donnée est présente dans la ligne
bool inLine(int num, int lineNum, int **sdk) {
    int j;
    for (j = 0; j < Size; j++) {
        if (sdk[lineNum][j] == num) {
            return true;
        }
    }
    return false;
}

// Vérifie si la valeur donnée est présente dans la colonne
bool inColumn(int num, int colNum, int **sdk) {
    int i;
    for (i = 0; i < Size; i++) {
        if (sdk[i][colNum] == num) {
            return true;
        }
    }
    return false;
}

// Vérifie si la valeur donnée est présente dans la boîte
// Pourrait être optimisé pour ne pas repasser sur certaines valeurs vérifiées
// par inLine et inColumn
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

// Vérifie si une valeur peut être placée dans le Sudoku
bool canBePlaced(int num, int lineNum, int colNum, int **sdk) {
    if (!inLine(num, lineNum, sdk) && !inColumn(num, colNum, sdk) &&
        !inBox(num, lineNum, colNum, sdk)) {
        return true;
    }
    return false;
}

/**
 * Trouve les possibilités à mettre dans une case
 * ret est le tableau de possibilités de la Cell (paramètre de sortie)
 * renvoie le nombre de possibilités
 */
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

/**
 * Trouve la première cellule contenant k possibilités
 * ret contient les coordonnées de la Cell (paramètre de sortie)
 * renvoie true si une cellule a été trouvée
 */
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

/**
 * Créé un Sudoku de cellules à partir d'un Sudoku d'entiers
 */
Cell **CellSudokuFromIntSudoku(int **sdk) {
    Cell **cellSdk = malloc(Size * sizeof(Cell *));
    int i, j, num;

    for (i = 0; i < Size; i++) {
        cellSdk[i] = malloc(Size * sizeof(Cell));
        for (j = 0; j < Size; j++) {
            cellSdk[i][j].possibilities = malloc(Size * sizeof(bool));
            if (sdk[i][j] == 0) { // Valeur inconnue
                cellSdk[i][j].possibCount =
                    findPossibilities(i, j, cellSdk[i][j].possibilities, sdk);
                cellSdk[i][j].solved = false;

            } else { // Valeur connue
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

// Détruit un sudoku de cellules
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
// Assigne une valeur à une cellule, devenant la seule possibilité qu'elle
// contient Met à jour les possbilités des autres cellules affectées
void setValue(Cell **cellSdk, int lineNum, int colNum, int val) {
    int i;
    // #pragma omp parallel for
    for (i = 0; i < Size; i++) {
        // Dans la ligne
        if (cellSdk[lineNum][i].possibilities[val - 1]) {
            cellSdk[lineNum][i].possibilities[val - 1] = false;
            cellSdk[lineNum][i].possibCount--;
        }
        // Dans la colonne
        if (cellSdk[i][colNum].possibilities[val - 1]) {
            cellSdk[i][colNum].possibilities[val - 1] = false;
            cellSdk[i][colNum].possibCount--;
        }
        // Retire toutes les possibilités de la cellule
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

    // Une seule possibilité vraie
    cellSdk[lineNum][colNum].possibilities[val - 1] = true;
    cellSdk[lineNum][colNum].possibCount = 1;
    cellSdk[lineNum][colNum].solved = true;
}

// Vérifie si toutes les cellules du sudoku sont résolues
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
// Imprime un sudoku de cellules
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
                        break; // Unique possibilité imprimée
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

// Vérifie si un Sudoku de cellules est résolvable
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

// Retire la dernière valeur du Pool de sudoku
Cell **popFromSdkPool() {
    if (sdkPoolSize != 0) {
        sdkPoolSize--;
        return sdkPool[sdkPoolSize];
    } else {
        printf("Could not find any Sdk from pool\n");
        return NULL;
    }
}

// Ajoute un sudoku au bout du pool de sudoku
void pushToSdkPool(Cell **cellSdk) {
    sdkPoolSize++;
    if (sdkPoolSize > sdkMaxPoolSize) {
        sdkPool = realloc(sdkPool, (sdkPoolSize) * sizeof(int **));
    }
    sdkPool[sdkPoolSize - 1] = cellSdk;
}

// Copie un Sudoku de Cells
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

// Résoud un Sudoku (récursif pour les sous-sudokus)
Cell **solveSudoku(Cell **cellSdk) {
    Cell **tmpCellSdk;
    int k, pIndex, i, count;
    int num;
    int tmp[2];
    for (num = 1; num <= Size; num++) { // On cherche les possibilités dans l'ordre croissant
        if (firstCellWithKPossibilities(cellSdk, num, tmp)) {//Si l'on a trouvé une possibilité 
            pIndex = -1;
            for (k = 0; k < num; k++) { // Pour toutes les possibilités
                // Créer un nouveau sudoku
                tmpCellSdk = copyCellSdk(cellSdk);
                // Trouver la possiblité
                for (pIndex++;
                     cellSdk[tmp[0]][tmp[1]].possibilities[pIndex] != true;
                     pIndex++);
                // Ecrire la valeur dans le sudoku
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
            #pragma omp parallel for private(i, tmpCellSdk)
            for (i = 0; i < count; i++) {
                #pragma omp critical
                tmpCellSdk = popFromSdkPool();
                tmpCellSdk = solveSudoku(tmpCellSdk);
                if (tmpCellSdk)//n'est pas NULL = sudoku résolu
                    exit(0);
            }
            return tmpCellSdk;
        }
    }
    printSudoku(cellSdk);
    return cellSdk;
}

int main(void) {

    int **sdk = initializeSudoku();
    loadFromInput(sdk);
    Cell **cellSdk = CellSudokuFromIntSudoku(sdk);
    printSudoku(cellSdk);
    sdkPool = NULL, sdkPoolSize = 0;
    solveSudoku(cellSdk);
    
    int i;
    for (i = 0; i < Size; i++) {
        free(sdk[i]);
    }
    free(sdk);
    destroyCellSdk(cellSdk);
    return 0;
}