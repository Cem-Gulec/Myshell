#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) 
{
    int x;
    int y;

   
    int sum;

    scanf("%d %d", &x, &y);

    sum = x + y;
    printf("%d + %d = %d\n", x, y, sum);

    return 0;
}