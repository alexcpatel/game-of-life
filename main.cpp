#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <functional>
#include <vector>

// Game constants
const int CELL_SIZE = 10;
const int GRID_WIDTH = 64;
const int GRID_HEIGHT = 48;
const sf::Time STEP_TIME = sf::seconds(0.1f);
const int NAV_BAR_HEIGHT = 40;

// UI constants
const int BUTTON_WIDTH = 80;
const int BUTTON_HEIGHT = 30;
const int BUTTON_MARGIN = 10;
const int BUTTON_TEXT_SIZE = 18;
const int HISTORY_LIMIT = 10000;

// Color constants
const sf::Color BUTTON_FILL_COLOR(100, 100, 100);
const sf::Color BUTTON_OUTLINE_COLOR = sf::Color::White;
const sf::Color BUTTON_TEXT_COLOR = sf::Color::White;
const sf::Color CELL_COLOR = sf::Color::White;
const sf::Color BACKGROUND_COLOR = sf::Color::Black;
const sf::Color NAV_BAR_COLOR(50, 50, 50);

// Add these to the UI constants section
const int PAUSE_BUTTON_SIZE = 30;
const int STEP_BUTTON_SIZE = 20;
const int PAUSE_BUTTON_Y_OFFSET = 5;
const int BUTTON_SPACING = 10;

class GameOfLife {
private:
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    std::deque<std::vector<std::vector<bool>>> history;
    bool isPaused;
    bool isMouseDown;

public:
    GameOfLife()
        : grid(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false)),
          nextGrid(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false)),
          isPaused(true),
          isMouseDown(false) {
        randomizeGrid();
    }

    void randomizeGrid() {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                grid[y][x] = (rand() % 4 == 0);  // 25% chance of being alive
            }
        }
    }

    void update() {
        if (!isPaused) {
            history.push_back(grid);
            if (history.size() > HISTORY_LIMIT) {
                history.pop_front();
            }
            for (int y = 0; y < GRID_HEIGHT; ++y) {
                for (int x = 0; x < GRID_WIDTH; ++x) {
                    int neighbors = countNeighbors(x, y);
                    if (grid[y][x]) {
                        nextGrid[y][x] = (neighbors == 2 || neighbors == 3);
                    } else {
                        nextGrid[y][x] = (neighbors == 3);
                    }
                }
            }
            std::swap(grid, nextGrid);
        }
    }

    int countNeighbors(int x, int y) {
        int count = 0;
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
                int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;
                count += grid[ny][nx];
            }
        }
        return count;
    }

    void draw(sf::RenderWindow& window) {
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                if (grid[y][x]) {
                    cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                    cell.setFillColor(CELL_COLOR);
                    window.draw(cell);
                }
            }
        }
    }

    void stepForward() {
        isPaused = true;
        history.push_back(grid);
        if (history.size() > HISTORY_LIMIT) {
            history.pop_front();
        }
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                int neighbors = countNeighbors(x, y);
                if (grid[y][x]) {
                    nextGrid[y][x] = (neighbors == 2 || neighbors == 3);
                } else {
                    nextGrid[y][x] = (neighbors == 3);
                }
            }
        }
        std::swap(grid, nextGrid);
    }

    void stepBackward() {
        if (!history.empty()) {
            grid = history.back();
            history.pop_back();
        }
    }

    void togglePause() { isPaused = !isPaused; }

    bool getPaused() const { return isPaused; }

    void setMouseDown(bool down) { isMouseDown = down; }

    void handleMouseInput(int x, int y, bool state) {
        if (isMouseDown && isPaused) {
            setCell(x, y, state);
        }
    }

    bool getCell(int x, int y) const {
        if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            return grid[y][x];
        }
        return false;
    }

    void setCell(int x, int y, bool state) {
        if (isPaused && x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
            grid[y][x] = state;
        }
    }
};

