#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

// ==========================================
// ENUMS E STRUCT
// ==========================================
enum class ShipType { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER };
enum class CellState { EMPTY, SHIP, HIT, MISS };
enum class GameState { MENU, NAME_INPUT, PLACEMENT, TRANSITION, PLAYING, GAME_OVER };

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
    Coordinate getStartPos() const { return startPos_; }
    const std::vector<Coordinate>& getCoordinates() const { return coordinates_; }
    int getHits() const { return hits_; }
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
            return 0;
        }

        if (state == CellState::SHIP) {
            state = CellState::HIT;
            for (auto& ship : ships_) {
                for (const auto& c : ship.getCoordinates()) {
                    if (c == coord) {
                        ship.hit();
                        hitShip = &ship;
                        if (ship.isSunk()) return 2;
                        return 1;
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
    int shipsSunkCount_;

public:
    Player(std::string name = "Player")
        : name_(name), shotsFired_(0), hitsLanded_(0), misses_(0), shipsSunkCount_(0) {}

    std::string getName() const { return name_; }
    Board& getBoard() { return board_; }
    const Board& getBoard() const { return board_; }

    void recordShot(bool hit, bool sunk) {
        shotsFired_++;
        if (hit) {
            hitsLanded_++;
            if (sunk) shipsSunkCount_++;
        } else {
            misses_++;
        }
    }

    void resetStats() {
        shotsFired_ = 0;
        hitsLanded_ = 0;
        misses_ = 0;
        shipsSunkCount_ = 0;
        board_.reset();
    }
};

// ==========================================
// GAMERULES
// ==========================================
class GameRules {
public:
    static std::vector<Ship> createStandardFleet() {
        return {
            Ship("Portaerei", 5, ShipType::CARRIER),
            Ship("Corazzata", 4, ShipType::BATTLESHIP),
            Ship("Incrociatore", 3, ShipType::CRUISER),
            Ship("Sottomarino", 3, ShipType::SUBMARINE),
            Ship("Cacciatorpediniere", 2, ShipType::DESTROYER)
        };
    }
};

// ==========================================
// CLASSE GAME
// ==========================================
class Game {
private:
    sf::RenderWindow window_;
    sf::Font font_;
    GameState state_;
    Player player1_;
    Player player2_;
    int activePlayer_;
    int currentShipIndex_;
    bool placementHorizontal_;
    std::string p1NameInput_;
    std::string p2NameInput_;
    int activeNameInput_;

    void handleEvents() {
        sf::Event event;
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window_.close();

            if (state_ == GameState::MENU) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x, my = event.mouseButton.y;
                    if (mx >= 362 && mx <= 662 && my >= 320 && my <= 380) state_ = GameState::NAME_INPUT;
                    if (mx >= 362 && mx <= 662 && my >= 420 && my <= 480) window_.close();
                }
            }
            else if (state_ == GameState::NAME_INPUT) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == '\r' || event.text.unicode == '\n') {
                        if (activeNameInput_ == 1) {
                            activeNameInput_ = 2;
                        } else {
                            if (!p1NameInput_.empty()) player1_ = Player(p1NameInput_);
                            if (!p2NameInput_.empty()) player2_ = Player(p2NameInput_);
                            player1_.getBoard().reset();
                            player2_.getBoard().reset();
                            for (auto& s : GameRules::createStandardFleet()) player1_.getBoard().addShip(s);
                            for (auto& s : GameRules::createStandardFleet()) player2_.getBoard().addShip(s);
                            activePlayer_ = 1;
                            currentShipIndex_ = 0;
                            state_ = GameState::PLACEMENT;
                        }
                    } else if (event.text.unicode == '\b') {
                        if (activeNameInput_ == 1 && !p1NameInput_.empty()) p1NameInput_.pop_back();
                        else if (activeNameInput_ == 2 && !p2NameInput_.empty()) p2NameInput_.pop_back();
                    } else if (event.text.unicode < 128 && event.text.unicode >= 32) {
                        if (activeNameInput_ == 1 && p1NameInput_.length() < 15) p1NameInput_ += static_cast<char>(event.text.unicode);
                        else if (activeNameInput_ == 2 && p2NameInput_.length() < 15) p2NameInput_ += static_cast<char>(event.text.unicode);
                    }
                }
            }
            else if (state_ == GameState::PLACEMENT) {
                Player& curPlayer = (activePlayer_ == 1) ? player1_ : player2_;
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) placementHorizontal_ = !placementHorizontal_;
                    if (event.key.code == sf::Keyboard::A) {
                        curPlayer.getBoard().autoPlaceShips();
                        currentShipIndex_ = curPlayer.getBoard().getShips().size();
                    }
                }
                if (event.type == sf::Event::MouseButtonPressed) {
                    int mx = event.mouseButton.x, my = event.mouseButton.y;
                    if (mx >= 100 && mx < 450 && my >= 150 && my < 500) {
                        int gridX = (mx - 100) / 35;
                        int gridY = (my - 150) / 35;
                        auto& ships = curPlayer.getBoard().getShips();
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            if (currentShipIndex_ < static_cast<int>(ships.size())) {
                                if (curPlayer.getBoard().placeShip(ships[currentShipIndex_], {gridX, gridY}, placementHorizontal_)) {
                                    currentShipIndex_++;
                                }
                            }
                        }
                    }
                    if (mx >= 700 && mx <= 920 && my >= 620 && my <= 680) {
                        if (currentShipIndex_ >= 5) {
                            if (activePlayer_ == 1) {
                                activePlayer_ = 2;
                                currentShipIndex_ = 0;
                                state_ = GameState::TRANSITION;
                            } else {
                                activePlayer_ = 1;
                                state_ = GameState::TRANSITION;
                            }
                        }
                    }
                }
            }
            else if (state_ == GameState::TRANSITION) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x, my = event.mouseButton.y;
                    if (mx >= 362 && mx <= 662 && my >= 450 && my <= 520) state_ = GameState::PLAYING;
                }
            }
            else if (state_ == GameState::PLAYING) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x, my = event.mouseButton.y;
                    if (mx >= 550 && mx < 900 && my >= 150 && my < 500) {
                        int gridX = (mx - 550) / 35;
                        int gridY = (my - 150) / 35;
                        Player& defender = (activePlayer_ == 1) ? player2_ : player1_;
                        Player& attacker = (activePlayer_ == 1) ? player1_ : player2_;
                        Ship* hitShip = nullptr;
                        int res = defender.getBoard().attackCell({gridX, gridY}, hitShip);
                        if (res != -1) {
                            attacker.recordShot(res > 0, res == 2);
                            if (defender.getBoard().allShipsSunk()) state_ = GameState::GAME_OVER;
                            else {
                                activePlayer_ = (activePlayer_ == 1) ? 2 : 1;
                                state_ = GameState::TRANSITION;
                            }
                        }
                    }
                }
            }
            else if (state_ == GameState::GAME_OVER) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    int mx = event.mouseButton.x, my = event.mouseButton.y;
                    if (mx >= 362 && mx <= 662 && my >= 520 && my <= 580) {
                        player1_.resetStats();
                        player2_.resetStats();
                        activePlayer_ = 1;
                        currentShipIndex_ = 0;
                        p1NameInput_.clear();
                        p2NameInput_.clear();
                        activeNameInput_ = 1;
                        state_ = GameState::MENU;
                    }
                }
            }
        }
    }

    void update() {}

    void render() {
        window_.clear(sf::Color(11, 25, 44));
        if (state_ == GameState::MENU) renderMenu();
        else if (state_ == GameState::NAME_INPUT) renderNameInput();
        else if (state_ == GameState::PLACEMENT) renderPlacement();
        else if (state_ == GameState::TRANSITION) renderTransition();
        else if (state_ == GameState::PLAYING) renderPlaying();
        else if (state_ == GameState::GAME_OVER) renderGameOver();
        window_.display();
    }

    void renderMenu() {
        sf::Text title("BATTAGLIA NAVALE", font_, 48);
        title.setFillColor(sf::Color::White);
        title.setPosition(310, 150);
        window_.draw(title);

        sf::RectangleShape btn1(sf::Vector2f(300, 60));
        btn1.setPosition(362, 320);
        btn1.setFillColor(sf::Color(30, 62, 98));
        window_.draw(btn1);
        sf::Text t1("NUOVA PARTITA", font_, 22);
        t1.setFillColor(sf::Color::White);
        t1.setPosition(430, 335);
        window_.draw(t1);

        sf::RectangleShape btn2(sf::Vector2f(300, 60));
        btn2.setPosition(362, 420);
        btn2.setFillColor(sf::Color(30, 62, 98));
        window_.draw(btn2);
        sf::Text t2("ESCI", font_, 22);
        t2.setFillColor(sf::Color::White);
        t2.setPosition(485, 435);
        window_.draw(t2);
    }

    void renderNameInput() {
        sf::Text title("INSERISCI I NOMI DEI GIOCATORI", font_, 32);
        title.setFillColor(sf::Color::White);
        title.setPosition(250, 150);
        window_.draw(title);

        sf::Text l1("Giocatore 1:", font_, 20);
        l1.setPosition(300, 280);
        window_.draw(l1);
        sf::RectangleShape box1(sf::Vector2f(300, 40));
        box1.setPosition(450, 275);
        box1.setFillColor(activeNameInput_ == 1 ? sf::Color(50, 90, 140) : sf::Color(30, 50, 80));
        window_.draw(box1);
        sf::Text t1(p1NameInput_, font_, 20);
        t1.setFillColor(sf::Color::White);
        t1.setPosition(460, 283);
        window_.draw(t1);

        sf::Text l2("Giocatore 2:", font_, 20);
        l2.setPosition(300, 360);
        window_.draw(l2);
        sf::RectangleShape box2(sf::Vector2f(300, 40));
        box2.setPosition(450, 355);
        box2.setFillColor(activeNameInput_ == 2 ? sf::Color(50, 90, 140) : sf::Color(30, 50, 80));
        window_.draw(box2);
        sf::Text t2(p2NameInput_, font_, 20);
        t2.setFillColor(sf::Color::White);
        t2.setPosition(460, 363);
        window_.draw(t2);

        sf::Text hint("Premi INVIO per procedere", font_, 16);
        hint.setFillColor(sf::Color(180, 180, 180));
        hint.setPosition(410, 460);
        window_.draw(hint);
    }

    void renderPlacement() {
        Player& curPlayer = (activePlayer_ == 1) ? player1_ : player2_;
        sf::Text header(curPlayer.getName() + " - Posiziona la Flotta", font_, 28);
        header.setFillColor(sf::Color::White);
        header.setPosition(100, 50);
        window_.draw(header);

        sf::Text info("Tasto 'R': Ruota nave | Tasto 'A': Posizionamento casuale", font_, 16);
        info.setFillColor(sf::Color(200, 200, 200));
        info.setPosition(100, 95);
        window_.draw(info);

        for (int y = 0; y < 10; ++y) {
            for (int x = 0; x < 10; ++x) {
                sf::RectangleShape cell(sf::Vector2f(33, 33));
                cell.setPosition(100 + x * 35, 150 + y * 35);
                CellState state = curPlayer.getBoard().getCellState(x, y);
                if (state == CellState::EMPTY) cell.setFillColor(sf::Color(30, 62, 98));
                else if (state == CellState::SHIP) cell.setFillColor(sf::Color(70, 130, 180));
                window_.draw(cell);
            }
        }

        auto& ships = curPlayer.getBoard().getShips();
        for (size_t i = 0; i < ships.size(); ++i) {
            sf::Text st(ships[i].getName() + " (" + std::to_string(ships[i].getSize()) + ")", font_, 18);
            st.setPosition(520, 150 + i * 40);
            st.setFillColor(i == currentShipIndex_ ? sf::Color::Yellow : (ships[i].isPlaced() ? sf::Color::Green : sf::Color::White));
            window_.draw(st);
        }

        if (currentShipIndex_ >= 5) {
            sf::RectangleShape btn(sf::Vector2f(220, 50));
            btn.setPosition(700, 620);
            btn.setFillColor(sf::Color(0, 150, 0));
            window_.draw(btn);
            sf::Text bt("CONFERMA", font_, 20);
            bt.setPosition(745, 633);
            bt.setFillColor(sf::Color::White);
            window_.draw(bt);
        }
    }

    void renderTransition() {
        Player& nextP = (activePlayer_ == 1) ? player1_ : player2_;
        sf::Text title("CAMBIO TURNO", font_, 36);
        title.setFillColor(sf::Color::White);
        title.setPosition(380, 200);
        window_.draw(title);

        sf::Text msg("Passa il computer a: " + nextP.getName(), font_, 24);
        msg.setFillColor(sf::Color::Yellow);
        msg.setPosition(340, 280);
        window_.draw(msg);

        sf::RectangleShape btn(sf::Vector2f(300, 60));
        btn.setPosition(362, 450);
        btn.setFillColor(sf::Color(30, 62, 98));
        window_.draw(btn);
        sf::Text bt("PRONTO", font_, 22);
        bt.setPosition(470, 465);
        bt.setFillColor(sf::Color::White);
        window_.draw(bt);
    }

    void renderPlaying() {
        Player& curPlayer = (activePlayer_ == 1) ? player1_ : player2_;
        Player& enemyPlayer = (activePlayer_ == 1) ? player2_ : player1_;

        sf::Text header("Turno di: " + curPlayer.getName(), font_, 24);
        header.setFillColor(sf::Color::White);
        header.setPosition(50, 30);
        window_.draw(header);

        sf::Text b1Title("La tua flotta", font_, 18);
        b1Title.setPosition(100, 120);
        window_.draw(b1Title);

        for (int y = 0; y < 10; ++y) {
            for (int x = 0; x < 10; ++x) {
                sf::RectangleShape cell(sf::Vector2f(33, 33));
                cell.setPosition(100 + x * 35, 150 + y * 35);
                CellState state = curPlayer.getBoard().getCellState(x, y);
                if (state == CellState::EMPTY) cell.setFillColor(sf::Color(30, 62, 98));
                else if (state == CellState::SHIP) cell.setFillColor(sf::Color(70, 130, 180));
                else if (state == CellState::HIT) cell.setFillColor(sf::Color::Red);
                else if (state == CellState::MISS) cell.setFillColor(sf::Color(200, 200, 200));
                window_.draw(cell);
            }
        }

        sf::Text b2Title("Griglia attacchi", font_, 18);
        b2Title.setPosition(550, 120);
        window_.draw(b2Title);

        for (int y = 0; y < 10; ++y) {
            for (int x = 0; x < 10; ++x) {
                sf::RectangleShape cell(sf::Vector2f(33, 33));
                cell.setPosition(550 + x * 35, 150 + y * 35);
                CellState state = enemyPlayer.getBoard().getCellState(x, y);
                if (state == CellState::SHIP || state == CellState::EMPTY) cell.setFillColor(sf::Color(30, 62, 98));
                else if (state == CellState::HIT) cell.setFillColor(sf::Color::Red);
                else if (state == CellState::MISS) cell.setFillColor(sf::Color(200, 200, 200));
                window_.draw(cell);
            }
        }
    }

    void renderGameOver() {
        Player& winner = (player1_.getBoard().allShipsSunk()) ? player2_ : player1_;
        sf::Text title("FINE PARTITA", font_, 42);
        title.setFillColor(sf::Color::White);
        title.setPosition(370, 180);
        window_.draw(title);

        sf::Text winText("Vincitore: " + winner.getName() + "!", font_, 28);
        winText.setFillColor(sf::Color::Yellow);
        winText.setPosition(370, 260);
        window_.draw(winText);

        sf::RectangleShape btn(sf::Vector2f(300, 60));
        btn.setPosition(362, 520);
        btn.setFillColor(sf::Color(30, 62, 98));
        window_.draw(btn);
        sf::Text bt("MENU PRINCIPALE", font_, 20);
        bt.setPosition(420, 535);
        bt.setFillColor(sf::Color::White);
        window_.draw(bt);
    }

public:
    Game()
        : window_(sf::VideoMode(1024, 768), "Battaglia Navale", sf::Style::Close),
          state_(GameState::MENU), player1_("Giocatore 1"), player2_("Giocatore 2"),
          activePlayer_(1), currentShipIndex_(0), placementHorizontal_(true), activeNameInput_(1) {
        window_.setFramerateLimit(60);
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        font_.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
    }

    void run() {
        while (window_.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }
};

// ==========================================
// MAIN
// ==========================================
int main() {
    Game game;
    game.run();
    return 0;
}
