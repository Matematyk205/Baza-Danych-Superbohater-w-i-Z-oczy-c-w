#include "ui.h"
#include "io.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

static void print_postac_line(size_t i, const Postac *p) {
    printf("%zu) %s | rola=%s | poziom=%d | dzielnica=%s | status=%s\n",
           i+1, p->pseudonim, rola_str(p->rola), p->poziom, p->dzielnica, status_str(p->status));
    if (p->moc[0]) printf("    moc: %s\n", p->moc);
}

static void listuj(const Baza *b) {
    printf("\n=== LISTA (%zu) ===\n", b->size);
    for (size_t i = 0; i < b->size; i++) {
        print_postac_line(i, &b->arr[i]);
    }
}

static void dodaj(Baza *b) {
    Postac p;
    char buf[512];

    for (;;) {
        safe_readline("Pseudonim (unikalny, max 100): ", buf, sizeof(buf));
        if (buf[0] == '\0') { printf("Pseudonim nie może być pusty.\n"); continue; }
        if ((int)strlen(buf) > PSEUDONYM_MAX) { printf("Za długi pseudonim.\n"); continue; }
        if (baza_find_index_by_pseudonim(b, buf) != -1) { printf("Taki pseudonim już istnieje.\n"); continue; }
        strncpy(p.pseudonim, buf, sizeof(p.pseudonim));
        p.pseudonim[PSEUDONYM_MAX] = '\0';
        break;
    }

    printf("Rola: 0=bohater, 1=złoczyńca, 2=antybohater, 3=neutralny\n");
    p.rola = read_int_range("Rola: ", 0, 3);

    safe_readline("Moc (opis): ", buf, sizeof(buf));
    strncpy(p.moc, buf, sizeof(p.moc)-1);
    p.moc[sizeof(p.moc)-1] = '\0';

    p.poziom = read_int_range("Poziom zagrożenia (0..10): ", 0, 10);

    safe_readline("Dzielnica działania: ", buf, sizeof(buf));
    strncpy(p.dzielnica, buf, sizeof(p.dzielnica)-1);
    p.dzielnica[sizeof(p.dzielnica)-1] = '\0';

    printf("Status: 0=aktywny, 1=uśpiony, 2=w trakcie misji, 3=ranny, 4=uwięziony\n");
    p.status = read_int_range("Status: ", 0, 4);

    if (!baza_push(b, &p)) {
        printf("Błąd: brak pamięci.\n");
        return;
    }
    printf("Dodano: %s\n", p.pseudonim);
}

static void edytuj(Baza *b) {
    char key[256];
    safe_readline("Podaj pseudonim do edycji: ", key, sizeof(key));
    int idx = baza_find_index_by_pseudonim(b, key);
    if (idx < 0) { printf("Nie znaleziono.\n"); return; }

    Postac *p = &b->arr[idx];
    char buf[512];

    printf("Edytujesz: %s (pseudonim NIEZMIENIALNY)\n", p->pseudonim);

    printf("Rola: 0=bohater, 1=złoczyńca, 2=antybohater, 3=neutralny (aktualnie: %s)\n", rola_str(p->rola));
    p->rola = read_int_range("Nowa rola: ", 0, 3);

    safe_readline("Nowa moc (ENTER=bez zmian): ", buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(p->moc, buf, sizeof(p->moc)-1);
        p->moc[sizeof(p->moc)-1] = '\0';
    }

    p->poziom = read_int_range("Nowy poziom (0..10): ", 0, 10);

    safe_readline("Nowa dzielnica (ENTER=bez zmian): ", buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(p->dzielnica, buf, sizeof(p->dzielnica)-1);
        p->dzielnica[sizeof(p->dzielnica)-1] = '\0';
    }

    printf("Status: 0=aktywny, 1=uśpiony, 2=w trakcie misji, 3=ranny, 4=uwięziony (aktualnie: %s)\n",
           status_str(p->status));
    p->status = read_int_range("Nowy status: ", 0, 4);

    printf("Zapisano zmiany.\n");   
}

static void usun_jeden(Baza *b) {
    char key[256];
    safe_readline("Podaj pseudonim do usunięcia: ", key, sizeof(key));
    int idx = baza_find_index_by_pseudonim(b, key);
    if (idx < 0) { printf("Nie znaleziono.\n"); return; }

    if (b->arr[idx].status == STATUS_UWIEZIONY) {
        printf("NIE MOŻNA usunąć: status = uwięziony.\n");
        return;
    }

    baza_remove_at(b, (size_t)idx);
    printf("Usunięto.\n");
}

