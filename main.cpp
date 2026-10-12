#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

const int DIM = 10;

// Codici colore ANSI
const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string CYAN = "\033[36m";
const string BLUE = "\033[34m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string MAGENTA = "\033[35m";
const string GRAY = "\033[90m";

struct Nave {
    string nome;
    int lunghezza;
    int colpiSubiti = 0;
    vector<pair<int, int>> coordinate;
};

struct Giocatore {
    string nome;
    char grigliaPropria[DIM][DIM];
    char grigliaAttacchi[DIM][DIM];
    vector<Nave> flotta;
};

void pulisciSchermo() {
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#else
    system("clear");
#endif
}

void attendiConferma(const string &msg) {
    cout << "\n" << YELLOW << msg << RESET << "\nPremere Invio per proseguire...";
    cin.ignore(10000, '\n');
    cin.get();
}

void inizializzaGiocatore(Giocatore &g, const string &nome) {
    g.nome = nome;
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            g.grigliaPropria[i][j] = '~';
            g.grigliaAttacchi[i][j] = '~';
        }
    }
    g.flotta = {
        {"Cacciatorpediniere", 2, 0, {}},
        {"Sottomarino", 3, 0, {}},
        {"Incrociatore", 3, 0, {}},
        {"Corazzata", 4, 0, {}},
        {"Portaerei", 5, 0, {}}
    };
}

void stampaGriglia(const char griglia[DIM][DIM], bool nascondiNave) {
    cout << "\n ";
    for (char c = 'A'; c < 'A' + DIM; ++c) {
        cout << BOLD << CYAN << c << " " << RESET;
    }
    cout << "\n " << GRAY << "┌" << "────────────────────" << RESET << "\n";
    for (int i = 0; i < DIM; ++i) {
        if (i + 1 < 10) cout << " ";
        cout << BOLD << CYAN << i + 1 << RESET << GRAY << " | " << RESET;
        for (int j = 0; j < DIM; ++j) {
            char cella = griglia[i][j];
            if (cella == '~') {
                cout << BLUE << "~ " << RESET;
            } else if (cella == 'N') {
                if (nascondiNave) cout << BLUE << "~ " << RESET;
                else cout << GREEN << BOLD << "N " << RESET;
            } else if (cella == 'X') {
                cout << RED << BOLD << "X " << RESET;
            } else if (cella == '0') {
                cout << GRAY << "0 " << RESET;
            }
        }
        cout << GRAY << "|" << RESET << "\n";
    }
    cout << " " << GRAY << "└" << "────────────────────" << RESET << "\n";
}

void stampaLegenda() {
    cout << GRAY << "Legenda: "
         << BLUE << "~- " << GRAY << "=Acqua "
         << GREEN << "N " << GRAY << "=Nave "
         << RED << "X " << GRAY << "=Colpito "
         << GRAY << "0=Mancato" << RESET << "\n";
}

void stampaSchermata(const Giocatore &attuale) {
    pulisciSchermo();
    cout << BOLD << MAGENTA << "========================================" << RESET << "\n";
    cout << BOLD << MAGENTA << "            TURNO DI " << attuale.nome << RESET << "\n";
    cout << BOLD << MAGENTA << "========================================" << RESET << "\n";

    cout << "\n" << BOLD << "GRIGLIA DEGLI ATTACCHI (I tuoi tiri):" << RESET;
    stampaGriglia(attuale.grigliaAttacchi, false);

    cout << "\n" << BOLD << "LA TUA GRIGLIA (La tua flotta):" << RESET;
    stampaGriglia(attuale.grigliaPropria, false);

    stampaLegenda();
}

bool posizioneValida(Giocatore &g, int r, int c, int len, char dir) {
    for (int i = 0; i < len; ++i) {
        int nr = r + (dir == 'V' ? i : 0);
        int nc = c + (dir == '0' ? i : 0);

        if (nr < 0 || nr >= DIM || nc < 0 || nc >= DIM) return false;
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                int chkR = nr + dr;
                int chkC = nc + dc;
                if (chkR >= 0 && chkR < DIM && chkC >= 0 && chkC < DIM) {
                    if (g.grigliaPropria[chkR][chkC] == 'N') return false;
                }
            }
        }
    }
    return true;
}

