#include <stdio.h>
#include <stdlib.h>

#define MATRICES_START_NUM 10
#define MATRICES_NUM_MULTIPLY_BY 2
#define ERROR_INVALID_INPUT 100
const char* const errorInvalidInput = "Error: Chybny vstup!\n";

// BASIC FUNCTIONS START (separate file for functions doesn't work in BRUTE)

int *copyArray(int *arr, int len) {
    int *copy = malloc(len * sizeof(int));
    if (copy != NULL) {
        for (int i = 0; i < len; ++i) {
            copy[i] = arr[i];
        }
    }
    return copy;
}

void printMatrix(int **mtrx, int n, int m) {
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < m; ++j) {
            printf("%i", mtrx[i][j]);

            if (j < (m - 1)) {
                printf(" ");
            }
        }
        printf("\n");
    }
}

void clearPointer(void *ptr) {
    if (ptr != NULL) {
        free(ptr);
        ptr = NULL;
    }
}

void clearMatrix(int ***mtrx, int n) {
    if (*mtrx != NULL) {
        for (int i = 0; i < n; ++i) {
            if ((*mtrx)[i] != NULL) {
                // Frees each row of the matrix
                free((*mtrx)[i]);
                (*mtrx)[i] = NULL;
            }
        }
        free(*mtrx);
        *mtrx = NULL;
    }
}

void clearMatrices(int ****matrices, int num, int *n) {
    if (*matrices != NULL) {
        for (int i = 0; i < num; ++i) {
            clearMatrix(&(*matrices)[i], n[i]);
        }
        free(*matrices);
        *matrices = NULL;
    }
}

int **createMatrix(int n, int m) {
    int **mtrx = calloc(n, sizeof(int *));
    if (mtrx != NULL) {
        for (int i = 0; i < n; ++i) {
            mtrx[i] = calloc(m, sizeof(int));

            if (mtrx[i] == NULL) {
                clearMatrix(&mtrx, (i + 1));
                break;
            }
        }
    }
    return mtrx;
}

int **copyMatrix(int **mtrx, int n, int m) {
    int **mtrxCopy = createMatrix(n, m);
    if (mtrxCopy != NULL) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                mtrxCopy[i][j] = mtrx[i][j];
            }
        }
    }
    return mtrxCopy;
}

int reallocate(int ****matrices, int **n, int **m, char **ops, int newSize) {
    int ret = EXIT_SUCCESS;
    *matrices = realloc(*matrices, newSize * sizeof(int **));
    *n = realloc(*n, newSize * sizeof(int));
    *m = realloc(*m, newSize * sizeof(int));
    *ops = realloc(*ops, newSize * sizeof(char));

    if (*matrices == NULL || *n == NULL || *m == NULL || *ops == NULL) {
        ret = EXIT_FAILURE;
    }
    return ret;
}

// BASIC FUNCTIONS END ///

int **readMatrix(int n, int m) {
    int **mtrx = createMatrix(n, m);
    if (mtrx != NULL) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (scanf("%i", &(mtrx[i][j])) != 1) {
                    clearMatrix(&mtrx, n);
                    // Ends the loop for invalid input
                    goto invalidInput;
                }
            }
        }
    }
    invalidInput:
    return mtrx;
}

int getOperation(char *op) {
    int ret = EXIT_FAILURE;
    for (int i = 0; i < 2; ++i) {
        if (scanf("%c", op) == 1 && (*op == '+' || *op == '-' || *op == '*')) {
            ret = EXIT_SUCCESS;
        }
    } 
    return ret;
}

int add(int **mtrx1, int **mtrx2, int n1, int m1, int n2, int m2, char sign) {
    int ret = EXIT_FAILURE;

    if (n1 == n2 && m1 == m2) {
        int s = (sign == '+') ? 1 : -1;
        for (int i = 0; i < n1; ++i) {
            for (int j = 0; j < m1; ++j) {
                mtrx1[i][j] = mtrx1[i][j] + s * mtrx2[i][j];
            }
        }
        ret = EXIT_SUCCESS;
    }
    return ret;
}

void multiply(int ***mtrx1, int **mtrx2, int n1, int m1, int n2, int m2) {
    int **result = NULL;
    if (m1 == n2) {
        result = createMatrix(n1, m2);
        for (int i = 0; i < n1; ++i) {
            for (int j = 0; j < m2; ++j) {
                for (int k = 0; k < m1; ++k) {
                    result[i][j] += (*mtrx1)[i][k] * mtrx2[k][j];
                }
            }
        }
    }
    clearMatrix(mtrx1, n1);
    *mtrx1 = result;
}

