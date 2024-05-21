#include <stdio.h>
#include <stdbool.h> 

#define MAX_PRIME_NUMBER 999983 // Nejvetsi mozne prvocislo do 10^6
#define AMOUNT_OF_PRIMES 78498 // Pocet prvocisel do 10^6

int main() {
    // Teprve vsechny prvky jsou 0, kdyz algoritmus nachazi slozene cislo, meni hodnotu na 1
    bool numbers[(MAX_PRIME_NUMBER + 1)] = {false};
    int primes[AMOUNT_OF_PRIMES] = {0};

    // Eratosthenovo sito + zaznam prvocisel
    int index_of_prime = 0;
    for (long int i = 2; i <= MAX_PRIME_NUMBER; ++i) {
        if (numbers[i] == 0) {
            primes[index_of_prime] = i;
            ++index_of_prime;
            
            for (long int j = (i * i); j <= MAX_PRIME_NUMBER; j += i) {
                numbers[j] = 1;
            }
        }
    }

    long int n = -1; // Jestli 'n' se nezmeni, tak to znamena ze byl spatny vstup

    while(scanf("%li", &n) && n > 0) {
        printf("Prvociselny rozklad cisla %ld je:\n", n);
        long int n_copy = n;

        if (n_copy != 1) {
            for (int i = 0; i < AMOUNT_OF_PRIMES; ++i) {
                if (n_copy % primes[i] == 0) {
                    int exp = 0;

                    do {
                        n_copy /= primes[i];
                        ++exp;
                    } while (n_copy % primes[i] == 0);

                    printf("%i", primes[i]);

                    if (exp > 1) {
                        printf("^%i", exp);
                    }

                    if (n_copy == 1) {
                        n_copy = 0;
                        break;
                    }

                    printf(" x ");
                }
            }
        }

        if (n_copy != 0) {
            printf("%li", n);
        }

        printf("\n");
    }

    if (n != 0) {
        fprintf(stderr, "Error: Chybny vstup!\n");
        return 100;
    }

    return 0;
}
