#include <stdio.h>
#include <stdlib.h>

// Defines to customize memory allocation
#define MATRIX_START_ROWS 3
#define MATRIX_ROWS_MULTIPLY_BY 2
#define MATRIX_START_COLS 5
#define MATRIX_COLS_MULTIPLY_BY 2
#define MATRICES_START_NUM 10
#define MATRICES_NUM_MULTIPLY_BY 2
// For numbers processing
#define MAX_POSSIBLE_NUM_AND_CHARS_LENGTH 100

#define ERROR_INVALID_INPUT 100
const char* const errorInvalidInput = "Error: Chybny vstup!\n";

// basic.c:START (separate file for functions doesn't work in BRUTE)

void printMatrix(int **mtrx, int n, int m) {
    printf("[");
    for (int i = 0; i < n; ++i){
        for (int j = 0; j < m; ++j) {
            printf("%i", mtrx[i][j]);

            if (j < m - 1) printf(" ");
        }

        if (i < n - 1) printf("; ");
    }
    printf("]\n");
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

int reallocate(int ****matrices, char **letters, int **n, int **m, int newSize) {
    int ret = EXIT_SUCCESS;
    *matrices = realloc(*matrices, newSize * sizeof(int **));
    *letters = realloc(*letters, newSize * sizeof(char));
    *n = realloc(*n, newSize * sizeof(int));
    *m = realloc(*m, newSize * sizeof(int));

    if (*matrices == NULL || *letters == NULL || *n == NULL || *m == NULL) {
        ret = EXIT_FAILURE;
    }
    return ret;
}

// If any matrix matches this letter
int isItAMatrixLetter(char l, char *letters, int len) {
    int ret = EXIT_FAILURE;
    for (int i = 0; i < len; ++i) {
        if (l == letters[i]) {
            ret = EXIT_SUCCESS;
            break;
        }
    }
    return ret;
}

int getMatrixLetterNumber(char l, char *letters, int len) {
    int num = 0;
    for (int i = 0; i < len; ++i) {
        if (l == letters[i]) {
            num = i;
            break;
        }
    }
    return num;
}

// basic.c:END

char checkString(char *str) {
    char ret = EXIT_SUCCESS;

    for (int i = 0; str[i] != '\0'; ++i) {
        if (str[i] == ';' || str[i] == ']') {
            ret = str[i];
            break;
        } 
        else if (! ((str[i] >= '0' && str[i] <= '9') || str[i] == ' ' || str[i] == '-')) {
            ret = EXIT_FAILURE;
            break;
        }
    }
    return ret;
}

int **readMatrix(int *n, int *m) {
    int sizeRows = MATRIX_START_ROWS;
    int sizeCols = MATRIX_START_COLS;
    int rows = 0, prevCols = 0;
    int **mtrx = calloc(sizeRows, sizeof(int *));
    // In case of memory allocation error
    if (mtrx == NULL) {
        goto finishFunction;
    }

    // Skips 2 characters
    char skip;
    if (scanf("%*c%c", &skip) == 1) {
        /* Do nothing, because "warning: ignoring return value of ‘scanf’, 
        declared with attribute warn_unused_result [-Wunused-result]" */
    }

    char strRet; 
    do {
        mtrx[rows] = calloc(sizeCols, sizeof(int));
        // In case of memory allocation error
        if (mtrx[rows] == NULL) {
            goto finishFunction;
        }
       
        int cols = 0;
        do {
            char str[MAX_POSSIBLE_NUM_AND_CHARS_LENGTH];           
            int scanfRet = scanf("%s", str);
            // Return ';', ']', EXIT_SUCCES or EXIT_FAILURE
            strRet = checkString(str);

            if(scanfRet == 1 && strRet != EXIT_FAILURE) {
                mtrx[rows][cols] = atoi(str);
            } else {
                strRet = EXIT_FAILURE;
                break;
            }

            if (cols + 2 >= sizeCols) {
                sizeCols *= 2;
                mtrx[rows] = realloc(mtrx[rows], sizeCols * sizeof(int));

                // In case of reallocation error
                if (mtrx[rows] == NULL) {
                    strRet = EXIT_FAILURE;
                    break;
                }
            }
            ++cols;
        }
        while (strRet == EXIT_SUCCESS);

        // Checks if all columns are of the same length
        if (rows > 0 && cols != prevCols) {
            strRet = EXIT_FAILURE;
        }
        prevCols = cols;

        if (rows + 2 >= sizeRows) {
            sizeRows *= 2;
            mtrx = realloc(mtrx, sizeRows * sizeof(int *));

            if (mtrx == NULL) {
                strRet = EXIT_FAILURE;
            }
        }

        if (strRet == EXIT_FAILURE) {
            // All errors lead here
            finishFunction:
            clearMatrix(&mtrx, (rows + 1));
            break;
        }

        ++rows;
    } 
    while (strRet == ';');

    // New sizes of matrix
    *n = rows, *m = prevCols;

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
    // Matrix letters
    char *letters = malloc(size * sizeof(char));

    // Input processing
    for (int i = 0; ; ++i) {
        if (scanf("%c", &letters[i]) == 1 && letters[i] == '\n') {
            // If there are 2 '\n', ends the loop
            if (scanf("%c", &letters[i]) == 1 && letters[i] == '\n') {
                break;
            }
        }
        else if(letters[i] < 'A' && letters[i] > 'Z') {
            ret = EXIT_FAILURE;
            break;
        }
        
        matrices[i] = readMatrix(&(n[i]), &(m[i]));
        ++numMatrices;

        if (matrices[i] == NULL) {
            // Invalid matrix input or memory allocation error
            ret = EXIT_FAILURE;
            break;
        }

        if ((i + 2) >= size) {
            size *= MATRICES_NUM_MULTIPLY_BY;
            
            // If memory reallocation is not possible
            if (reallocate(&matrices, &letters, &n, &m, size) == EXIT_FAILURE) {
                ret = EXIT_FAILURE;
                break;
            }
        }
    }

    size = MATRICES_START_NUM;
    // Array for operands, 0 is the first matrix, 1 is the second and so on
    int *mtrxId = malloc(size * sizeof(int));
    // Operations
    char *ops = malloc(size * sizeof(char));
    // Expression length
    int expLen = 0;

    // Reading expression
    if (ret == EXIT_SUCCESS) {
        for (int i = 0; ; ++i) {
            char letter;
            int letterRet = scanf("%c", &letter);
            int opsRet = scanf("%c", &(ops[i]));
            int answer = isItAMatrixLetter(letter, letters, numMatrices);
            ++expLen;

            // Numbers (id's) are easier to work with than letters
            mtrxId[i] = getMatrixLetterNumber(letter, letters, numMatrices); 

            if (! (letterRet == 1 && opsRet == 1 && answer == EXIT_SUCCESS &&
            (ops[i] == '+' || ops[i] == '-' || ops[i] == '*' || ops[i] == '\n'))) {
                // Invalid input
                ret = EXIT_FAILURE;
                break;
            }
            else if (ops[i] == '\n') {
                // End of file
                break;
            }

            if (i + 2 >= size) {
                size *= MATRICES_NUM_MULTIPLY_BY;
                mtrxId = realloc(mtrxId, size * sizeof(int));
                ops = realloc(ops, size * sizeof(char));

                if (mtrxId == NULL || ops == NULL) {
                    ret = EXIT_FAILURE;
                    break;
                }
            }
        }
    }

    /* Sizes for each matrix in the expression,
    because the algorithm will change them (e.g when multiplying) */
    int rows[size], cols[size];
    for (int i = 0; i < expLen; ++i) {
        rows[i] = n[mtrxId[i]];
        cols[i] = m[mtrxId[i]];
    }

    // The algorithm always checks the next operation, and only then the current one
    if (ret == EXIT_SUCCESS) {
        /* In case the first operation is a multiplication, 
        'mtrxId[0]' - first matrix number in the expression */
        int **resultingMtrx = (ops[0] == '*') ? NULL : copyMatrix(matrices[mtrxId[0]], rows[0], cols[0]);
        
        for (int i = 0; i < expLen; ++i) {
            int **currentMtrx = copyMatrix(matrices[mtrxId[i]], n[mtrxId[i]], m[mtrxId[i]]);

            if (currentMtrx == NULL) {
                ret = EXIT_FAILURE;
                // Free's memory
                int tempRows = (i == 0) ? rows[0] : cols[i-1];
                clearMatrix(&resultingMtrx, tempRows);
                goto finishProgram;
            }

            if (i != (expLen - 1) && ops[i] == '*') {
                // Multiplies all matrcies in row (of '*')
                int j;
                for (j = i; ops[j] == '*' && j < (expLen - 1); ++j) {
                    multiply(&currentMtrx, matrices[mtrxId[j + 1]], 
                    rows[j], cols[j], rows[j + 1], cols[j + 1]);

                    // New sizes after multiplication
                    rows[j + 1] = rows[j];
                
                    // Zero means no operation
                    ops[j] = '0';

                    // Multiply() will return NULL if multiplication is impossible
                    if (currentMtrx == NULL) {            
                        ret = EXIT_FAILURE;
                        int tempRows = (i == 0) ? rows[0] : rows[i - 1];
                        clearMatrix(&resultingMtrx, tempRows);
                        goto finishProgram;
                    }
                }

                /* The sizes of all matrices that participated in the multiplication 
                are changed to the sizes of the last matrix */
                for (int k = i; k < j; ++k) {
                    rows[k] = rows[j];
                    cols[k] = cols[j];
                }

                // In case the first operation is a multiplication
                if (i == 0) {
                    resultingMtrx = copyMatrix(currentMtrx, rows[j], cols[j]);
                    
                    // If memory allocation for the copy is not possible
                    if (resultingMtrx == NULL) {
                        ret = EXIT_FAILURE;
                        clearMatrix(&currentMtrx, rows[j]);
                        goto finishProgram;
                    }
                }
            }

            if (i != 0 && (ops[i - 1] == '+' || ops[i - 1] == '-')) {
                // There is also subtraction here (if ops[i - 1] == '-')
                if (add(resultingMtrx, currentMtrx, rows[i - 1], cols[i - 1], 
                rows[i], cols[i], ops[i - 1]) == EXIT_FAILURE) {
                    ret = EXIT_FAILURE;
                    clearMatrix(&resultingMtrx, rows[i - 1]);
                    clearMatrix(&currentMtrx, rows[i]);
                    goto finishProgram;
                }
            }

            clearMatrix(&currentMtrx, n[mtrxId[i]]);
        }

        printMatrix(resultingMtrx, rows[expLen - 1], cols[expLen - 1]);
        clearMatrix(&resultingMtrx, rows[expLen - 1]);
    }
    
    finishProgram:

    clearMatrices(&matrices, numMatrices, n);
    clearPointer(mtrxId);
    clearPointer(ops);
    clearPointer(letters);
    clearPointer(n); 
    clearPointer(m); 

    if (ret == EXIT_FAILURE) {
        fprintf(stderr, "%s", errorInvalidInput);
        ret = ERROR_INVALID_INPUT;
    }

    return ret;
}