void posizionaNavi(Giocatore &g) {
    for (auto &nave : g.flotta) {
        bool inserita = false;
        while (!inserita) {
            pulisciSchermo();
            cout << BOLD << MAGENTA << "=== FASE POSIZIONAMENTO: " << g.nome << " ===" << RESET << "\n";
            stampaGriglia(g.grigliaPropria, false);
            stampaLegenda();

            cout << "\nNave da posizionare: " << BOLD << GREEN << nave.nome << RESET
                 << " (Lunghezza: " << BOLD << nave.lunghezza << RESET << ")\n";
            cout << "Regola: Le navi non possono essere adiacenti nemmeno in diagonale.\n";
            cout << "Inserisci coordinate (es. " << BOLD << "A1 0" << RESET << " per Orizzontale o "
                 << BOLD << "B5 V" << RESET << " per Verticale): ";

            string coord;
            char dir;
            if (!(cin >> coord >> dir)) {
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (coord.length() < 2 || coord.length() > 3) continue;

            char colChar = toupper(coord[0]);
            int c = colChar - 'A';
            int r = -1;

            try {
                r = stoi(coord.substr(1)) - 1;
            } catch (...) {
                continue;
            }

            dir = toupper(dir);
            if (c < 0 || c >= DIM || r < 0 || r >= DIM || (dir != '0' && dir != 'V')) {
                cout << RED << "\nCoordinate o orientamento non validi!" << RESET << "\n";
                cin.ignore(10000, '\n');
                cin.get();
                continue;
            }

            if (posizioneValida(g, r, c, nave.lunghezza, dir)) {
                for (int i = 0; i < nave.lunghezza; ++i) {
                    int nr = r + (dir == 'V' ? i : 0);
                    int nc = c + (dir == '0' ? i : 0);
                    g.grigliaPropria[nr][nc] = 'N';
                    nave.coordinate.push_back({nr, nc});
                }
                inserita = true;
            } else {
                cout << RED << "\nPosizione non valida o troppo vicina un'altra nave!" << RESET << "\n";
                cin.ignore(10000, '\n');
                cin.get();
            }
        }
    }
    pulisciSchermo();
    cout << BOLD << MAGENTA << "=== FLOTTA COMPLETATA: " << g.nome << " ===" << RESET << "\n";
    stampaGriglia(g.grigliaPropria, false);
    attendiConferma("Tutte le navi sono state posizionate con successo!");
}

bool flottaDistrutta(const Giocatore &g) {
    for (const auto &nave : g.flotta) {
        if (nave.colpiSubiti < nave.lunghezza) return false;
    }
    return true;
}

bool eseguiAttacco(Giocatore &attaccante, Giocatore &difensore, int r, int c) {
    if (difensore.grigliaPropria[r][c] == 'N') {
        difensore.grigliaPropria[r][c] = 'X';
        attaccante.grigliaAttacchi[r][c] = 'X';
        cout << RED << BOLD << "\n>>> COLPITO! <<<" << RESET << "\n";

        for (auto &nave : difensore.flotta) {
            for (const auto &p : nave.coordinate) {
                if (p.first == r && p.second == c) {
                    nave.colpiSubiti++;
                    if (nave.colpiSubiti == nave.lunghezza) {
                        cout << RED << BOLD << ">>> COLPITO E AFFONDATO! Hai distrutto: " << nave.nome << "! <<<" << RESET << "\n";
                    }
                    return true;
                }
            }
        }
        return true;
    } else {
        difensore.grigliaPropria[r][c] = '0';
        attaccante.grigliaAttacchi[r][c] = '0';
        cout << BLUE << BOLD << "\n>>> ACQUA! <<<" << RESET << "\n";
        return false;
    }
}

int main() {
    srand(time(0));

    Giocatore g1, g2;
    inizializzaGiocatore(g1, "Giocatore 1");
    inizializzaGiocatore(g2, "Giocatore 2");

    pulisciSchermo();
    cout << BOLD << CYAN << "========================================" << "\n";
    cout << "      BATTAGLIA NAVALE (2P)\n";
    cout << "========================================" << RESET << "\n";

    attendiConferma("Giocatore 1, preparati a posizionare le tue navi.");
    posizionaNavi(g1);

    pulisciSchermo();
    attendiConferma("Passa il turno al Giocatore 2 per il posizionamento.");
    posizionaNavi(g2);

    bool turnoG1 = true;

    while (!flottaDistrutta(g1) && !flottaDistrutta(g2)) {
        Giocatore &attaccante = turnoG1 ? g1 : g2;
        Giocatore &difensore = turnoG1 ? g2 : g1;

        stampaSchermata(attaccante);

        int r = -1, c = -1;
        bool mossaValida = false;

        while (!mossaValida) {
            cout << "\nInserisci coordinata di attacco (es. " << BOLD << "A1" << RESET << " o " << BOLD << "F7" << RESET << "): ";
            string coord;
            if (!(cin >> coord)) {
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (coord.length() < 2 || coord.length() > 3) continue;

            char colChar = toupper(coord[0]);
            c = colChar - 'A';

            try {
                r = stoi(coord.substr(1)) - 1;
            } catch (...) {
                continue;
            }

            if (r < 0 || r >= DIM || c < 0 || c >= DIM) {
                cout << RED << "Coordinate fuori dalla griglia!" << RESET << "\n";
                continue;
            }

            if (attaccante.grigliaAttacchi[r][c] != '~') {
                cout << RED << "Hai gia' sparato in questa posizione!" << RESET << "\n";
                continue;
            }

            mossaValida = true;
        }

        bool colpito = eseguiAttacco(attaccante, difensore, r, c);

        if (flottaDistrutta(difensore)) {
            pulisciSchermo();
            cout << BOLD << GREEN << "========================================" << "\n";
            cout << "  COMPLIMENTI " << attaccante.nome << " !\n";
            cout << "  HAI DISTRUTTO LA FLOTTA NEMICA E VINTO!\n";
            cout << "========================================" << RESET << "\n";
            break;
        }

        if (colpito) {
            attendiConferma("Hai colpito il bersaglio! Mantieni il turno e spari ancora.");
        } else {
            attendiConferma("Colpo a vuoto. Passa il dispositivo all'altro giocatore.");
            turnoG1 = !turnoG1;
        }
    }

    return 0;
}
