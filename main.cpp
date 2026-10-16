#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cctype>
#include <iomanip>

using namespace std;

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

// ==========================================
// CONFIGURAZIONE COLORI ANSI
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
    cout << "\033[2J\033[1;1H";
}

const string COLORE_RESET = "\033[0m";
const string COLORE_BLU = "\033[34m";   // Acqua
const string COLORE_ROSSO = "\033[31m";    // Colpo a segno
const string COLORE_BIANCO = "\033[37m";  // Mancato
const string COLORE_VERDE = "\033[32m";  // Nave
const string COLORE_GIALLO = "\033[33m"; // Evidenziato / Info
const string COLORE_CIANO = "\033[36m";   // Intestazioni

// ==========================================
// STRUTTURE E ENUMS
// ==========================================
enum class TipoNave { PORTAEREI, CORAZZATA, INCROCIATORE, SOTTOMARINO, CACCIATORPEDINIERE };
enum class StatoCella { VUOTA, NAVE, COLPITO, MANCATO };

struct Coordinata {
    int x;
    int y;
    bool operator==(const Coordinata& altra) const { return x == altra.x && y == altra.y; }
};

// ==========================================
// CLASSE NAVE
// ==========================================
class Nave {
private:
    string nome_;
    int dimensione_;
    TipoNave tipo_;
    bool orizzontale_;
    Coordinata posIniziale_;
    vector<Coordinata> coordinate_;
    int colpiRicevuti_;

public:
    Nave(string nome, int dimensione, TipoNave tipo)
        : nome_(nome), dimensione_(dimensione), tipo_(tipo), orizzontale_(true), posIniziale_{-1, -1}, colpiRicevuti_(0) {}

    void posiziona(Coordinata inizio, bool orizzontale) {
        posIniziale_ = inizio;
        orizzontale_ = orizzontale;
        coordinate_.clear();
        for (int i = 0; i < dimensione_; ++i) {
            if (orizzontale_) coordinate_.push_back({inizio.x + i, inizio.y});
            else coordinate_.push_back({inizio.x, inizio.y + i});
        }
    }

    bool èAffondata() const { return colpiRicevuti_ >= dimensione_; }
    void colpisci() { if (colpiRicevuti_ < dimensione_) colpiRicevuti_++; }
    
    string getNome() const { return nome_; }
    int getDimensione() const { return dimensione_; }
    TipoNave getTipo() const { return tipo_; }
    bool èOrizzontale() const { return orizzontale_; }
    const vector<Coordinata>& getCoordinate() const { return coordinate_; }
    bool èPosizionata() const { return posIniziale_.x != -1 && posIniziale_.y != -1; }
};

// ==========================================
// CLASSE GRIGLIA
// ==========================================
class Griglia {
private:
    static const int DIMENSIONE = 10;
    StatoCella griglia_[DIMENSIONE][DIMENSIONE];
    vector<Nave> navi_;

public:
    Griglia() { azzera(); }

    void azzera() {
        for (int y = 0; y < DIMENSIONE; ++y)
            for (int x = 0; x < DIMENSIONE; ++x)
                griglia_[y][x] = StatoCella::VUOTA;
        navi_.clear();
    }

    bool puòPosizionare(const Nave& nave, Coordinata inizio, bool orizzontale) const {
        int dim = nave.getDimensione();
        if (orizzontale && inizio.x + dim > DIMENSIONE) return false;
        if (!orizzontale && inizio.y + dim > DIMENSIONE) return false;
        if (inizio.x < 0 || inizio.y < 0) return false;

        for (int i = 0; i < dim; ++i) {
            int x = orizzontale ? inizio.x + i : inizio.x;
            int y = orizzontale ? inizio.y : inizio.y + i;
            if (griglia_[y][x] == StatoCella::NAVE) return false;
        }
        return true;
    }

