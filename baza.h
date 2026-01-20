#ifndef BAZA_H
#define BAZA_H

#include <stddef.h>

#define PSEUDONYM_MAX 100
#define DZIELNICA_MAX 100
#define MOC_MAX 256

typedef enum {
    ROLA_BOHATER = 0,
    ROLA_ZLOCZYNCA,
    ROLA_ANTYBOHATER,
    ROLA_NEUTRALNY
} Rola;

typedef enum {
    STATUS_AKTYWNY = 0,
    STATUS_USPIONY,
    STATUS_W_TRAKCIE_MISJI,
    STATUS_RANNY,
    STATUS_UWIEZIONY
} Status;

typedef struct {
    char pseudonim[PSEUDONYM_MAX + 1];
    int rola;
    char moc[MOC_MAX];
    int poziom;
    char dzielnica[DZIELNICA_MAX + 1];
    int status;
} Postac;

typedef struct {
    Postac *arr;
    size_t size;
    size_t cap;
} Baza;

void baza_init(Baza *b);
void baza_free(Baza *b);

int  baza_push(Baza *b, const Postac *p);
int  baza_find_index_by_pseudonim(const Baza *b, const char *pseudonim);
int  baza_remove_at(Baza *b, size_t idx);
int  baza_remove_many_by_status(Baza *b, int status, size_t *removed, size_t *blocked);
int  baza_remove_many_by_poziom_lt(Baza *b, int threshold, size_t *removed, size_t *blocked);

void baza_sort_by_pseudonim(Baza *b);
void baza_sort_by_poziom_desc(Baza *b);

const char* rola_str(int r);
const char* status_str(int s);

#endif
