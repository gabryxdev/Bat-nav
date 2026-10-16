#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

#define DIMENSIONE 10
#define NUMERO_NAVI 5

// ==========================================
// ENUMS E STRUTTURE
// ==========================================
typedef enum {
    STATO_VUOTA = 0,
    STATO_NAVE = 1,
    STATO_COLPITO = 2,
    STATO_MANCATO = 3
} StatoCella;

typedef struct {
    int x;
    int y;
} Coordinata;

typedef struct {
    char nome[30];
    int dimensione;
    int colpiRicevuti;
    int posizionata;
    int orizzontale;
    Coordinata posIniziale;
    Coordinata coordinate[5]; // Massima dimensione nave = 5
} Nave;

typedef struct {
    char nome[50];
    StatoCella griglia[DIMENSIONE][DIMENSIONE];
    Nave navi[NUMERO_NAVI];
    int colpiSparati;
    int colpiASegno;
    int mancati;
} Giocatore;

// ==========================================
// CONFIGURAZIONE COLORI E SCHERMO
// ==========================================
void abilitaAnsi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void pulisciSchermo() {
    printf("\033[2J\033[1;1H");
}

const char* COLORE_RESET = "\033[0m";
const char* COLORE_BLU = "\033[34m";   // Acqua
const char* COLORE_ROSSO = "\033[31m";    // Colpo a segno
const char* COLORE_BIANCO = "\033[37m";  // Mancato
const char* COLORE_VERDE = "\033[32m";  // Nave
const char* COLORE_GIALLO = "\033[33m"; // Info
const char* COLORE_CIANO = "\033[36m";   // Intestazioni

// ==========================================
// INIZIALIZZAZIONE FLOTTA
// ==========================================
void inizializzaFlotta(Nave navi[]) {
    strcpy(navi[0].nome, "Portaerei");
    navi[0].dimensione = 5;
    
    strcpy(navi[1].nome, "Corazzata");
    navi[1].dimensione = 4;
    
    strcpy(navi[2].nome, "Incrociatore");
    navi[2].dimensione = 3;
    
    strcpy(navi[3].nome, "Sottomarino");
    navi[3].dimensione = 3;
    
    strcpy(navi[4].nome, "Cacciatorpediniere");
    navi[4].dimensione = 2;

    for (int i = 0; i < NUMERO_NAVI; i++) {
        navi[i].colpiRicevuti = 0;
        navi[i].posizionata = 0;
        navi[i].orizzontale = 1;
        navi[i].posIniziale.x = -1;
        navi[i].posIniziale.y = -1;
    }
}

void inizializzaGiocatore(Giocatore* g, const char* nome) {
    strcpy(g->nome, nome);
    g->colpiSparati = 0;
    g->colpiASegno = 0;
    g->mancati = 0;

    for (int y = 0; y < DIMENSIONE; y++) {
        for (int x = 0; x < DIMENSIONE; x++) {
            g->griglia[y][x] = STATO_VUOTA;
        }
    }
    inizializzaFlotta(g->navi);
}

// ==========================================
// LOGICA GRIGLIA E NAVI
// ==========================================
int puoPosizionare(Giocatore* g, int indiceNave, Coordinata inizio, int orizzontale) {
    int dim = g->navi[indiceNave].dimensione;
    if (orizzontale && inizio.x + dim > DIMENSIONE) return 0;
    if (!orizzontale && inizio.y + dim > DIMENSIONE) return 0;
    if (inizio.x < 0 || inizio.y < 0) return 0;

    for (int i = 0; i < dim; i++) {
        int x = orizzontale ? inizio.x + i : inizio.x;
        int y = orizzontale ? inizio.y : inizio.y + i;
        if (g->griglia[y][x] == STATO_NAVE) return 0;
    }
    return 1;
}

void rimuoviNaveDallaGriglia(Giocatore* g, int indiceNave) {
    if (!g->navi[indiceNave].posizionata) return;
    for (int i = 0; i < g->navi[indiceNave].dimensione; i++) {
        Coordinata c = g->navi[indiceNave].coordinate[i];
        g->griglia[c.y][c.x] = STATO_VUOTA;
    }
    g->navi[indiceNave].posizionata = 0;
}

int posizionaNave(Giocatore* g, int indiceNave, Coordinata inizio, int orizzontale) {
    if (!puoPosizionare(g, indiceNave, inizio, orizzontale)) return 0;
    
    rimuoviNaveDallaGriglia(g, indiceNave);

    g->navi[indiceNave].posIniziale = inizio;
    g->navi[indiceNave].orizzontale = orizzontale;
    g->navi[indiceNave].posizionata = 1;

    for (int i = 0; i < g->navi[indiceNave].dimensione; i++) {
        int x = orizzontale ? inizio.x + i : inizio.x;
        int y = orizzontale ? inizio.y : inizio.y + i;
        g->navi[indiceNave].coordinate[i] = (Coordinata){x, y};
        g->griglia[y][x] = STATO_NAVE;
    }
    return 1;
}

