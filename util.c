#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

void safe_readline(const char *prompt, char *buf, size_t buflen) {
    if (prompt) printf("%s", prompt);
    if (!fgets(buf, (int)buflen, stdin)) {
        buf[0] = '\0';
        return;
    }
    trim_newline(buf);
}

int read_int_range(const char *prompt, int minv, int maxv) {
    char tmp[128];
    for (;;) {
        safe_readline(prompt, tmp, sizeof(tmp));
        if (tmp[0] == '\0') {
            printf("Wartość nie może być pusta.\n");
            continue;
        }
        char *end = NULL;
        long v = strtol(tmp, &end, 10);
        if (*end != '\0') {
            printf("To nie jest liczba.\n");
            continue;
        }
        if (v < minv || v > maxv) {
            printf("Wartość poza zakresem (%d..%d).\n", minv, maxv);
            continue;
        }
        return (int)v;
    }
}

int starts_with(const char *s, const char *prefix) {
    while (*prefix) {
        if (*s != *prefix) return 0;
        s++; prefix++;
    }
    return 1;
}
