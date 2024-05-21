#include <stdio.h>

int main() {
    int num = 0, amount = 0, amountOfZeros = 0, amountOfPositive = 0, amountOfEven = 0, sum = 0, max = -10000, min = 10000;

    while (scanf("%d", &num) == 1) {
        if (num < -10000 || num > 10000) {
            printf("\nError: Vstup je mimo interval!\n");
            return 100;
        }
        
        if (amount > 0) printf(", ");
        printf("%d", num);

        ++amount;
        if (num >= 0) num == 0 ? ++amountOfZeros : ++amountOfPositive;
        if (num % 2 == 0) ++amountOfEven;

        sum += num;
        if (num >= max) max = num;
        if (num <= min) min = num;
    }
    printf("\n");

    printf("Pocet cisel: %d\n", amount);
    printf("Pocet kladnych: %d\n", amountOfPositive);
    printf("Pocet zapornych: %d\n", amount - (amountOfPositive + amountOfZeros));
    printf("Procento kladnych: %.2f\n", (float)amountOfPositive / amount * 100);
    printf("Procento zapornych: %.2f\n", 100 - (float)(amountOfPositive + amountOfZeros) / amount * 100);

    printf("Pocet sudych: %d\n", amountOfEven);
    printf("Pocet lichych: %d\n", amount - amountOfEven);
    printf("Procento sudych: %.2f\n", (float)amountOfEven / amount * 100);
    printf("Procento lichych: %.2f\n", 100 - (float)amountOfEven / amount * 100);

    printf("Prumer: %.2f\n", (float)sum / amount);
    printf("Maximum: %d\n", max);
    printf("Minimum: %d\n", min);
    
    return 0;
}
