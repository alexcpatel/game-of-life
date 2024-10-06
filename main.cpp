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
const int STEP_ICON_SIZE = 12;

// Add these constants
const std::vector<float> SPEED_MULTIPLIERS = {1.0f, 2.0f, 4.0f, 8.0f};
const int SPEED_DISPLAY_SIZE = 30;
const int SPEED_DISPLAY_MARGIN = 10;

class GameOfLife {
private:
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> nextGrid;
    std::deque<std::vector<std::vector<bool>>> history;
    bool isPaused;
    bool isMouseDown;
    int speedMultiplierIndex;

public:
    GameOfLife()
        : grid(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false)),
          nextGrid(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false)),
          isPaused(true),
          isMouseDown(false),
          speedMultiplierIndex(0) {
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

    void clearGrid() {
        for (int y = 0; y < GRID_HEIGHT; ++y) {
            for (int x = 0; x < GRID_WIDTH; ++x) {
                grid[y][x] = false;
            }
        }
    }

    void toggleSpeedMultiplier() {
        speedMultiplierIndex = (speedMultiplierIndex + 1) % SPEED_MULTIPLIERS.size();
    }

    float getSpeedMultiplier() const { return SPEED_MULTIPLIERS[speedMultiplierIndex]; }

    int getSpeedMultiplierIndex() const { return speedMultiplierIndex; }
};

class Button {
public:
    enum class Type { PlayPause, StepForward, StepBackward };

private:
    sf::RectangleShape shape;
    sf::ConvexShape playIcon;
    sf::RectangleShape pauseIcon[2];
    sf::RectangleShape stepForwardIconLine;
    sf::ConvexShape stepForwardIconTriangle;
    sf::RectangleShape stepBackwardIconLine;
    sf::ConvexShape stepBackwardIconTriangle;
    std::function<void()> callback;
    bool isPaused;
    Type buttonType;

public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, std::function<void()> cb,
           Type type)
        : callback(cb), isPaused(true), buttonType(type) {
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

        // Set up step forward icon (|>)
        {
            // Vertical line
            stepForwardIconLine.setSize(sf::Vector2f(size.x * 0.15f, size.y * 0.6f));
            stepForwardIconLine.setFillColor(BUTTON_TEXT_COLOR);
            stepForwardIconLine.setPosition(position.x + size.x * 0.2f, position.y + size.y * 0.2f);

            // Triangle
            stepForwardIconTriangle.setPointCount(3);
            stepForwardIconTriangle.setPoint(0, sf::Vector2f(size.x * 0.45f, size.y * 0.2f));
            stepForwardIconTriangle.setPoint(1, sf::Vector2f(size.x * 0.45f, size.y * 0.8f));
            stepForwardIconTriangle.setPoint(2, sf::Vector2f(size.x * 0.8f, size.y * 0.5f));
            stepForwardIconTriangle.setFillColor(BUTTON_TEXT_COLOR);
            stepForwardIconTriangle.setPosition(position);
        }

        // Set up step backward icon (<|)
        {
            // Vertical line
            stepBackwardIconLine.setSize(sf::Vector2f(size.x * 0.15f, size.y * 0.6f));
            stepBackwardIconLine.setFillColor(BUTTON_TEXT_COLOR);
            stepBackwardIconLine.setPosition(position.x + size.x * 0.65f,
                                             position.y + size.y * 0.2f);

            // Triangle
            stepBackwardIconTriangle.setPointCount(3);
            stepBackwardIconTriangle.setPoint(0, sf::Vector2f(size.x * 0.55f, size.y * 0.2f));
            stepBackwardIconTriangle.setPoint(1, sf::Vector2f(size.x * 0.55f, size.y * 0.8f));
            stepBackwardIconTriangle.setPoint(2, sf::Vector2f(size.x * 0.2f, size.y * 0.5f));
            stepBackwardIconTriangle.setFillColor(BUTTON_TEXT_COLOR);
            stepBackwardIconTriangle.setPosition(position);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        switch (buttonType) {
            case Type::PlayPause:
                if (isPaused) {
                    window.draw(playIcon);
                } else {
                    window.draw(pauseIcon[0]);
                    window.draw(pauseIcon[1]);
                }
                break;
            case Type::StepForward:
                window.draw(stepForwardIconLine);
                window.draw(stepForwardIconTriangle);
                break;
            case Type::StepBackward:
                window.draw(stepBackwardIconLine);
                window.draw(stepBackwardIconTriangle);
                break;
            default:
                break;
        }
    }

    bool isMouseOver(const sf::Vector2i& mousePos) const {
        return shape.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                static_cast<float>(mousePos.y));
    }

    void click() {
        if (callback) {
            callback();
        }
    }

    void setPaused(bool paused) { isPaused = paused; }

    bool getPaused() const { return isPaused; }
};

// Add this new class after the Button class
class SpeedDisplay {
private:
    sf::RectangleShape shape;

public:
    SpeedDisplay(const sf::Vector2f& position, const sf::Vector2f& size) {
        shape.setPosition(position);
        shape.setSize(size);
    }

