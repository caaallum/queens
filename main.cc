#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <random>
#include <queue>
#include <unordered_set>
#include <functional>

/*
    ==========================
    Level structure
    ==========================
*/

struct Level {
    int rows;
    int cols;
    std::vector<int> regions;
};

/*
    ==========================
    Level Generator
    ==========================
*/

Level generateLevel(int rows, int cols, unsigned seed = std::random_device{}()) {
    std::mt19937 rng(seed);

    Level level;
    level.rows = rows;
    level.cols = cols;
    level.regions.resize(rows * cols, -1);

    std::vector<sf::Vector2i> queens;
    std::vector<int> columns(cols, 0);

    std::function<bool(int)> placeRow = [&](int row) -> bool {
        if (row == rows)
            return true;

        std::vector<int> colOrder(cols);
        for (int i = 0; i < cols; ++i) colOrder[i] = i;
        std::shuffle(colOrder.begin(), colOrder.end(), rng);

        for (int i = 0; i < cols; ++i)
            printf("%d ", colOrder[i]);

        for (int col : colOrder) {
            if (columns[col])
                continue;

            bool adjacent = false;
            for (auto& q : queens) {
                if (std::abs(q.x - row) <= 1 &&
                    std::abs(q.y - col) <= 1) {
                    adjacent = true;
                    break;
                }
            }
            if (adjacent)
                continue;

            columns[col] = 1;
            queens.push_back({ row, col });

            if (placeRow(row + 1))
                return true;

            queens.pop_back();
            columns[col] = 0;
        }
        return false;
    };

    placeRow(0);

    // One region per queen
    for (int i = 0; i < (int)queens.size(); ++i) {
        auto& q = queens[i];
        level.regions[q.x * cols + q.y] = i;
    }

    // Flood fill
    std::queue<int> q;
    for (int i = 0; i < rows * cols; ++i)
        if (level.regions[i] != -1)
            q.push(i);

    int d[5] = { -1, 0, 1, 0, -1 };

    while (!q.empty()) {
        int idx = q.front();
        q.pop();
        int r = idx / cols;
        int c = idx % cols;

        for (int i = 0; i < 4; ++i) {
            int nr = r + d[i];
            int nc = c + d[i + 1];
            if (nr < 0 || nc < 0 || nr >= rows || nc >= cols)
                continue;

            int nidx = nr * cols + nc;
            if (level.regions[nidx] == -1) {
                level.regions[nidx] = level.regions[idx];
                q.push(nidx);
            }
        }
    }

    return level;
}

/*
    ==========================
    QueensGrid
    ==========================
*/

class QueensGrid : public sf::Drawable, public sf::Transformable {
public:
    QueensGrid(const Level& level, float cellSize)
        : m_cellSize(cellSize) {
        reset(level);
    }

    void reset(const Level& level) {
        m_rows = level.rows;
        m_cols = level.cols;
        m_cells.clear();
        m_cells.resize(m_rows * m_cols);

        for (int i = 0; i < m_rows * m_cols; ++i) {
            m_cells[i].region = level.regions[i];
            m_cells[i].color = regionColor(level.regions[i]);
            m_cells[i].state = CellState::Empty;
        }
    }

    void handleEvent(const std::optional<sf::Event>& event,
        const sf::RenderWindow& window) {
        if (!event)
            return;

        if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouse->button != sf::Mouse::Button::Left)
                return;
        }
        else return;

        auto pos = sf::Mouse::getPosition(window);
        auto local = window.mapPixelToCoords(pos) - getPosition();

        int c = static_cast<int>(local.x / m_cellSize);
        int r = static_cast<int>(local.y / m_cellSize);

        if (r < 0 || c < 0 || r >= m_rows || c >= m_cols)
            return;

        if (m_lastClick == sf::Vector2i(r, c) &&
            m_clickClock.getElapsedTime().asMilliseconds() < 300) {
            m_pendingSingleClick = false;
            toggleQueen(r, c);
        }
        else {
            m_pendingSingleClick = true;
            m_lastClick = { r, c };
            m_clickClock.restart();
        }
    }

    void update(std::function<void()> onWin) {
        if (m_pendingSingleClick &&
            m_clickClock.getElapsedTime().asMilliseconds() >= 300) {
            togglePlus(m_lastClick.x, m_lastClick.y);
            m_pendingSingleClick = false;
        }

        if (checkWin()) {
            onWin();
        }
    }

