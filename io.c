#include "io.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 2048
#define FIELD_MAX 1024

static void append_char(char *out, size_t outcap, size_t *pos, char c) {
    if (*pos + 1 >= outcap) return;
    out[*pos] = c;
    (*pos)++;
    out[*pos] = '\0';
}

static void unescape_field(const char *in, char *out, size_t outcap) {
    size_t pos = 0;
    out[0] = '\0';
    for (size_t i = 0; in[i] != '\0'; i++) {
        if (in[i] == '\\') {
            char n = in[i+1];
            if (n == 'n') { append_char(out, outcap, &pos, '\n'); i++; continue; }
            if (n == 'r') { append_char(out, outcap, &pos, '\r'); i++; continue; }
            if (n == 't') { append_char(out, outcap, &pos, '\t'); i++; continue; }
            if (n == '\\'){ append_char(out, outcap, &pos, '\\'); i++; continue; }
            if (n == '|') { append_char(out, outcap, &pos, '|'); i++; continue; }
            append_char(out, outcap, &pos, '\\');
            continue;
        }
        append_char(out, outcap, &pos, in[i]);
    }
}

static void escape_field(const char *in, char *out, size_t outcap) {
    size_t pos = 0;
    out[0] = '\0';
    for (size_t i = 0; in[i] != '\0'; i++) {
        char c = in[i];
        if (c == '\n') { append_char(out, outcap, &pos, '\\'); append_char(out, outcap, &pos, 'n'); continue; }
        if (c == '\r') { append_char(out, outcap, &pos, '\\'); append_char(out, outcap, &pos, 'r'); continue; }
        if (c == '\t') { append_char(out, outcap, &pos, '\\'); append_char(out, outcap, &pos, 't'); continue; }
        if (c == '\\') { append_char(out, outcap, &pos, '\\'); append_char(out, outcap, &pos, '\\'); continue; }
        if (c == '|')  { append_char(out, outcap, &pos, '\\'); append_char(out, outcap, &pos, '|'); continue; }
        append_char(out, outcap, &pos, c);
    }
}

static int split_6_fields(const char *line, char fields[6][FIELD_MAX]) {
    int f = 0;
    size_t pos = 0;
    for (int i = 0; i < 6; i++) fields[i][0] = '\0';

    for (size_t i = 0;; i++) {
        char c = line[i];
        if (c == '|' || c == '\0') {
            fields[f][pos] = '\0';
            f++;
            pos = 0;
            if (c == '\0') break;
            if (f > 6) break;
            continue;
        }
        if (f >= 6) return 0;
        if (pos + 1 < FIELD_MAX) fields[f][pos++] = c;
    }
    return (f == 6);
}

int load_from_file(const char *filename, Baza *b) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return 1;
    }

    char line[LINE_MAX];
    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        char raw[6][FIELD_MAX];
        if (!split_6_fields(line, raw)) {
            printf("Pominięto uszkodzony rekord (zły format pól): %s\n", line);
            continue;
        }

        Postac p;
        char tmp[FIELD_MAX];

        unescape_field(raw[0], p.pseudonim, sizeof(p.pseudonim));

        unescape_field(raw[1], tmp, sizeof(tmp));
        p.rola = atoi(tmp);

        unescape_field(raw[2], p.moc, sizeof(p.moc));

        unescape_field(raw[3], tmp, sizeof(tmp));
        p.poziom = atoi(tmp);

        unescape_field(raw[4], p.dzielnica, sizeof(p.dzielnica));

        unescape_field(raw[5], tmp, sizeof(tmp));
        p.status = atoi(tmp);

        if (p.pseudonim[0] == '\0' || (int)strlen(p.pseudonim) > PSEUDONYM_MAX) {
            printf("Pominięto rekord (zły pseudonim): %s\n", line);
            continue;
        }
        if (p.poziom < 0 || p.poziom > 10) {
            printf("Pominięto rekord (poziom poza 0..10): %s\n", line);
            continue;
        }

        if (baza_find_index_by_pseudonim(b, p.pseudonim) != -1) {
            printf("Pominięto rekord (duplikat pseudonimu): %s\n", p.pseudonim);
            continue;
        }

        if (!baza_push(b, &p)) {
            fclose(fp);
            printf("Błąd: brak pamięci podczas wczytywania.\n");
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

int save_to_file(const char *filename, const Baza *b) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;

    for (size_t i = 0; i < b->size; i++) {
        char e0[LINE_MAX], e2[LINE_MAX], e4[LINE_MAX];
        escape_field(b->arr[i].pseudonim, e0, sizeof(e0));
        escape_field(b->arr[i].moc, e2, sizeof(e2));
        escape_field(b->arr[i].dzielnica, e4, sizeof(e4));

        fprintf(fp, "%s|%d|%s|%d|%s|%d\n",
                e0, b->arr[i].rola, e2, b->arr[i].poziom, e4, b->arr[i].status);
    }

    fclose(fp);
    return 1;
}