    bool posizionaNave(Nave& nave, Coordinata inizio, bool orizzontale) {
        if (!puòPosizionare(nave, inizio, orizzontale)) return false;
        rimuoviNave(nave.getTipo());

        nave.posiziona(inizio, orizzontale);
        for (const auto& coord : nave.getCoordinate()) {
            griglia_[coord.y][coord.x] = StatoCella::NAVE;
        }
        navi_.push_back(nave);
        return true;
    }

    bool rimuoviNave(TipoNave tipo) {
        for (auto it = navi_.begin(); it != navi_.end(); ++it) {
            if (it->getTipo() == tipo) {
                if (it->èPosizionata()) {
                    for (const auto& coord : it->getCoordinate()) {
                        griglia_[coord.y][coord.x] = StatoCella::VUOTA;
                    }
                }
                navi_.erase(it);
                return true;
            }
        }
        return false;
    }

    int attaccaCella(Coordinata coord, Nave*& naveColpita) {
        if (coord.x < 0 || coord.x >= DIMENSIONE || coord.y < 0 || coord.y >= DIMENSIONE) return -1;

        StatoCella& stato = griglia_[coord.y][coord.x];
        if (stato == StatoCella::COLPITO || stato == StatoCella::MANCATO) return -1;

        if (stato == StatoCella::VUOTA) {
            stato = StatoCella::MANCATO;
            return 0; // Mancato
        }

        if (stato == StatoCella::NAVE) {
            stato = StatoCella::COLPITO;
            for (auto& nave : navi_) {
                for (const auto& c : nave.getCoordinate()) {
                    if (c == coord) {
                        nave.colpisci();
                        naveColpita = &nave;
                        if (nave.èAffondata()) return 2; // Affondata
                        return 1; // Colpita
                    }
                }
            }
        }
        return -1;
    }

    bool tutteNaviAffondate() const {
        if (navi_.empty()) return false;
        for (const auto& nave : navi_) {
            if (!nave.èAffondata()) return false;
        }
        return true;
    }

    void posizionaNaviCasualmente() {
        for (int y = 0; y < DIMENSIONE; ++y)
            for (int x = 0; x < DIMENSIONE; ++x)
                griglia_[y][x] = StatoCella::VUOTA;
        
        vector<Nave> naviCorrenti = navi_;
        navi_.clear();

        for (auto& nave : naviCorrenti) {
            bool posizionata = false;
            while (!posizionata) {
                Coordinata inizio = {rand() % DIMENSIONE, rand() % DIMENSIONE};
                bool oriz = (rand() % 2 == 0);
                if (puòPosizionare(nave, inizio, oriz)) {
                    nave.posiziona(inizio, oriz);
                    for (const auto& coord : nave.getCoordinate()) {
                        griglia_[coord.y][coord.x] = StatoCella::NAVE;
                    }
                    navi_.push_back(nave);
                    posizionata = true;
                }
            }
        }
    }

    StatoCella getStatoCella(int x, int y) const { return griglia_[y][x]; }
    vector<Nave>& getNavi() { return navi_; }
    const vector<Nave>& getNavi() const { return navi_; }
    void aggiungiNave(Nave nave) { navi_.push_back(nave); }
};

// ==========================================
// CLASSE GIOCATORE
// ==========================================
class Giocatore {
private:
    string nome_;
    Griglia griglia_;
    int colpiSparati_;
    int colpiASegno_;
    int mancati_;

public:
    Giocatore(string nome = "Giocatore")
        : nome_(nome), colpiSparati_(0), colpiASegno_(0), mancati_(0) {}

    string getNome() const { return nome_; }
    Griglia& getGriglia() { return griglia_; }
    const Griglia& getGriglia() const { return griglia_; }

    int getColpiSparati() const { return colpiSparati_; }
    int getColpiASegno() const { return colpiASegno_; }
    int getMancati() const { return mancati_; }

    void registraColpo(bool successo) {
        colpiSparati_++;
        if (successo) colpiASegno_++;
        else mancati_++;
    }