void posizionaNaviCasualmente(Giocatore* g) {
    for (int y = 0; y < DIMENSIONE; y++) {
        for (int x = 0; x < DIMENSIONE; x++) {
            g->griglia[y][x] = STATO_VUOTA;
        }
    }
    for (int i = 0; i < NUMERO_NAVI; i++) {
        g->navi[i].posizionata = 0;
        g->navi[i].colpiRicevuti = 0;
    }

    for (int i = 0; i < NUMERO_NAVI; i++) {
        int posizionato = 0;
        while (!posizionato) {
            Coordinata inizio = {rand() % DIMENSIONE, rand() % DIMENSIONE};
            int oriz = rand() % 2;
            if (posizionaNave(g, i, inizio, oriz)) {
                posizionato = 1;
            }
        }
    }
}

// ==========================================
// INTERFACCIA E STAMPA
// ==========================================
void stampaGriglia(Giocatore* g, int nascondiNavi) {
    printf("    A B C D E F G H I J\n");
    printf("    ---------------------\n");
    for (int y = 0; y < DIMENSIONE; y++) {
        printf("%2d | ", y + 1);
        for (int x = 0; x < DIMENSIONE; x++) {
            StatoCella stato = g->griglia[y][x];
            if (stato == STATO_VUOTA) {
                printf("%s~ %s", COLORE_BLU, COLORE_RESET);
            } else if (stato == STATO_NAVE) {
                if (nascondiNavi) printf("%s~ %s", COLORE_BLU, COLORE_RESET);
                else printf("%sS %s", COLORE_VERDE, COLORE_RESET);
            } else if (stato == STATO_COLPITO) {
                printf("%sX %s", COLORE_ROSSO, COLORE_RESET);
            } else if (stato == STATO_MANCATO) {
                printf("%sO %s", COLORE_BIANCO, COLORE_RESET);
            }
        }
        printf("|\n");
    }
    printf("    ---------------------\n");
}

