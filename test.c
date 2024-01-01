#include <stdio.h>
#include <stdlib.h>


int main() {
    int *ptr = malloc(5 * sizeof(int));
    free(ptr);
    ptr[2] = 3;
}