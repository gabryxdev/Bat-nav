#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <cctype>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

// ==========================================
// CONFIGURAZIONE COLORI ANSI
// ==========================================
void enableAnsi() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

const std::string COLOR_RESET = "\033[0m";
const std::string COLOR_BLUE = "\033[34m";   // Acqua
const std::string COLOR_RED = "\033[31m";    // Colpo a segno
const std::string COLOR_WHITE = "\033[37m";  // Mancato
const std::string COLOR_GREEN = "\033[32m";  // Nave
const std::string COLOR_YELLOW = "\033[33m"; // Evidenziato / Info
const std::string COLOR_CYAN = "\033[36m";   // Intestazioni

// ==========================================
// STRUTTURE E ENUMS
// ==========================================
enum class ShipType { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER };
enum class CellState { EMPTY, SHIP, HIT, MISS };

struct Coordinate {
    int x;
    int y;
    bool operator==(const Coordinate& other) const { return x == other.x && y == other.y; }
};

// ==========================================
// CLASSE SHIP
// ==========================================
class Ship {
private:
    std::string name_;
    int size_;
    ShipType type_;
    bool horizontal_;
    Coordinate startPos_;
    std::vector<Coordinate> coordinates_;
    int hits_;

public:
    Ship(std::string name, int size, ShipType type)
        : name_(name), size_(size), type_(type), horizontal_(true), startPos_{-1, -1}, hits_(0) {}

    void place(Coordinate start, bool horizontal) {
        startPos_ = start;
        horizontal_ = horizontal;
        coordinates_.clear();
        for (int i = 0; i < size_; ++i) {
            if (horizontal_) coordinates_.push_back({start.x + i, start.y});
            else coordinates_.push_back({start.x, start.y + i});
        }
    }

    bool isSunk() const { return hits_ >= size_; }
    void hit() { if (hits_ < size_) hits_++; }
    
    std::string getName() const { return name_; }
    int getSize() const { return size_; }
    ShipType getType() const { return type_; }
    bool isHorizontal() const { return horizontal_; }
    const std::vector<Coordinate>& getCoordinates() const { return coordinates_; }
    bool isPlaced() const { return startPos_.x != -1 && startPos_.y != -1; }
};

// ==========================================
// CLASSE BOARD
// ==========================================
class Board {
private:
    static const int SIZE = 10;
    CellState grid_[SIZE][SIZE];
    std::vector<Ship> ships_;

public:
    Board() { reset(); }

    void reset() {
        for (int y = 0; y < SIZE; ++y)
            for (int x = 0; x < SIZE; ++x)
                grid_[y][x] = CellState::EMPTY;
        ships_.clear();
    }

    bool canPlaceShip(const Ship& ship, Coordinate start, bool horizontal) const {
        int size = ship.getSize();
        if (horizontal && start.x + size > SIZE) return false;
        if (!horizontal && start.y + size > SIZE) return false;
        if (start.x < 0 || start.y < 0) return false;

        for (int i = 0; i < size; ++i) {
            int x = horizontal ? start.x + i : start.x;
            int y = horizontal ? start.y : start.y + i;
            if (grid_[y][x] == CellState::SHIP) return false;
        }
        return true;
    }

    bool placeShip(Ship& ship, Coordinate start, bool horizontal) {
        if (!canPlaceShip(ship, start, horizontal)) return false;
        removeShip(ship.getType());

        ship.place(start, horizontal);
        for (const auto& coord : ship.getCoordinates()) {
            grid_[coord.y][coord.x] = CellState::SHIP;
        }
        ships_.push_back(ship);
        return true;
    }

    bool removeShip(ShipType type) {
        for (auto it = ships_.begin(); it != ships_.end(); ++it) {
            if (it->getType() == type) {
                if (it->isPlaced()) {
                    for (const auto& coord : it->getCoordinates()) {
                        grid_[coord.y][coord.x] = CellState::EMPTY;
                    }
                }
                ships_.erase(it);
                return true;
            }
        }
        return false;
    }

    int attackCell(Coordinate coord, Ship*& hitShip) {
        if (coord.x < 0 || coord.x >= SIZE || coord.y < 0 || coord.y >= SIZE) return -1;

        CellState& state = grid_[coord.y][coord.x];
        if (state == CellState::HIT || state == CellState::MISS) return -1;

        if (state == CellState::EMPTY) {
            state = CellState::MISS;
            return 0; // Mancato
        }

        if (state == CellState::SHIP) {
            state = CellState::HIT;
            for (auto& ship : ships_) {
                for (const auto& c : ship.getCoordinates()) {
                    if (c == coord) {
                        ship.hit();
                        hitShip = &ship;
                        if (ship.isSunk()) return 2; // Affondata
                        return 1; // Colpita
                    }
                }
            }
        }
        return -1;
    }