void configuraFlotta(Giocatore* g) {
    pulisciSchermo();
    printf("%s=== POSIZIONAMENTO FLOTTA: %s ===%s\n\n", COLORE_CIANO, g->nome, COLORE_RESET);
    printf("Vuoi posizionare le navi casualmente o manualmente? (c = casuale, m = manuale): ");
    char scelta;
    scanf(" %c", &scelta);
    while (getchar() != '\n');

    if (tolower(scelta) == 'c') {
        posizionaNaviCasualmente(g);
        printf("%s\nFlotta posizionata casualmente con successo!\n%s", COLORE_VERDE, COLORE_RESET);
        printf("Premi Invio per continuare...");
        getchar();
        return;
    }

    for (int i = 0; i < NUMERO_NAVI; i++) {
        int posizionata = 0;
        while (!posizionata) {
            pulisciSchermo();
            printf("%sPosizionamento di: %s (Lunghezza: %d)%s\n\n", COLORE_CIANO, g->navi[i].nome, g->navi[i].dimensione, COLORE_RESET);
            stampaGriglia(g, 0);

            char colonnaChar;
            int riga;
            char orientamento;
            printf("\nInserisci coordinata iniziale (es. A 3): ");
            if (scanf(" %c %d", &colonnaChar, &riga) != 2) {
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');

            printf("Orientamento (H = Orizzontale, V = Verticale): ");
            scanf(" %c", &orientamento);
            while (getchar() != '\n');

            int x = toupper(colonnaChar) - 'A';
            int y = riga - 1;
            int oriz = (toupper(orientamento) == 'H');

            if (posizionaNave(g, i, {x, y}, oriz)) {
                posizionata = 1;
            } else {
                printf("%s\nPosizione non valida o fuori limiti! Premi Invio per riprovare...%s", COLORE_ROSSO, COLORE_RESET);
                getchar();
            }
        }
    }
}

int attaccaCella(Giocatore* avversario, Coordinata coord, Nave** naveColpita) {
    if (coord.x < 0 || coord.x >= DIMENSIONE || coord.y < 0 || coord.y >= DIMENSIONE) return -1;

    StatoCella* stato = &avversario->griglia[coord.y][coord.x];
    if (*stato == STATO_COLPITO || *stato == STATO_MANCATO) return -1;

    if (*stato == STATO_VUOTA) {
        *stato = STATO_MANCATO;
        return 0; // Mancato
    }

    if (*stato == STATO_NAVE) {
        *stato = STATO_COLPITO;
        for (int i = 0; i < NUMERO_NAVI; i++) {
            Nave* n = &avversario->navi[i];
            for (int j = 0; j < n->dimensione; j++) {
                if (n->coordinate[j].x == coord.x && n->coordinate[j].y == coord.y) {
                    n->colpiRicevuti++;
                    *naveColpita = n;
                    if (n->colpiRicevuti >= n->dimensione) return 2; // Affondata
                    return 1; // Colpita
                }
            }
        }
    }
    return -1;
}

int tutteNaviAffondate(Giocatore* g) {
    for (int i = 0; i < NUMERO_NAVI; i++) {
        if (g->navi[i].colpiRicevuti < g->navi[i].dimensione) return 0;
    }
    return 1;
}

// ==========================================
// MAIN
// ==========================================
int main() {
    abilitaAnsi();
    srand((unsigned int)time(NULL));

    pulisciSchermo();
    printf("%s========================================\n", COLORE_GIALLO);
    printf("        BATTAGLIA NAVALE IN C          \n");
    printf("========================================\n\n%s", COLORE_RESET);

    char nome1[50], nome2[50];
    printf("Nome Giocatore 1: ");
    scanf("%49s", nome1);
    printf("Nome Giocatore 2: ");
    scanf("%49s", nome2);
    while (getchar() != '\n');

    Giocatore g1, g2;
    inizializzaGiocatore(&g1, nome1[0] ? nome1 : "Giocatore 1");
    inizializzaGiocatore(&g2, nome2[0] ? nome2 : "Giocatore 2");

    configuraFlotta(&g1);

    pulisciSchermo();
    printf("Passa il computer a %s%s%s.\n\nPremi Invio quando sei pronto...", COLORE_GIALLO, g2.nome, COLORE_RESET);
    getchar();

    configuraFlotta(&g2);

    int partitaFinita = 0;
    Giocatore* attivo = &g1;
    Giocatore* avversario = &g2;

    while (!partitaFinita) {
        pulisciSchermo();
        printf("%sTurno di: %s%s%s\n\n", COLORE_CIANO, COLORE_GIALLO, attivo->nome, COLORE_RESET);
        
        printf("La tua flotta:\n");
        stampaGriglia(attivo, 0);
        
        printf("\nGriglia attacchi:\n");
        stampaGriglia(avversario, 1);

        char colonnaChar;
        int riga;
        printf("\nInserisci coordinate colpo (es. B 4): ");
        if (scanf(" %c %d", &colonnaChar, &riga) != 2) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        int x = toupper(colonnaChar) - 'A';
        int y = riga - 1;

        Nave* naveColpita = NULL;
        int risultato = attaccaCella(avversario, (Coordinata){x, y}, &naveColpita);

        if (risultato == -1) {
            printf("%s\nColpo non valido o casella gia colpita! Premi Invio per riprovare...%s", COLORE_ROSSO, COLORE_RESET);
            getchar();
            continue;
        }

        attivo->colpiSparati++;
        if (risultato > 0) attivo->colpiASegno++;
        else attivo->mancati++;

        printf("\nRisultato: ");
        if (risultato == 0) {
            printf("%sACQUA!%s\n", COLORE_BIANCO, COLORE_RESET);
        } else if (risultato == 1) {
            printf("%sCOLPITO!%s\n", COLORE_ROSSO, COLORE_RESET);
        } else if (risultato == 2) {
            printf("%sCOLPITO E AFFONDATO (%s)!%s\n", COLORE_ROSSO, naveColpita->nome, COLORE_RESET);
        }

        if (tutteNaviAffondate(avversario)) {
            pulisciSchermo();
            printf("%s\n========================================\n", COLORE_VERDE);
            printf("               FINE PARTITA             \n");
            printf("========================================\n");
            printf(" VITTORIA! %s ha distrutto la flotta nemica!\n\n", attivo->nome);
            printf("--- STATISTICHE FINALI ---\n");
            printf("%s -> Colpi sparati: %d | A segno: %d | Mancati: %d\n", 
                   attivo->nome, attivo->colpiSparati, attivo->colpiASegno, attivo->mancati);
            printf("%s -> Colpi sparati: %d | A segno: %d | Mancati: %d\n", 
                   avversario->nome, avversario->colpiSparati, avversario->colpiASegno, avversario->mancati);
            printf("========================================\n%s", COLORE_RESET);
            partitaFinita = 1;
        } else {
            printf("\nPremi Invio per passare il turno all'avversario...");
            getchar();

            Giocatore* temp = attivo;
            attivo = avversario;
            avversario = temp;
        }
    }

    return 0;
}