class Button {
private:
    sf::RectangleShape shape;
    sf::ConvexShape playIcon;
    sf::RectangleShape pauseIcon[2];
    std::function<void()> callback;
    bool isPaused;

public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, std::function<void()> cb)
        : callback(cb), isPaused(true) {
        shape.setPosition(position);
        shape.setSize(size);
        shape.setFillColor(BUTTON_FILL_COLOR);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(BUTTON_OUTLINE_COLOR);

        // Set up play icon (triangle)
        playIcon.setPointCount(3);
        playIcon.setPoint(0, sf::Vector2f(size.x * 0.3f, size.y * 0.2f));
        playIcon.setPoint(1, sf::Vector2f(size.x * 0.3f, size.y * 0.8f));
        playIcon.setPoint(2, sf::Vector2f(size.x * 0.8f, size.y * 0.5f));
        playIcon.setFillColor(BUTTON_TEXT_COLOR);
        playIcon.setPosition(position);

        // Set up pause icon (two vertical lines)
        for (int i = 0; i < 2; ++i) {
            pauseIcon[i].setSize(sf::Vector2f(size.x * 0.15f, size.y * 0.6f));
            pauseIcon[i].setFillColor(BUTTON_TEXT_COLOR);
            pauseIcon[i].setPosition(position.x + size.x * (0.3f + i * 0.25f),
                                     position.y + size.y * 0.2f);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        if (isPaused) {
            window.draw(playIcon);
        } else {
            window.draw(pauseIcon[0]);
            window.draw(pauseIcon[1]);
        }
    }

    bool isMouseOver(const sf::Vector2i& mousePos) const {
        return shape.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                static_cast<float>(mousePos.y));
    }

    void togglePausePlay() { isPaused = !isPaused; }

    void click() {
        if (callback) {
            callback();
        }
        togglePausePlay();
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    sf::RenderWindow window(
        sf::VideoMode(GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE + NAV_BAR_HEIGHT),
        "Conway's Game of Life");
    window.setFramerateLimit(60);

    GameOfLife game;
    sf::Clock clock;

    Button pausePlayButton(
        sf::Vector2f(
            (GRID_WIDTH * CELL_SIZE - PAUSE_BUTTON_SIZE - STEP_BUTTON_SIZE - BUTTON_SPACING) / 2,
            GRID_HEIGHT * CELL_SIZE + PAUSE_BUTTON_Y_OFFSET),
        sf::Vector2f(PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE), [&game]() { game.togglePause(); });

    Button stepForwardButton(
        sf::Vector2f((GRID_WIDTH * CELL_SIZE + PAUSE_BUTTON_SIZE + BUTTON_SPACING) / 2,
                     GRID_HEIGHT * CELL_SIZE + PAUSE_BUTTON_Y_OFFSET),
        sf::Vector2f(STEP_BUTTON_SIZE, STEP_BUTTON_SIZE), [&game]() { game.stepForward(); });

    bool drawMode = true;  // True for drawing, false for erasing

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    game.randomizeGrid();
                } else if (event.key.code == sf::Keyboard::P) {
                    game.togglePause();
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (pausePlayButton.isMouseOver(mousePos)) {
                        pausePlayButton.click();
                    } else if (stepForwardButton.isMouseOver(mousePos)) {
                        stepForwardButton.click();
                    } else {
                        int x = event.mouseButton.x / CELL_SIZE;
                        int y = event.mouseButton.y / CELL_SIZE;
                        drawMode = !game.getPaused() || !game.getCell(x, y);
                        game.setMouseDown(true);
                        game.handleMouseInput(x, y, drawMode);
                    }
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    game.setMouseDown(false);
                }
            } else if (event.type == sf::Event::MouseMoved) {
                int x = event.mouseMove.x / CELL_SIZE;
                int y = event.mouseMove.y / CELL_SIZE;
                game.handleMouseInput(x, y, drawMode);
            }
        }

        if (clock.getElapsedTime() >= STEP_TIME) {
            game.update();
            clock.restart();
        }

        window.clear(BACKGROUND_COLOR);
        game.draw(window);
        pausePlayButton.draw(window);
        stepForwardButton.draw(window);
        window.display();
    }

    return 0;
}