static void usun_wiele(Baza *b) {
    printf("Usuń wiele:\n");
    printf("1) usuń wszystkie o statusie X (z blokadą UWIEZIONY)\n");
    printf("2) usuń wszystkie o poziomie < N (z blokadą UWIEZIONY)\n");

    int opt = read_int_range("Wybór: ", 1, 2);
    size_t removed = 0, blocked = 0;

    if (opt == 1) {
        printf("Status: 0=aktywny, 1=uśpiony, 2=w trakcie misji, 3=ranny, 4=uwięziony\n");
        int st = read_int_range("Status X: ", 0, 4);
        baza_remove_many_by_status(b, st, &removed, &blocked);
    } else {
        int thr = read_int_range("N (0..10): ", 0, 10);
        baza_remove_many_by_poziom_lt(b, thr, &removed, &blocked);
    }

    printf("Usunięto: %zu | Zablokowano (uwięziony): %zu\n", removed, blocked);
}

static void sortuj(Baza *b) {
    printf("Sort:\n");
    printf("1) pseudonim A-Z\n");
    printf("2) poziom malejąco\n");
    int opt = read_int_range("Wybór: ", 1, 2);

    if (opt == 1) baza_sort_by_pseudonim(b);
    else baza_sort_by_poziom_desc(b);

    printf("Posortowano.\n");
}

static void szukaj(const Baza *b) {
    printf("Wyszukiwanie:\n");
    printf("Tekst:\n");
    printf("1) pseudonim (pełne)\n");
    printf("2) pseudonim (prefiks)\n");
    printf("3) dzielnica (pełne)\n");
    printf("4) dzielnica (prefiks)\n");
    int t = read_int_range("Wybór tekstu: ", 1, 4);

    char text[256];
    safe_readline("Wartość tekstowa: ", text, sizeof(text));
    if (text[0] == '\0') { printf("Tekst nie może być pusty.\n"); return; }

    printf("Kryterium liczbowe (poziom):\n");
    printf("1) poziom == X\n");
    printf("2) poziom >= X\n");
    int nopt = read_int_range("Wybór: ", 1, 2);
    int x = read_int_range("X (0..10): ", 0, 10);

    printf("\n=== WYNIKI ===\n");
    size_t found = 0;

    for (size_t i = 0; i < b->size; i++) {
        const Postac *p = &b->arr[i];

        int text_ok = 0;
        if (t == 1) text_ok = (strcmp(p->pseudonim, text) == 0);
        if (t == 2) text_ok = starts_with(p->pseudonim, text);
        if (t == 3) text_ok = (strcmp(p->dzielnica, text) == 0);
        if (t == 4) text_ok = starts_with(p->dzielnica, text);

        if (!text_ok) continue;

        int num_ok = 0;
        if (nopt == 1) num_ok = (p->poziom == x);
        else num_ok = (p->poziom >= x);

        if (!num_ok) continue;

        print_postac_line(i, p);
        found++;
    }

    if (found == 0) printf("Brak wyników.\n");
    else printf("Znaleziono: %zu\n", found);
}

void ui_menu_loop(Baza *b, const char *plik) {
    for (;;) {
        printf("\n=== MENU ===\n");
        printf("Plik: %s\n", plik);
        printf("1) Lista\n");
        printf("2) Dodaj\n");
        printf("3) Edytuj\n");
        printf("4) Usuń jeden\n");
        printf("5) Usuń wiele (kryterium)\n");
        printf("6) Szukaj / filtruj (tekst+liczba)\n");
        printf("7) Sortuj\n");
        printf("8) Zapisz\n");
        printf("0) Wyjście\n");

        int c = read_int_range("Wybór: ", 0, 8);
        switch (c) {
            case 1: listuj(b); break;
            case 2: dodaj(b); break;
            case 3: edytuj(b); break;
            case 4: usun_jeden(b); break;
            case 5: usun_wiele(b); break;
            case 6: szukaj(b); break;
            case 7: sortuj(b); break;
            case 8:
                if (!save_to_file(plik, b)) printf("Błąd zapisu do pliku.\n");
                else printf("Zapisano do: %s\n", plik);
                break;
            case 0:
                if (!save_to_file(plik, b)) {
                    printf("Uwaga: nie udało się zapisać przy wyjściu.\n");
                }
                return;
        }
    }
}
