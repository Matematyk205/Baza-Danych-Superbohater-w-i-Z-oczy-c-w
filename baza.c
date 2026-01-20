#include "baza.h"
#include <stdlib.h>
#include <string.h>

static int baza_reserve(Baza *b, size_t newcap) {
    if (newcap <= b->cap) return 1;
    Postac *tmp = (Postac*)realloc(b->arr, newcap * sizeof(Postac));
    if (!tmp) return 0;
    b->arr = tmp;
    b->cap = newcap;
    return 1;
}

void baza_init(Baza *b) {
    b->arr = NULL;
    b->size = 0;
    b->cap = 0;
}

void baza_free(Baza *b) {
    free(b->arr);
    b->arr = NULL;
    b->size = 0;
    b->cap = 0;
}

int baza_push(Baza *b, const Postac *p) {
    if (b->size == b->cap) {
        size_t next = (b->cap == 0) ? 8 : (b->cap * 2);
        if (!baza_reserve(b, next)) return 0;
    }
    b->arr[b->size++] = *p;
    return 1;
}

int baza_find_index_by_pseudonim(const Baza *b, const char *pseudonim) {
    for (size_t i = 0; i < b->size; i++) {
        if (strcmp(b->arr[i].pseudonim, pseudonim) == 0) return (int)i;
    }
    return -1;
}

int baza_remove_at(Baza *b, size_t idx) {
    if (idx >= b->size) return 0;
    for (size_t i = idx + 1; i < b->size; i++) {
        b->arr[i - 1] = b->arr[i];
    }
    b->size--;
    return 1;
}

static void remove_many_impl(Baza *b, int (*match)(const Postac*, void*), void *ctx,
                             size_t *removed, size_t *blocked) {
    size_t w = 0;
    size_t rem = 0;
    size_t blk = 0;

    for (size_t r = 0; r < b->size; r++) {
        Postac cur = b->arr[r];

        if (match(&cur, ctx)) {
            if (cur.status == STATUS_UWIEZIONY) {
                b->arr[w++] = cur;
                blk++;
            } else {
                rem++;
            }
        } else {
            b->arr[w++] = cur;
        }
    }

    b->size = w;
    if (removed) *removed = rem;
    if (blocked) *blocked = blk;
}

static int match_status(const Postac *p, void *ctx) {
    int st = *(int*)ctx;
    return p->status == st;
}

int baza_remove_many_by_status(Baza *b, int status, size_t *removed, size_t *blocked) {
    remove_many_impl(b, match_status, &status, removed, blocked);
    return 1;
}

typedef struct { int thr; } CtxPoziom;
static int match_poziom_lt(const Postac *p, void *ctx) {
    CtxPoziom *c = (CtxPoziom*)ctx;
    return p->poziom < c->thr;
}

int baza_remove_many_by_poziom_lt(Baza *b, int threshold, size_t *removed, size_t *blocked) {
    CtxPoziom ctx = { threshold };
    remove_many_impl(b, match_poziom_lt, &ctx, removed, blocked);
    return 1;
}

static int cmp_pseudonim(const void *a, const void *b) {
    const Postac *pa = (const Postac*)a;
    const Postac *pb = (const Postac*)b;
    return strcmp(pa->pseudonim, pb->pseudonim);
}

static int cmp_poziom_desc(const void *a, const void *b) {
    const Postac *pa = (const Postac*)a;
    const Postac *pb = (const Postac*)b;
    return pb->poziom - pa->poziom;
}

void baza_sort_by_pseudonim(Baza *b) {
    qsort(b->arr, b->size, sizeof(Postac), cmp_pseudonim);
}

void baza_sort_by_poziom_desc(Baza *b) {
    qsort(b->arr, b->size, sizeof(Postac), cmp_poziom_desc);
}

const char* rola_str(int r) {
    switch (r) {
        case ROLA_BOHATER: return "bohater";
        case ROLA_ZLOCZYNCA: return "złoczyńca";
        case ROLA_ANTYBOHATER: return "antybohater";
        case ROLA_NEUTRALNY: return "neutralny";
        default: return "???";
    }
}

const char* status_str(int s) {
    switch (s) {
        case STATUS_AKTYWNY: return "aktywny";
        case STATUS_USPIONY: return "uśpiony";
        case STATUS_W_TRAKCIE_MISJI: return "w trakcie misji";
        case STATUS_RANNY: return "ranny";
        case STATUS_UWIEZIONY: return "uwięziony";
        default: return "???";
    }
}
