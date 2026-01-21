#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void trim_newline(char *s);

void safe_readline(const char *prompt, char *buf, size_t buflen);

int read_int_range(const char *prompt, int minv, int maxv);

int starts_with(const char *s, const char *prefix);

#endif
