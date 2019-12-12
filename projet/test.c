#include <stdio.h>
#include <stdlib.h>
#define MAX_COL 4
#define MAX_ROW 5
#define COL MAX_COL-1
#define ROW MAX_ROW
int main(int argc, char const *argv[])
{
    int test[0][0];
    int max_col = MAX_COL, max_row = MAX_ROW;
    int **test2;
    int (*testptr)[max_col][max_row] = &test;
    int ptroffest = &test[0][4] - &test[0][0];
    printf("ptroffset = %d\n", ptroffest);
    // &test = testptr;
    test2 = malloc(MAX_COL * sizeof(int *));
    int i, j;
    for (i = 0; i < MAX_COL; i++) {
        test2[i] = malloc(MAX_ROW * sizeof(int));
        for (j = 0; j < MAX_ROW; j++) {
            test[i][j] = i*MAX_ROW+j;
            test2[i][j] = i * MAX_ROW + j;
            printf("test[%d][%d] = %d\n",i,j, test[i][j]);
            printf("test2[%d][%d] = %d\n",i,j, test2[i][j]);
        }
    }
    
    
    return 0;
}
