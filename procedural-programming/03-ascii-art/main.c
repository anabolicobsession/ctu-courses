#include <stdio.h>
#define MIN_VALUE_FROM_THE_INTERVAL 3
#define MAX_VALUE_FROM_THE_INTERVAL 69
#define MIN_VALUE_FOR_FENCE 1
    
int main() {
    // c = -1 pro pripad, kdyz ho uzivatel nestanovi
    int a, b, c = -1;
    
    /*
    scanf("%d%d%d", &a, &b, &c) funguje spatne, jestli b nebo c se rovna 'x', 
    protoze konvertuje pismeno v cislo misto chybneho vystupa, proto jsem pouzil trikrat if ()
    */

    if (scanf("%d\n", &a) != 1) {
        fprintf(stderr, "Error: Chybny vstup!\n");
        return 100;
    }

    if (scanf("%d\n", &b) != 1) {
        fprintf(stderr, "Error: Chybny vstup!\n");
        return 100;
    }

    if (a == b && a % 2 == 1) {
        if (scanf("%d\n", &c) != 1) {
            fprintf(stderr, "Error: Chybny vstup!\n");
            return 100;
        }
    }

    if (a < MIN_VALUE_FROM_THE_INTERVAL || a > MAX_VALUE_FROM_THE_INTERVAL || 
    b < MIN_VALUE_FROM_THE_INTERVAL || b > MAX_VALUE_FROM_THE_INTERVAL) {
        fprintf(stderr, "Error: Vstup mimo interval!\n");
        return 101;
    } else if (a % 2 == 0) {
        fprintf(stderr, "Error: Sirka neni liche cislo!\n");
        return 102;
    } else if (c != -1 && (c >= a || c < MIN_VALUE_FOR_FENCE)) {
        fprintf(stderr, "Error: Neplatna velikost plotu!\n");
        return 103;
    }
    
    for (int i = a / 2; i > 0; --i) {
        for (int j = 0; j <= a / 2 + (a / 2 - i); ++j) {
            if (j == i || j == a - i - 1) {
                printf("X");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < a; ++j) {
            if (i == 0 || j == 0 || i == b - 1 || j == a - 1) {
                printf("X");
            // Overuje se, jestli ve vstupu nebylo 'c'
            } else if (c == -1) {
                printf(" ");
            } else {
                if ((i % 2 == 1 && j % 2 == 1) || (i % 2 == 0 && j % 2 == 0)) {
                    printf("o");
                } else {
                    printf("*");
                }
            } 
        }

        if (i >= a - c) {
            for (int o = c; o > 0; --o) {
                if (o % 2 == 0 && (i == a - c ||i == b - 1)) {
                    printf("-");
                } else if (o % 2 == 1) {
                    printf("|");
                } else {
                    printf(" ");
                }
            }
        }

        printf("\n");
    }

    return 0;
}
