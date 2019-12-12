#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define MAX_COL 1100 //1024
// #define MAX_ROW 4100 //4096

#define SQ_OFFSET 0
#define RW_OFFSET Size *Size
#define CL_OFFSET Size *Size * 2
#define BX_OFFSET Size *Size * 3

struct str_node {
    struct str_node *Header;

    struct str_node *Left;
    struct str_node *Right;
    struct str_node *Up;
    struct str_node *Down;

    char IDName;
    int IDNum;
    int count;
};

int Size;
int N_input;

int nCol;
int nRow;
// struct str_node Matrix[MAX_COL][MAX_ROW];
struct str_node **Matrix;
struct str_node Root;
struct str_node *RootNode = &Root;
// struct str_node *RowHeader[MAX_ROW];
struct str_node **RowHeader;
// char Data[MAX_COL][MAX_ROW];
char **Data;
// int Result[MAX_ROW];
int *Result;
int nResult = 0;
char Finished;
int GlobalProgressUpdate;
int MaxK;

// --> Initialisation functions
static inline int dataLeft(int i) { return i - 1 < 0 ? nCol - 1 : i - 1; }
static inline int dataRight(int i) { return (i + 1) % nCol; }
static inline int dataUp(int i) { return i - 1 < 0 ? nRow - 1 : i - 1; }
static inline int dataDown(int i) { return (i + 1) % nRow; }

void CreateMatrix(void) {
    int a, b, i, j;
    // Build toroidal linklist matrix according to data bitmap
    printf("CreateMatrix nested fors...\n");
    for (a = 0; a < nCol; a++) {
#pragma omp parallel for private(b, i, j)
        for (b = 0; b < nRow; b++) {
            if (Data[a][b] != 0) {
                // Left pointer
                i = a;
                j = b;
                do {
                    i = dataLeft(i);
                } while (Data[i][j] == 0);
#pragma omp critical
                Matrix[a][b].Left = &Matrix[i][j];
                // Right pointer
                i = a;
                j = b;
                do {
                    i = dataRight(i);
                } while (Data[i][j] == 0);
#pragma omp critical
                Matrix[a][b].Right = &Matrix[i][j];
                // Up pointer
                i = a;
                j = b;
                do {
                    j = dataUp(j);
                } while (Data[i][j] == 0);
#pragma omp critical
                Matrix[a][b].Up = &Matrix[i][j];
                // Down pointer
                i = a;
                j = b;
                do {
                    j = dataDown(j);
                } while (Data[i][j] == 0);
#pragma omp critical
                Matrix[a][b].Down = &Matrix[i][j];
// Header pointer
#pragma omp critical
                Matrix[a][b].Header = &Matrix[a][nRow - 1];
#pragma omp critical
                Matrix[a][b].IDNum = b;
// Row Header
#pragma omp critical
                RowHeader[b] = &Matrix[a][b];
            }
        }
    }
    printf("CreateMatrix second for...\n");
    for (a = 0; a < nCol; a++) {
        Matrix[a][nRow - 1].IDName = 'C';
        Matrix[a][nRow - 1].IDNum = a;
    }
    // Insert root
    Root.IDName = 'R';
    Root.Left = &Matrix[nCol - 1][nRow - 1];
    Root.Right = &Matrix[0][nRow - 1];
    Matrix[nCol - 1][nRow - 1].Right = &Root;
    Matrix[0][nRow - 1].Left = &Root;
}

// --> DLX Algorithm functions
int countOnes(struct str_node *c) {
    struct str_node *r;
    int i = 0;
    for (r = c->Down; r != c; r = r->Down) {
        i++;
    }
    return i;
}

struct str_node *ChooseColumn(void) {
    struct str_node *best, *c;
    int minOnes = 100000; // Should be max ?
    for (c = RootNode->Right; c != RootNode; c = c->Right) {
        if (minOnes > c->count) {
            minOnes = c->count;
            best = c;
        }
        // printf("Considering c->%d, c -> %c\n",c->IDNum, c->IDName);
    }
    return best;
}

