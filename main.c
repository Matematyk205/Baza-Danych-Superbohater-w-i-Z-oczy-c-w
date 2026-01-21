#include "baza.h"
#include "io.h"
#include "ui.h"
#include <stdio.h>

static void usage(const char *prog) {
    printf("Użycie: %s <plik_bazy.txt>\n", prog);
    printf("Przykład: %s baza_postaci.txt\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const char *plik = argv[1];

    Baza b;
    baza_init(&b);

    if (!load_from_file(plik, &b)) {
        baza_free(&b);
        return 2;
    }

    ui_menu_loop(&b, plik);

    baza_free(&b);
    return 0;
}