    bool allShipsSunk() const {
        if (ships_.empty()) return false;
        for (const auto& ship : ships_) {
            if (!ship.isSunk()) return false;
        }
        return true;
    }

    void autoPlaceShips() {
        for (int y = 0; y < SIZE; ++y)
            for (int x = 0; x < SIZE; ++x)
                grid_[y][x] = CellState::EMPTY;
        
        std::vector<Ship> currentShips = ships_;
        ships_.clear();

        for (auto& ship : currentShips) {
            bool placed = false;
            while (!placed) {
                Coordinate start = {std::rand() % SIZE, std::rand() % SIZE};
                bool horiz = (std::rand() % 2 == 0);
                if (canPlaceShip(ship, start, horiz)) {
                    ship.place(start, horiz);
                    for (const auto& coord : ship.getCoordinates()) {
                        grid_[coord.y][coord.x] = CellState::SHIP;
                    }
                    ships_.push_back(ship);
                    placed = true;
                }
            }
        }
    }

    CellState getCellState(int x, int y) const { return grid_[y][x]; }
    std::vector<Ship>& getShips() { return ships_; }
    const std::vector<Ship>& getShips() const { return ships_; }
    void addShip(Ship ship) { ships_.push_back(ship); }
};

// ==========================================
// CLASSE PLAYER
// ==========================================
class Player {
private:
    std::string name_;
    Board board_;
    int shotsFired_;
    int hitsLanded_;
    int misses_;

public:
    Player(std::string name = "Giocatore")
        : name_(name), shotsFired_(0), hitsLanded_(0), misses_(0) {}

    std::string getName() const { return name_; }
    Board& getBoard() { return board_; }
    const Board& getBoard() const { return board_; }

    int getShotsFired() const { return shotsFired_; }
    int getHitsLanded() const { return hitsLanded_; }
    int getMisses() const { return misses_; }

    void recordShot(bool hit) {
        shotsFired_++;
        if (hit) hitsLanded_++;
        else misses_++;
    }

    void resetStats() {
        shotsFired_ = 0;
        hitsLanded_ = 0;
        misses_ = 0;
        board_.reset();
    }
};

// ==========================================
// FUNZIONI DI SUPPORTO E INTERFACCIA
// ==========================================
std::vector<Ship> createStandardFleet() {
    return {
        Ship("Portaerei", 5, ShipType::CARRIER),
        Ship("Corazzata", 4, ShipType::BATTLESHIP),
        Ship("Incrociatore", 3, ShipType::CRUISER),
        Ship("Sottomarino", 3, ShipType::SUBMARINE),
        Ship("Cacciatorpediniere", 2, ShipType::DESTROYER)
    };
}

void printBoard(const Board& board, bool hideShips) {
    std::cout << "    A B C D E F G H I J\n";
    std::cout << "   ---------------------\n";
    for (int y = 0; y < 10; ++y) {
        std::cout << (y + 1 < 10 ? " " : "") << y + 1 << " | ";
        for (int x = 0; x < 10; ++x) {
            CellState state = board.getCellState(x, y);
            if (state == CellState::EMPTY) std::cout << COLOR_BLUE << "~ " << COLOR_RESET;
            else if (state == CellState::SHIP) {
                if (hideShips) std::cout << COLOR_BLUE << "~ " << COLOR_RESET;
                else std::cout << COLOR_GREEN << "S " << COLOR_RESET;
            }
            else if (state == CellState::HIT) std::cout << COLOR_RED << "X " << COLOR_RESET;
            else if (state == CellState::MISS) std::cout << COLOR_WHITE << "O " << COLOR_RESET;
        }
        std::cout << "|\n";
    }
    std::cout << "   ---------------------\n";
}