void Cover(struct str_node *ColNode) {
    struct str_node *RowNode, *RightNode;
    ColNode->Right->Left = ColNode->Left;
    ColNode->Left->Right = ColNode->Right;
    for (RowNode = ColNode->Down; RowNode != ColNode; RowNode = RowNode->Down) {
        for (RightNode = RowNode->Right; RightNode != RowNode;
             RightNode = RightNode->Right) {
            RightNode->Up->Down = RightNode->Down;
            RightNode->Down->Up = RightNode->Up;
            RightNode->Header->count--;
        }
    }
}

void UnCover(struct str_node *ColNode) {
    struct str_node *RowNode, *LeftNode;
    for (RowNode = ColNode->Up; RowNode != ColNode; RowNode = RowNode->Up) {
        for (LeftNode = RowNode->Left; LeftNode != RowNode;
             LeftNode = LeftNode->Left) {
            LeftNode->Up->Down = LeftNode;
            LeftNode->Down->Up = LeftNode;
            LeftNode->Header->count++;
        }
    }
    ColNode->Right->Left = ColNode;
    ColNode->Left->Right = ColNode;
}

void SolutionRow(struct str_node *RowNode) {
    Cover(RowNode->Header);
    struct str_node *RightNode;
    for (RightNode = RowNode->Right; RightNode != RowNode;
         RightNode = RightNode->Right) {
        Cover(RightNode->Header);
    }
}

void PrintSolution(void);

void Search(int k) {
    /*if(GlobalProgressUpdate < k) {
     printf("== Search(%d)\n", k);
     PrintSolution();
     GlobalProgressUpdate = k;
     }*/
    if ((RootNode->Left == RootNode && RootNode->Right == RootNode) ||
        k == (Size * Size - MaxK)) {
        // Valid solution!
        // printf("----------- SOLUTION FOUND -----------\n");
        PrintSolution();
        Finished = 1;
        return;
    }

    struct str_node *Column = ChooseColumn();
    if (Column->count == 0) {
        // printf("No solutions found\n");
        // PrintSolution();
        return;
    }
    Cover(Column);

    struct str_node *RowNode;
    struct str_node *RightNode;
    for (RowNode = Column->Down; RowNode != Column && !Finished;
         RowNode = RowNode->Down) {
        // Try this row node on!
        Result[nResult++] = RowNode->IDNum;
        for (RightNode = RowNode->Right; RightNode != RowNode;
             RightNode = RightNode->Right) {
            Cover(RightNode->Header);
        }
#pragma omp task
        Search(k + 1);
        // Ok, that node didn't quite work
        for (RightNode = RowNode->Left; RightNode != RowNode;
             RightNode = RightNode->Left) {
            UnCover(RightNode->Header);
        }
        Result[--nResult] = 0;
    }
#pragma omp taskwait
    UnCover(Column);
}

// --> Sodoku to Exact Cover conversion

// Functions that extract data from a given 3-digit integer index number in the
// format [N] [R] [C].
static inline int retNb(int N) { return N / (Size * Size); }
static inline int retRw(int N) { return (N / Size) % Size; }
static inline int retCl(int N) { return N % Size; }
static inline int retBx(int N) {
    return ((retRw(N) / N_input) * N_input) + (retCl(N) / N_input);
}
static inline int retSq(int N) { return retRw(N) * Size + retCl(N); }
static inline int retRn(int N) { return retNb(N) * Size + retRw(N); }
static inline int retCn(int N) { return retNb(N) * Size + retCl(N); }
static inline int retBn(int N) { return retNb(N) * Size + retBx(N); }
// Function that get 3-digit integer index from given info
static inline int getIn(int Nb, int Rw, int Cl) {
    return Nb * Size * Size + Rw * Size + Cl;
}

