#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
    printf("%d ", atoi(argv[1]));
    int i;
    for (i = 2; i <= atoi(argv[1]) * 2; i++) {
        printf("%d ", i);
        i++;
    }
    for (i = 1; i <= atoi(argv[1])*2; i++) {
        printf("%d ", i);
        i++;
    }
    return 0;
}
