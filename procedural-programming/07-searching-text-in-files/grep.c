#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define RED_COLOR "\x1B[31m"
#define RESET_COLOR "\x1B[0m"
#define REGEX_ARGUMENT "-E"
#define COLOR_ARGUMENT "--color=always"

enum {
    START_SIZE = 10, SIZE_MULTIPLY_BY = 2, MAX_POSSIBLE_PATTERN = 9999,
    NO_END_LINE = 1, SUBSTRING_FOUND = 2, SUBSTRING_NOT_FOUND = 3,
    FILE_ERROR = 200, INVALID_INPUT_ERROR = 201
};

int patterns_starts[1000] = {0};
int patterns_ends[1000] = {0};

typedef struct line {
    char *string;
    int capacity;
    int length;
    int tmp;
} line_t;

void init_line(line_t *line) {
    line->capacity = START_SIZE;
    line->string = (char *)malloc(line->capacity * sizeof(char));
    line->length = 0;
    line->tmp = 0;
}

void resize_line(line_t *line) {
    line->capacity *= SIZE_MULTIPLY_BY;
    line->string = (char *)realloc(line->string, line->capacity * sizeof(char));
}

int read_line(line_t *line, FILE *file) {
    int ret = NO_END_LINE;
    char c; 
    while(ret == NO_END_LINE) {
        c = getc(file);
        (c == '\n' || c == EOF) ? ret = c : (line->string[line->length++] = c);
        if (line->length + 1 >= line->capacity) resize_line(line);
    }
    return ret;
}

void print_line(line_t *line) {
    for (int i = 0; i < line->length; ++i) {
        printf("%c", line->string[i]);
    }
    printf("\n");
}

void free_line(line_t *line) {
    if (line->string != NULL) {
        free(line->string);
        line->string = NULL;
        line->capacity = 0;
        line->length = 0;
        line->tmp = 0;
    }
}

typedef struct pattern {
    char *string;
    int capacity;
    int length;
    char mode;
    char regex_letter;
    int regex_index;
} pattern_t;

void resize_pattern(pattern_t *pattern) {
    pattern->capacity *= SIZE_MULTIPLY_BY;
    pattern->string = (char *)realloc(pattern->string, pattern->capacity * sizeof(char));
}

void init_pattern(pattern_t *pattern, char *user_pattern) {
    pattern->capacity = START_SIZE;
    pattern->string = (char *)malloc(pattern->capacity * sizeof(char));
    pattern->mode = '0';
    pattern->regex_letter = '0';
    pattern->regex_index = -1;
    int j = 0;
    for (int i = 0; user_pattern[i] != '\0'; ++i, ++j) {
        char c = user_pattern[i];
        if (c > '+' && c < '~' && c != '?') {
            pattern->string[j] = c;
        } else if (c == '?' || c == '*' || c == '+') {
            pattern->mode = c;
            pattern->regex_letter = user_pattern[i - 1];
            pattern->regex_index = i - 1;
            j -= 2;
        } else {
            exit(INVALID_INPUT_ERROR);
        }

        if (i + 2 >= pattern->capacity) resize_pattern(pattern);
    }
    pattern->length = j;
}

void free_pattern(pattern_t *pattern) {
    if (pattern->string != NULL) {
        free(pattern->string);
        pattern->string = NULL;
        pattern->capacity = 0;
        pattern->length = 0;
    }
}

int open_file(FILE **file, char *name) {
    *file = fopen(name, "r");
    if (*file == NULL) return false;
    return true;
}

void close_file(FILE **file) {
    if (*file != NULL && fclose(*file) == EOF) exit(FILE_ERROR);
}

int find_pattern(line_t *line, pattern_t *pattern) {
    int ret = SUBSTRING_NOT_FOUND;
    if (pattern->length > line->length) return 0;
    
    int limit = 0;
    bool regex_letter_should_be = false;
    switch (pattern->mode) {
        case '?':
        limit = 1;
        break;
        case '+':
        regex_letter_should_be = true;
        case '*':
        limit = MAX_POSSIBLE_PATTERN;
        break;
    }

    for (int i = 0; i + pattern->length <= line->length; ++i) {
        int j, k, unmatched_chars_num = 0;
        for (j = 0, k = 0; i + j < line->length && k < pattern->length; ++j, ++k) {
            if (line->string[i + j] == pattern->string[k]) {
                if (regex_letter_should_be == true && j == pattern->regex_index && 
                    line->string[i + j] != pattern->regex_letter) {
                    break;
                }
                
                continue;
            } else if (k == pattern->regex_index && line->string[i + j] == pattern->regex_letter && unmatched_chars_num < limit) {
                ++unmatched_chars_num; 
                --k;
            } else {
                break;
            }
        }
        
        if (k == pattern->length) {
            patterns_starts[line->tmp] = i;
            patterns_ends[line->tmp] = i + j;
            line->tmp += 1;
            ret = SUBSTRING_FOUND;
        }
    }
    return ret;
}

int compare_strings(char *str1, char *str2) {
    for (int i = 0; str1[i] != '\0' && str2[i] != '\0'; ++i) {
        if (str1[i] != str2[i]) return false;
    }
    return true;
}

int check_argument(char *argv[], int argc, char *arg) {
    for (int i = 0; i < argc; ++i) {
        if (compare_strings(argv[i], arg) == true) {
            return i;
        }
    }   
    return 0;
}

void print_colored_line(line_t *line, pattern_t *pattern) {
    int id = 0;
    for (int i = 0; i < line->length; ++i) {
        if (i == patterns_starts[id]) {
            printf("\x1b[01;31m\x1b[K");
            while(i < patterns_ends[id]) {
                printf("%c", line->string[i++]);
            }
            printf("\x1b[m\x1b[K");
            --i;
            
            if (id + 1 < line->tmp) {
                id += 1;
            }
            continue;
        }
        
        printf("%c", line->string[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int ret = EOF;
    FILE *file = NULL;
    pattern_t pattern;
    bool color_argument = false;
    if (argc > 1) {
        if (open_file(&file, argv[argc - 1]) == false) {
            file = stdin;
            init_pattern(&pattern, argv[argc - 1]);
        } else {
            init_pattern(&pattern, argv[argc - 2]);
        }

        if (check_argument(argv, argc, COLOR_ARGUMENT) == true) {
            color_argument = true;
        }
        ret = '\n';
    }
    
    bool patterns_found = false;
    while (ret == '\n') {
        line_t line;
        init_line(&line);
        if ((ret = read_line(&line, file)) == '\n' || (ret = read_line(&line, file)) == EOF) {
            if (find_pattern(&line, &pattern) == SUBSTRING_FOUND) {
                color_argument == true ? print_colored_line(&line, &pattern) : print_line(&line);
                patterns_found = true;
            }
        }
        free_line(&line);
    }
    
    if (argc > 1) {
        free_pattern(&pattern);
    }
    close_file(&file);
    return !patterns_found;
}