    void draw(sf::RenderWindow& window, int speedMultiplierIndex) {
        const int maxSpeed = SPEED_MULTIPLIERS.size();
        const float barWidth = shape.getSize().x * 0.8f;
        const float barHeight = 5.0f;
        const float barSpacing = barWidth / (maxSpeed * 2 - 1);
        const float startX = shape.getPosition().x + (shape.getSize().x - barWidth) / 2;
        const float startY = shape.getPosition().y + shape.getSize().y / 2 - barHeight / 2;

        for (int i = 0; i < maxSpeed; ++i) {
            sf::RectangleShape speedBar;
            speedBar.setSize(sf::Vector2f(barWidth / maxSpeed - barSpacing, barHeight));
            speedBar.setPosition(startX + i * (barWidth / maxSpeed), startY);

            if (i <= speedMultiplierIndex) {
                speedBar.setFillColor(BUTTON_TEXT_COLOR);
            } else {
                speedBar.setFillColor(sf::Color(100, 100, 100));  // Dimmed color for inactive bars
            }

            window.draw(speedBar);
        }
    }

    bool isMouseOver(const sf::Vector2i& mousePos) const {
        return shape.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                static_cast<float>(mousePos.y));
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
        sf::Vector2f((GRID_WIDTH * CELL_SIZE - PAUSE_BUTTON_SIZE) / 2,
                     GRID_HEIGHT * CELL_SIZE + PAUSE_BUTTON_Y_OFFSET),
        sf::Vector2f(PAUSE_BUTTON_SIZE, PAUSE_BUTTON_SIZE),
        [&game, &pausePlayButton]() {
            game.togglePause();
            pausePlayButton.setPaused(game.getPaused());
        },
        Button::Type::PlayPause);

    Button stepForwardButton(
        sf::Vector2f((GRID_WIDTH * CELL_SIZE + PAUSE_BUTTON_SIZE) / 2 + BUTTON_SPACING,
                     GRID_HEIGHT * CELL_SIZE + PAUSE_BUTTON_Y_OFFSET +
                         (PAUSE_BUTTON_SIZE - STEP_BUTTON_SIZE) / 2),
        sf::Vector2f(STEP_BUTTON_SIZE, STEP_BUTTON_SIZE), [&game]() { game.stepForward(); },
        Button::Type::StepForward);

    Button stepBackwardButton(
        sf::Vector2f(
            (GRID_WIDTH * CELL_SIZE - PAUSE_BUTTON_SIZE) / 2 - BUTTON_SPACING - STEP_BUTTON_SIZE,
            GRID_HEIGHT * CELL_SIZE + PAUSE_BUTTON_Y_OFFSET +
                (PAUSE_BUTTON_SIZE - STEP_BUTTON_SIZE) / 2),
        sf::Vector2f(STEP_BUTTON_SIZE, STEP_BUTTON_SIZE), [&game]() { game.stepBackward(); },
        Button::Type::StepBackward);

    SpeedDisplay speedDisplay(
        sf::Vector2f(GRID_WIDTH * CELL_SIZE - SPEED_DISPLAY_SIZE - SPEED_DISPLAY_MARGIN,
                     GRID_HEIGHT * CELL_SIZE + SPEED_DISPLAY_MARGIN),
        sf::Vector2f(SPEED_DISPLAY_SIZE, SPEED_DISPLAY_SIZE));

    bool drawMode = true;  // True for drawing, false for erasing

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::R) {
                    game.randomizeGrid();
                } else if (event.key.code == sf::Keyboard::Space) {
                    game.togglePause();
                    pausePlayButton.setPaused(game.getPaused());
                } else if (event.key.code == sf::Keyboard::Left) {
                    game.stepBackward();
                } else if (event.key.code == sf::Keyboard::Right) {
                    game.stepForward();
                } else if (event.key.code == sf::Keyboard::C) {
                    game.clearGrid();
                } else if (event.key.code == sf::Keyboard::S) {
                    game.toggleSpeedMultiplier();
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (pausePlayButton.isMouseOver(mousePos)) {
                        pausePlayButton.click();
                    } else if (stepForwardButton.isMouseOver(mousePos)) {
                        stepForwardButton.click();
                    } else if (stepBackwardButton.isMouseOver(mousePos)) {
                        stepBackwardButton.click();
                    } else if (speedDisplay.isMouseOver(mousePos)) {
                        game.toggleSpeedMultiplier();
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

        if (clock.getElapsedTime() >= STEP_TIME / game.getSpeedMultiplier()) {
            game.update();
            clock.restart();
        }

        window.clear(BACKGROUND_COLOR);
        game.draw(window);
        pausePlayButton.draw(window);
        stepForwardButton.draw(window);
        stepBackwardButton.draw(window);
        speedDisplay.draw(window, game.getSpeedMultiplierIndex());
        window.display();
    }

    return 0;
}