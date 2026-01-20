#ifndef IO_H
#define IO_H

#include "baza.h"

int load_from_file(const char *filename, Baza *b);
int save_to_file(const char *filename, const Baza *b);

#endif