    void azzeraStatistiche() {
        colpiSparati_ = 0;
        colpiASegno_ = 0;
        mancati_ = 0;
        griglia_.azzera();
    }
};

// ==========================================
// FUNZIONI DI SUPPORTO E INTERFACCIA
// ==========================================
vector<Nave> creaFlottaStandard() {
    return {
        Nave("Portaerei", 5, TipoNave::PORTAEREI),
        Nave("Corazzata", 4, TipoNave::CORAZZATA),
        Nave("Incrociatore", 3, TipoNave::INCROCIATORE),
        Nave("Sottomarino", 3, TipoNave::SOTTOMARINO),
        Nave("Cacciatorpediniere", 2, TipoNave::CACCIATORPEDINIERE)
    };
}

void stampaGriglia(const Griglia& griglia, bool nascondiNavi) {
    cout << "    A B C D E F G H I J\n";
    cout << "    ---------------------\n";
    for (int y = 0; y < 10; ++y) {
        cout << (y + 1 < 10 ? " " : "") << y + 1 << " | ";
        for (int x = 0; x < 10; ++x) {
            StatoCella stato = griglia.getStatoCella(x, y);
            if (stato == StatoCella::VUOTA) cout << COLORE_BLU << "~ " << COLORE_RESET;
            else if (stato == StatoCella::NAVE) {
                if (nascondiNavi) cout << COLORE_BLU << "~ " << COLORE_RESET;
                else cout << COLORE_VERDE << "S " << COLORE_RESET;
            }
            else if (stato == StatoCella::COLPITO) cout << COLORE_ROSSO << "X " << COLORE_RESET;
            else if (stato == StatoCella::MANCATO) cout << COLORE_BIANCO << "O " << COLORE_RESET;
        }
        cout << "|\n";
    }
    cout << "    ---------------------\n";
}