void setupPlayerFleet(Player& player) {
    for (auto& ship : createStandardFleet()) {
        player.getBoard().addShip(ship);
    }
    
    clearScreen();
    std::cout << COLOR_CYAN << "=== POSIZIONAMENTO FLOTTA: " << player.getName() << " ===" << COLOR_RESET << "\n\n";
    std::cout << "Vuoi posizionare le navi casualmente o manualmente? (c = casuale, m = manuale): ";
    char choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (std::tolower(choice) == 'c') {
        player.getBoard().autoPlaceShips();
        std::cout << COLOR_GREEN << "\nFlotta posizionata casualmente con successo!\n" << COLOR_RESET;
        std::cout << "Premi Invio per continuare...";
        std::cin.get();
        return;
    }

    auto& ships = player.getBoard().getShips();
    for (size_t i = 0; i < ships.size(); ++i) {
        bool placed = false;
        while (!placed) {
            clearScreen();
            std::cout << COLOR_CYAN << "Posizionamento di: " << ships[i].getName() << " (Lunghezza: " << ships[i].getSize() << ")" << COLOR_RESET << "\n\n";
            printBoard(player.getBoard(), false);
            
            char colChar;
            int row;
            char orient;
            std::cout << "\nInserisci coordinata iniziale (es. A 3): ";
            if (!(std::cin >> colChar >> row)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cout << "Orientamento (H = Orizzontale, V = Verticale): ";
            std::cin >> orient;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            int x = std::toupper(colChar) - 'A';
            int y = row - 1;
            bool horiz = (std::toupper(orient) == 'H');

            if (player.getBoard().placeShip(ships[i], {x, y}, horiz)) {
                placed = true;
            } else {
                std::cout << COLOR_RED << "\nPosizione non valida o fuori limiti! Premi Invio per riprovare..." << COLOR_RESET;
                std::cin.get();
            }
        }
    }
}

int main() {
    enableAnsi();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    clearScreen();
    std::cout << COLOR_YELLOW << "========================================\n";
    std::cout << "       BATTAGLIA NAVALE IN C++          \n";
    std::cout << "========================================\n\n" << COLOR_RESET;

    std::string name1, name2;
    std::cout << "Nome Giocatore 1: ";
    std::cin >> name1;
    std::cout << "Nome Giocatore 2: ";
    std::cin >> name2;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Player p1(name1.empty() ? "Giocatore 1" : name1);
    Player p2(name2.empty() ? "Giocatore 2" : name2);

    setupPlayerFleet(p1);

    clearScreen();
    std::cout << "Passa il computer a " << COLOR_YELLOW << p2.getName() << COLOR_RESET << ".\n\nPremi Invio quando sei pronto...";
    std::cin.get();

    setupPlayerFleet(p2);

    bool gameOver = false;
    Player* activeP = &p1;
    Player* enemyP = &p2;

    while (!gameOver) {
        clearScreen();
        std::cout << COLOR_CYAN << "Turno di: " << COLOR_YELLOW << activeP->getName() << COLOR_RESET << "\n\n";
        
        std::cout << "La tua flotta:\n";
        printBoard(activeP->getBoard(), false);
        
        std::cout << "\nGriglia attacchi:\n";
        printBoard(enemyP->getBoard(), true);

        char colChar;
        int row;
        std::cout << "\nInserisci coordinate colpo (es. B 4): ";
        if (!(std::cin >> colChar >> row)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        int x = std::toupper(colChar) - 'A';
        int y = row - 1;

        Ship* hitShip = nullptr;
        int result = enemyP->getBoard().attackCell({x, y}, hitShip);

        if (result == -1) {
            std::cout << COLOR_RED << "\nColpo non valido o casella gia colpita! Premi Invio per riprovare..." << COLOR_RESET;
            std::cin.get();
            continue;
        }

        activeP->recordShot(result > 0);

        std::cout << "\nRisultato: ";
        if (result == 0) {
            std::cout << COLOR_WHITE << "ACQUA!" << COLOR_RESET << "\n";
        } else if (result == 1) {
            std::cout << COLOR_RED << "COLPITO!" << COLOR_RESET << "\n";
        } else if (result == 2) {
            std::cout << COLOR_RED << "COLPITO E AFFONDATO (" << hitShip->getName() << ")!" << COLOR_RESET << "\n";
        }

        if (enemyP->getBoard().allShipsSunk()) {
            clearScreen();
            std::cout << COLOR_GREEN << "\n========================================\n";
            std::cout << "             FINE PARTITA               \n";
            std::cout << "========================================\n";
            std::cout << " VITTORIA! " << activeP->getName() << " ha distrutto la flotta nemica!\n\n";
            std::cout << "--- STATISTICHE FINALI ---\n";
            std::cout << activeP->getName() << " -> Colpi sparati: " << activeP->getShotsFired() 
                      << " | A segno: " << activeP->getHitsLanded() 
                      << " | Mancati: " << activeP->getMisses() << "\n";
            std::cout << enemyP->getName() << " -> Colpi sparati: " << enemyP->getShotsFired() 
                      << " | A segno: " << enemyP->getHitsLanded() 
                      << " | Mancati: " << enemyP->getMisses() << "\n";
            std::cout << "========================================\n" << COLOR_RESET;
            gameOver = true;
        } else {
            std::cout << "\nPremi Invio per passare il turno all'avversario...";
            std::cin.get();

            Player* temp = activeP;
            activeP = enemyP;
            enemyP = temp;
        }
    }

    return 0;
}
