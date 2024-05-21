#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#define SYSCALL1 1
#define SYSCALL3 3
#define SYSCALL4 4

#define MAX_HEX_INTEGER_LEN 8

int syscall1(int b)
{
    int ret;
    asm volatile (
                "int $0x80" 
                : "=a"(ret)
                : "a"(SYSCALL1), "b"(b) 
                : "memory"
                );
    return  ret;
}

int syscall3(int b, int c, int d)
{
    int ret;
    asm volatile (
                "int $0x80"
                : "=a"(ret)
                : "a"(SYSCALL3), "b"(b), "c"(c), "d"(d)
                : "memory"
                );
    return ret;
}

int syscall4(int b, int c, int d)
{
    int ret;
    asm volatile (
                "int $0x80"
                : "=a"(ret)
                : "a"(SYSCALL4), "b"(b), "c"(c), "d"(d)
                : "memory"
                );
    return ret;
}

int isnum(char ch)
{
    return ch >= '0' && ch <= '9';
}

int isspc(char ch)
{
    return ch == ' ' || ch == '\n';
}

int dec2hex(unsigned int num, char *hex_str)
{
    char hex_rev[MAX_HEX_INTEGER_LEN];
    int i;

    if (num == 0) {
        hex_str[0] = '0';
        hex_str[1] = '\0';
        return 1;
    }

    for (i = 0; num != 0; num /= 16, i++) {
        int d = num % 16;
        hex_rev[i] = d < 10 ? '0' + d : 'a' + (d - 10);
    }

    for (int j = 0; j < i; j++) hex_str[j] = hex_rev[i - j - 1];
    hex_str[i] = '\0';

    return i;
}

void print_hex(unsigned num)
{
    char buf[MAX_HEX_INTEGER_LEN + 3];
    int len = 0;

    buf[len++] = '0';
    buf[len++] = 'x';
    len += dec2hex(num, buf + 2);
    buf[len++] = '\n';

    int ret = syscall4(STDOUT_FILENO, (int)buf, len);
    if (ret != len) syscall1(1);
}

void _start() 
{
    char buf[20];
    unsigned num = 0;
    int i;
    int num_digits = 0;
    unsigned chars_in_buffer = 0;

    for (;; i++, chars_in_buffer--) {
        if (chars_in_buffer == 0) {
            int ret = syscall3(STDIN_FILENO, (int)buf, sizeof(buf));
            if (ret < 0) syscall1(1);
            i = 0;
            chars_in_buffer = ret;
        }

        if (num_digits > 0 && (chars_in_buffer == 0 || !isnum(buf[i]))) {
            print_hex(num);
            num_digits = 0;
            num = 0;
        }
        
        if (chars_in_buffer == 0 || (!isspc(buf[i]) && !isnum(buf[i]))) {
            syscall1(0);
        }

        if (isnum(buf[i])) {
            num = num * 10 + buf[i] - '0';
            num_digits++;
        }
    }

    syscall1(0);
}
