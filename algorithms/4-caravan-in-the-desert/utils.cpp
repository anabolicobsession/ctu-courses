#include <iostream>

const int INT_MAX_LEN = 10;

int get_integer_from_stdin() {
    unsigned char str[INT_MAX_LEN];
    int c, i = 0;

    while ((c = getchar()) != EOF && !isdigit(c));
    while (c != EOF && isdigit(c)) {
        str[i++] = (unsigned char)c;
        c = getchar();
    }

    int num = 0, m = 1;
    while (i--) {
        num += (str[i] - '0') * m;
        m *= 10;
    }

    return m != 1 ? num : EOF;
}