void PrintSolution(void) {
    int a, b;
    // int Sodoku[Size][Size] = {}
    int **Sodoku = malloc(Size * sizeof(int *));
    for (a = 0; a < Size; a++) {
        Sodoku[a] = malloc(Size * sizeof(int));
        for (b = 0; b < Size; b++)
            Sodoku[a][b] = -1;
    }
    for (a = 0; a < nResult; a++)
        Sodoku[retRw(Result[a])][retCl(Result[a])] = retNb(Result[a]);
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            if (b % N_input == 0) {
                printf("\t");
            }
            printf("%d", Sodoku[a][b] + 'A');
        }
        if ((a + 1) % N_input == 0) {
            printf("\n");
        }
        printf("\n");
    }
}

void BuildData(void) {
    printf("Building Data...\n");
    int a, b, c;
    int Index;
    nCol = Size * Size * 4;
    nRow = Size * Size * Size + 1;
    printf("First nested for...\n");
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            for (c = 0; c < Size; c++) {
                Index = getIn(c, a, b);
                Data[SQ_OFFSET + retSq(Index)][Index] =
                    1; // Constraint 1: Only 1 per square
                Data[RW_OFFSET + retRn(Index)][Index] =
                    1; // Constraint 2: Only 1 of per number per Row
                Data[CL_OFFSET + retCn(Index)][Index] =
                    1; // Constraint 3: Only 1 of per number per Column
                Data[BX_OFFSET + retBn(Index)][Index] =
                    1; // Constraint 4: Only 1 of per number per Box
            }
        }
    }
    printf("Second for (nCol)\n");
    for (a = 0; a < nCol; a++) {
        Data[a][nRow - 1] = 2;
    }
    printf("CreateMatrix...\n");
    CreateMatrix();

    printf("Four sequential fors...\n");
    for (a = 0; a < RW_OFFSET; a++) {
        Matrix[a][nRow - 1].IDName = 'S';
        Matrix[a][nRow - 1].IDNum = a;
    }
    for (a = RW_OFFSET; a < CL_OFFSET; a++) {
        Matrix[a][nRow - 1].IDName = 'R';
        Matrix[a][nRow - 1].IDNum = a - RW_OFFSET;
    }
    for (a = CL_OFFSET; a < BX_OFFSET; a++) {
        Matrix[a][nRow - 1].IDName = 'C';
        Matrix[a][nRow - 1].IDNum = a - CL_OFFSET;
    }
    for (a = BX_OFFSET; a < nCol; a++) {
        Matrix[a][nRow - 1].IDName = 'B';
        Matrix[a][nRow - 1].IDNum = a - BX_OFFSET;
    }
}

static inline void AddNumber(int N, int R, int C) {
    SolutionRow(RowHeader[getIn(N, R, C)]);
    MaxK++;
    Result[nResult++] = getIn(N, R, C);
}

void LoadPuzzle() {
    printf("Loading puzzle...\n");
    int a, b;
    int temp;
    for (a = 0; a < Size; a++) {
        for (b = 0; b < Size; b++) {
            scanf("%d ", &temp);
            if (temp != 0) {
                AddNumber(temp - 1, a, b);
            }
        }
    }
}

int main(void) {
    printf("Waiting for input...\n");
    scanf("%d ", &N_input);
    // n = 4;
    Size = N_input * N_input;
    int max_col = Size * Size * 4;
    int max_row = Size * Size * Size;

    Data = malloc(max_col * sizeof(int *));
    Matrix = malloc(max_col * sizeof(struct str_node *));
    int i;
    for (i = 0; i < max_col; i++) {
        Matrix[i] = malloc(max_row * sizeof(struct str_node));
        Data[i] = calloc(max_row, sizeof(int));
    }
    Result = malloc(max_row * sizeof(int));
    RowHeader = malloc(max_row * sizeof(int *));
    Finished = 0;
    MaxK = 0;
    nResult = 0;
    BuildData();
    LoadPuzzle();
    printf("Got input (puzzle loaded)\n");
    struct str_node *r;
    printf("Counting ones...\n");
    for (r = RootNode->Right; r != RootNode; r = r->Right) {
        r->count = countOnes(r);
    }

    printf("Searching...\n");
    Search(0);
    return 0;
}