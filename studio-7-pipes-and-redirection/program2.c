// program2.c
#include <stdio.h>

int main(void) {
    char input[100];

    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        printf("Program 2 got: %s", input);
    }

    return 0;
}