void configuraFlottaGiocatore(Giocatore& giocatore) {
    for (auto& nave : creaFlottaStandard()) {
        giocatore.getGriglia().aggiungiNave(nave);
    }
    
    pulisciSchermo();
    cout << COLORE_CIANO << "=== POSIZIONAMENTO FLOTTA: " << giocatore.getNome() << " ===" << COLORE_RESET << "\n\n";
    cout << "Vuoi posizionare le navi casualmente o manualmente? (c = casuale, m = manuale): ";
    char scelta;
    cin >> scelta;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (tolower(scelta) == 'c') {
        giocatore.getGriglia().posizionaNaviCasualmente();
        cout << COLORE_VERDE << "\nFlotta posizionata casualmente con successo!\n" << COLORE_RESET;
        cout << "Premi Invio per continuare...";
        cin.get();
        return;
    }

    auto& navi = giocatore.getGriglia().getNavi();
    for (size_t i = 0; i < navi.size(); ++i) {
        bool posizionata = false;
        while (!posizionata) {
            pulisciSchermo();
            cout << COLORE_CIANO << "Posizionamento di: " << navi[i].getNome() << " (Lunghezza: " << navi[i].getDimensione() << ")" << COLORE_RESET << "\n\n";
            stampaGriglia(giocatore.getGriglia(), false);
            
            char colonnaChar;
            int riga;
            char orientamento;
            cout << "\nInserisci coordinata iniziale (es. A 3): ";
            if (!(cin >> colonnaChar >> riga)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cout << "Orientamento (H = Orizzontale, V = Verticale): ";
            cin >> orientamento;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            int x = toupper(colonnaChar) - 'A';
            int y = riga - 1;
            bool oriz = (toupper(orientamento) == 'H');

            if (giocatore.getGriglia().posizionaNave(navi[i], {x, y}, oriz)) {
                posizionata = true;
            } else {
                cout << COLORE_ROSSO << "\nPosizione non valida o fuori limiti! Premi Invio per riprovare..." << COLORE_RESET;
                cin.get();
            }
        }
    }
}

int main() {
    abilitaAnsi();
    srand(static_cast<unsigned int>(time(nullptr)));

    pulisciSchermo();
    cout << COLORE_GIALLO << "========================================\n";
    cout << "        BATTAGLIA NAVALE IN C++         \n";
    cout << "========================================\n\n" << COLORE_RESET;

    string nome1, nome2;
    cout << "Nome Giocatore 1: ";
    cin >> nome1;
    cout << "Nome Giocatore 2: ";
    cin >> nome2;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Giocatore g1(nome1.empty() ? "Giocatore 1" : nome1);
    Giocatore g2(nome2.empty() ? "Giocatore 2" : nome2);

    configuraFlottaGiocatore(g1);

    pulisciSchermo();
    cout << "Passa il computer a " << COLORE_GIALLO << g2.getNome() << COLORE_RESET << ".\n\nPremi Invio quando sei pronto...";
    cin.get();

    configuraFlottaGiocatore(g2);

    bool partitaFinita = false;
    Giocatore* giocatoreAttivo = &g1;
    Giocatore* giocatoreAvversario = &g2;

    while (!partitaFinita) {
        pulisciSchermo();
        cout << COLORE_CIANO << "Turno di: " << COLORE_GIALLO << giocatoreAttivo->getNome() << COLORE_RESET << "\n\n";
        
        cout << "La tua flotta:\n";
        stampaGriglia(giocatoreAttivo->getGriglia(), false);
        
        cout << "\nGriglia attacchi:\n";
        stampaGriglia(giocatoreAvversario->getGriglia(), true);

        char colonnaChar;
        int riga;
        cout << "\nInserisci coordinate colpo (es. B 4): ";
        if (!(cin >> colonnaChar >> riga)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        int x = toupper(colonnaChar) - 'A';
        int y = riga - 1;

        Nave* naveColpita = nullptr;
        int risultato = giocatoreAvversario->getGriglia().attaccaCella({x, y}, naveColpita);

        if (risultato == -1) {
            cout << COLORE_ROSSO << "\nColpo non valido o casella gia colpita! Premi Invio per riprovare..." << COLORE_RESET;
            cin.get();
            continue;
        }

        giocatoreAttivo->registraColpo(risultato > 0);

        cout << "\nRisultato: ";
        if (risultato == 0) {
            cout << COLORE_BIANCO << "ACQUA!" << COLORE_RESET << "\n";
        } else if (risultato == 1) {
            cout << COLORE_ROSSO << "COLPITO!" << COLORE_RESET << "\n";
        } else if (risultato == 2) {
            cout << COLORE_ROSSO << "COLPITO E AFFONDATO (" << naveColpita->getNome() << ")!" << COLORE_RESET << "\n";
        }

        if (giocatoreAvversario->getGriglia().tutteNaviAffondate()) {
            pulisciSchermo();
            cout << COLORE_VERDE << "\n========================================\n";
            cout << "               FINE PARTITA             \n";
            cout << "========================================\n";
            cout << " VITTORIA! " << giocatoreAttivo->getNome() << " ha distrutto la flotta nemica!\n\n";
            cout << "--- STATISTICHE FINALI ---\n";
            cout << giocatoreAttivo->getNome() << " -> Colpi sparati: " << giocatoreAttivo->getColpiSparati() 
                      << " | A segno: " << giocatoreAttivo->getColpiASegno() 
                      << " | Mancati: " << giocatoreAttivo->getMancati() << "\n";
            cout << giocatoreAvversario->getNome() << " -> Colpi sparati: " << giocatoreAvversario->getColpiSparati() 
                      << " | A segno: " << giocatoreAvversario->getColpiASegno() 
                      << " | Mancati: " << giocatoreAvversario->getMancati() << "\n";
            cout << "========================================\n" << COLORE_RESET;
            partitaFinita = true;
        } else {
            cout << "\nPremi Invio per passare il turno all'avversario...";
            cin.get();

            Giocatore* temp = giocatoreAttivo;
            giocatoreAttivo = giocatoreAvversario;
            giocatoreAvversario = temp;
        }
    }

    return 0;
}
