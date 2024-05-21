#include <stdio.h>
#include <stdlib.h>

#define ERROR_CHYBNY_VSTUP 100
#define ERROR_CHYBNA_DELKA_VSTUPU 101
#define LETTERS_IN_ALPHABET 52
#define FIRST_MALLOC_CHARS 25
#define MALLOC_CHARS_MULTIPLY_FOR_STEP 2
#define MAX_POSSIBLE_LEVENSHTEIN_DIST 999999

static char alphabet[LETTERS_IN_ALPHABET] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int max(int n, int m) {
    return n > m ? n : m;
}

int letter_num_in_alphabet(char l) {
    int n = 0;
    while (l != alphabet[n]) { ++n; }
    return n;
}

int is_this_char_in_alphabet(char c) {
    int answer = 0;
    for (int i = 0; i < LETTERS_IN_ALPHABET; ++i) {
        if (c == alphabet[i]) {
            answer = 1;
            break;
        }
    }
    return answer;
}

int str_len(char *str) {
    int len = 0;
    while (str[len] != '\n' && str[len] != '\0') { ++len; }
    return len;
}

int record_string_to(char **str) {
    int ret = 0, malloc_chars = FIRST_MALLOC_CHARS;
    *str = (char *)malloc(malloc_chars * sizeof(char));
    **str = getchar();
    for (int i = 0; *(*str + i) != '\n'; ++i, *(*str + i) = getchar()) {
        if (is_this_char_in_alphabet(*(*str + i)) == 1) {
            // Jestli pro nasledujici symbol nebyla vyhrazena pamet
            if ((i + 1) == malloc_chars) {
                malloc_chars *= MALLOC_CHARS_MULTIPLY_FOR_STEP;
                *str = realloc(*str, malloc_chars * sizeof(char));
            }
        } else {
            ret = 100;
            break;
        }
    }
    return ret;
}

int compare_letters(char l1, char l2) {
    int sh = letter_num_in_alphabet(l2) - letter_num_in_alphabet(l1);
    if (sh < 0) { sh += LETTERS_IN_ALPHABET; }
    return sh;
}

char shift_letter(char l, int sh) {
    int shl = letter_num_in_alphabet(l) + sh;
    return alphabet[shl % LETTERS_IN_ALPHABET];
}

void shift_string(char *str, int sh) {
    for (int i = 0; *(str + i) != '\n'; ++i) {
        *(str + i) = shift_letter(*(str + i), sh);
    }
}

void print_and_shift_string(char *str, int sh) {
    for (int i = 0; *(str + i) != '\n'; ++i) {
        printf("%c", shift_letter(*(str + i), sh));
    }
    printf("\n");
}

void decrypt_string_simple(char *cipher_str, char *str_with_noise) {
    int shifts_array[LETTERS_IN_ALPHABET] = {0};
    int len = str_len(cipher_str);

    for (int i = 0; i < len; ++i) {
        shifts_array[compare_letters(*(cipher_str + i), *(str_with_noise + i))] += 1;
    }

    int max = 0, shift = 0;
    for (int i = 0; i < LETTERS_IN_ALPHABET; ++i) {
        if (shifts_array[i] > max) {
            max = shifts_array[i];
            shift = i; 
        }
    }
    print_and_shift_string(cipher_str, shift);
}

int min(int n, int m, int l) { 
    return n < m ? (n < l ? n : l) : (m < l ? m : l); 
}

void swap_arrays(int a1[], int a2[], int len) {
    for (int i = 0; i < len; ++i) {
        int t = a1[i];
        a1[i] = a2[i];
        a2[i] = t;
    }
}

int get_levenshtein_dist(char *str1, char *str2) {
    int str1_len = (str_len(str1) + 1);
    int str2_len = (str_len(str2) + 1);
    if (str2_len > str1_len) {
        return get_levenshtein_dist(str2, str1);
    }

    int prev_row[str2_len];
    int curr_row[str2_len];
    for (int i = 0; i <= str2_len; ++i) {
        prev_row[i] = 0;
        curr_row[i] = 0;
    } 

    for (int i = 1; i <= str2_len; ++i) {
        prev_row[i] = i;
    }

    for (int i = 1; i <= str1_len; ++i) {
        curr_row[0] = i;

        for (int j = 1; j < str2_len; ++j) {
            int cost_up = prev_row[j] + 1;
            int cost_left = curr_row[j - 1] + 1;
            int cost_replacement = (str1[i-1] == str2[j-1]) ? 0 : 1;
            int cost_substitution = prev_row[j - 1] + cost_replacement;
            curr_row[j] = min(cost_up, cost_left, cost_substitution);
        }

        swap_arrays(prev_row, curr_row, str2_len);
    }

    return prev_row[str2_len - 1];
}

void decrypt_by_levenstein_dist(char *cipher_str, char *str_with_noise) {
    int shift = 0;
    int min_lvs_dist = MAX_POSSIBLE_LEVENSHTEIN_DIST;

    for (int sh = 0; sh < LETTERS_IN_ALPHABET; ++sh) {
        shift_string(cipher_str, sh);
        
        int lvs_dist = get_levenshtein_dist(cipher_str, str_with_noise);
        if (lvs_dist < min_lvs_dist) {
            min_lvs_dist = lvs_dist;
            shift = sh;
        }

        shift_string(cipher_str, (LETTERS_IN_ALPHABET - sh));
    }

    print_and_shift_string(cipher_str, shift);
}

void clear_pointer(char **str) {
    if (*str != NULL) {
        free(*str);
        *str = NULL;
    }
}

int compare_strings(char *str1, char *str2) {
    int ret = 1, len = str_len(str2);
    for (int i = 0; i < len; ++i) {
        if (*(str1 + i) != *(str2 + i)) { 
            ret = 0;
            break;
        }
    }
    return ret;
}

int main(int argc, char **argv) {
    int is_there_an_optional_arg = 0;
    for (int i = 0; i < argc; ++i) {
        is_there_an_optional_arg = compare_strings(*(argv + i), "-prp-optional");
    }
    
    char *cipher_str = NULL, *str_with_noise = NULL;
    int ret1 = record_string_to(&cipher_str);
    int ret2 = record_string_to(&str_with_noise);
    int ret = max(ret1, ret2);

    if (ret == 0 && is_there_an_optional_arg == 0) {
        if (str_len(cipher_str) == str_len(str_with_noise)) {
            decrypt_string_simple(cipher_str, str_with_noise);
        } else {
            ret = ERROR_CHYBNA_DELKA_VSTUPU;
        }
    } 
    else if (ret == 0 && is_there_an_optional_arg == 1) {
        decrypt_by_levenstein_dist(cipher_str, str_with_noise);
    }

    clear_pointer(&cipher_str);
    clear_pointer(&str_with_noise);

    switch (ret) {
        case ERROR_CHYBNY_VSTUP:
            fprintf(stderr, "Error: Chybny vstup!\n");
            break;
        case ERROR_CHYBNA_DELKA_VSTUPU:
            fprintf(stderr, "Error: Chybna delka vstupu!\n");
            break;
    }

    return ret;
}