int main(int argc, char *argv[]) {
    int ret = EXIT_SUCCESS;
    int size = MATRICES_START_NUM;
    // All matrices
    int ***matrices = malloc(size * sizeof(int **));
    int numMatrices = 0;
    // Number of matrix rows and columns
    int *n = malloc(size * sizeof(int));
    int *m = malloc(size * sizeof(int));
    // Operations
    char *ops = malloc(size * sizeof(char));

    // Input processing
    for (int i = 0; (scanf("%i%i", &n[i], &m[i]) == 2) && (n[i] > 0 && m[i] > 0); ++i) {
        matrices[i] = readMatrix(n[i], m[i]);
        ++numMatrices;

        if (matrices[i] == NULL) {
            ret = EXIT_FAILURE;
            break;
        }

        if (getOperation(&(ops[i])) == EXIT_FAILURE) {
            break;
        }

        if ((i + 2) >= size) {
            size *= MATRICES_NUM_MULTIPLY_BY;
            
            // If memory reallocation is not possible
            if (reallocate(&matrices, &n, &m, &ops, size) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
                break;
            }
        }
    }

    int *nCopy = copyArray(n, numMatrices);
    int *mCopy = copyArray(m, numMatrices);
    if (nCopy == NULL || mCopy == NULL) {
        ret = EXIT_FAILURE; 
        goto finishProgram;
    }

    if (ret != EXIT_FAILURE) {
        // In case the first operation is a multiplication
        int **resultingMtrx = (ops[0] == '*') ? NULL : copyMatrix(matrices[0], n[0], m[0]);
        
        for (int i = 0; i < numMatrices; ++i) {
            int **currentMtrx = copyMatrix(matrices[i], nCopy[i], mCopy[i]);

            if (currentMtrx == NULL) {
                ret = EXIT_FAILURE;
                // Free's memory
                int tempRows = (i == 0) ? n[0] : n[i - 1];
                clearMatrix(&resultingMtrx, tempRows);
                goto finishProgram;
            }

            if (i != (numMatrices - 1) && ops[i] == '*') {
                int j;
                for (j = i; ops[j] == '*' && j < (numMatrices - 1); ++j) {
                    multiply(&currentMtrx, matrices[j + 1], n[j], m[j], n[j + 1], m[j + 1]);
                    n[j + 1] = n[j];
                    // Zero means no operation
                    ops[j] = '0';

                    // Will return zero if multiplication is impossible
                    if (currentMtrx == NULL) {
                        ret = EXIT_FAILURE;
                        int tempRows = (i == 0) ? n[0] : n[i - 1];
                        clearMatrix(&resultingMtrx, tempRows);
                        goto finishProgram;
                    }
                }

                /* The sizes of all matrices that participated in the multiplication 
                are changed to the sizes of the last matrix */
                for (int k = i; k < j; ++k) {
                    n[k] = n[j];
                    m[k] = m[j];
                }

                // In case the first operation is a multiplication
                if (i == 0) {
                    resultingMtrx = copyMatrix(currentMtrx, n[j], m[j]);
                    
                    // If memory allocation for the copy is not possible
                    if (resultingMtrx == NULL) {
                        ret = EXIT_FAILURE;
                        clearMatrix(&currentMtrx, n[j]);
                        goto finishProgram;
                    }
                }
            }

            if (i != 0 && (ops[i - 1] == '+' || ops[i - 1] == '-')) {
                // There is also subtraction here (if ops[i - 1] == '-')
                if(add(resultingMtrx, currentMtrx, n[i - 1], m[i - 1], n[i], m[i], ops[i - 1]) == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                    clearMatrix(&resultingMtrx, n[i - 1]);
                    clearMatrix(&currentMtrx, n[i]);
                    goto finishProgram;
                }
            }

            clearMatrix(&currentMtrx, nCopy[i]);
        }

        printf("%i %i\n", n[numMatrices - 1], m[numMatrices - 1]);
        printMatrix(resultingMtrx, n[numMatrices - 1], m[numMatrices - 1]);
        clearMatrix(&resultingMtrx, n[numMatrices - 1]);
    }
    
    finishProgram:

    clearMatrices(&matrices, numMatrices, nCopy);
    clearPointer(nCopy); 
    clearPointer(mCopy);
    clearPointer(n); 
    clearPointer(m); 
    clearPointer(ops);

    if (ret == EXIT_FAILURE) {
        fprintf(stderr, "%s", errorInvalidInput);
        ret = ERROR_INVALID_INPUT;
    }

    return ret;
}