private:
    enum class CellState { Empty, Plus, Queen };

    struct Cell {
        int region;
        sf::Color color;
        CellState state = CellState::Empty;
    };

    int m_rows{}, m_cols{};
    float m_cellSize;
    std::vector<Cell> m_cells;

    sf::Clock m_clickClock;
    sf::Vector2i m_lastClick{ -1, -1 };
    bool m_pendingSingleClick = false;

    static sf::Color regionColor(int id) {
        static sf::Color palette[] = {
            {220,20,60},{60,80,200},{60,180,90},
            {200,200,60},{180,60,180},{60,180,180}
        };
        return palette[id % (sizeof(palette) / sizeof(sf::Color))];
    }

    Cell& cell(int r, int c) { return m_cells[r * m_cols + c]; }
    const Cell& cell(int r, int c) const { return m_cells[r * m_cols + c]; }

    void togglePlus(int r, int c) {
        auto& cell = this->cell(r, c);
        if (cell.state == CellState::Queen) return;
        cell.state = (cell.state == CellState::Plus)
            ? CellState::Empty
            : CellState::Plus;
    }

    void toggleQueen(int r, int c) {
        auto& cell = this->cell(r, c);
        if (cell.state == CellState::Queen) {
            cell.state = CellState::Empty;
            return;
        }
        if (canPlaceQueen(r, c))
            cell.state = CellState::Queen;
    }

    bool canPlaceQueen(int r, int c) const {
        const auto& t = cell(r, c);

        for (const auto& cell : m_cells)
            if (cell.state == CellState::Queen && cell.region == t.region)
                return false;

        for (int i = 0; i < m_rows; ++i)
            if (cell(i, c).state == CellState::Queen)
                return false;

        for (int i = 0; i < m_cols; ++i)
            if (cell(r, i).state == CellState::Queen)
                return false;

        for (int dr = -1; dr <= 1; ++dr)
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int nr = r + dr, nc = c + dc;
                if (nr >= 0 && nc >= 0 && nr < m_rows && nc < m_cols &&
                    cell(nr, nc).state == CellState::Queen)
                    return false;
            }

        return true;
    }

    bool checkWin() const {
        std::unordered_set<int> regionsWithQueen;
        int queenCount = 0;

        for (const auto& cell : m_cells) {
            if (cell.state == CellState::Queen) {
                queenCount++;
                regionsWithQueen.insert(cell.region);
            }
        }

        return queenCount > 0 &&
            queenCount == (int)regionsWithQueen.size() &&
            queenCount == countRegions();
    }

    int countRegions() const {
        std::unordered_set<int> regions;
        for (const auto& cell : m_cells)
            regions.insert(cell.region);
        return regions.size();
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();

        sf::RectangleShape tile({ m_cellSize - 2.f, m_cellSize - 2.f });
        sf::CircleShape queen(m_cellSize * 0.35f);
        queen.setFillColor(sf::Color::Black);
        sf::RectangleShape plus;
        plus.setFillColor(sf::Color::Black);

        for (int r = 0; r < m_rows; ++r)
            for (int c = 0; c < m_cols; ++c) {
                const auto& cell = this->cell(r, c);

                tile.setPosition({ c * m_cellSize + 1, r * m_cellSize + 1 });
                tile.setFillColor(cell.color);
                target.draw(tile, states);

                if (cell.state == CellState::Queen) {
                    queen.setPosition({
                        c * m_cellSize + m_cellSize * 0.15f,
                        r * m_cellSize + m_cellSize * 0.15f
                        });
                    target.draw(queen, states);
                }
                else if (cell.state == CellState::Plus) {
                    float t = m_cellSize * 0.1f, l = m_cellSize * 0.6f;
                    plus.setSize({ l, t });
                    plus.setPosition({ c * m_cellSize + m_cellSize * 0.2f,
                                       r * m_cellSize + m_cellSize * 0.5f - t / 2 });
                    target.draw(plus, states);
                    plus.setSize({ t, l });
                    plus.setPosition({ c * m_cellSize + m_cellSize * 0.5f - t / 2,
                                       r * m_cellSize + m_cellSize * 0.2f });
                    target.draw(plus, states);
                }
            }
    }
};

/*
    ==========================
    Main
    ==========================
*/

constexpr auto GRID_WIDTH = 10;
constexpr auto GRID_HEIGHT = 10;
constexpr auto CELL_SIZE = 80.0f;

#define WINDOW_WIDTH (GRID_WIDTH * CELL_SIZE)
#define WINDOW_HEIGHT (GRID_HEIGHT * CELL_SIZE)

int main() {
    sf::RenderWindow window(sf::VideoMode({ (unsigned int)WINDOW_WIDTH, (unsigned int)WINDOW_HEIGHT }), "Queens");
    window.setFramerateLimit(144);

    Level level = generateLevel(GRID_WIDTH, GRID_HEIGHT);
    QueensGrid grid(level, CELL_SIZE);

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            grid.handleEvent(event, window);
        }

        grid.update([&]() {
            level = generateLevel(5, 5);
            grid.reset(level);
            });

        window.clear();
        window.draw(grid);
        window.display();
    }
}
