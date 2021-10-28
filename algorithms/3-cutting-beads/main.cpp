#include <array>

using namespace std;

const int INT_MAX_LEN = 12;

/**
 * Read 1 non-negative number from standard input ignoring any non-digit characters.
 *
 * @return number if one was in input, EOF otherwise.
 */
int get_number_from_stdin() {
    unsigned char str[INT_MAX_LEN];
    int c = 0, i = 0;

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

int main() {
    return 0;
}
