#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <omp.h>

#include <chrono>
#include <thread>
#include <SFML/Graphics.hpp>

#define GRID_SIZE 50
#define STEPS 1000
#define THREADS 4
#define CELL_SIZE 12

using namespace std;

// Initialize a random grid
void initialize_grid(vector<vector<int>>& grid) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = rand() % 2;  // Random 0 or 1
        }
    }
}

// Count live neighbors
int count_neighbors(const vector<vector<int>>& grid, int x, int y) {
    int count = 0;
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
            count += grid[nx][ny];
        }
    }
    return count;
}

// Compute next generation (parallel)
void next_generation_parallel(vector<vector<int>>& grid) {
    vector<vector<int>> new_grid = grid;
    
    #pragma omp parallel for num_threads(THREADS) collapse(2)
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int neighbors = count_neighbors(grid, i, j);
            if (grid[i][j] == 1 && (neighbors < 2 || neighbors > 3)) {
                new_grid[i][j] = 0; // Dies
            } else if (grid[i][j] == 0 && neighbors == 3) {
                new_grid[i][j] = 1; // Becomes alive
            }
        }
    }
    grid.swap(new_grid);
}

// Compute next generation (sequential)
void next_generation_sequential(vector<vector<int>>& grid) {
    vector<vector<int>> new_grid = grid;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            int neighbors = count_neighbors(grid, i, j);
            if (grid[i][j] == 1 && (neighbors < 2 || neighbors > 3)) {
                new_grid[i][j] = 0;
            } else if (grid[i][j] == 0 && neighbors == 3) {
                new_grid[i][j] = 1;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(35)); 
    }
    grid.swap(new_grid);
}

// Render the grid using SFML
void render_grid(sf::RenderWindow &window, const vector<vector<int>>& grid) {
    window.clear(sf::Color(20, 20, 30)); 
    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
    
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 1) {
                cell.setPosition(j * CELL_SIZE + 1, i * CELL_SIZE + 1);
                cell.setFillColor(sf::Color(100, 255, 100)); 
                cell.setOutlineColor(sf::Color(0, 200, 0, 180));
                cell.setOutlineThickness(1);
                window.draw(cell);
            }
        }
    }
    window.display();
}

int main() {
    srand(time(0));
    vector<vector<int>> grid(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    initialize_grid(grid);
    
    sf::RenderWindow window(sf::VideoMode(GRID_SIZE * CELL_SIZE, GRID_SIZE * CELL_SIZE), "Game of Life", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    
    bool use_parallel = false;
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Failed to load font!\n";
        return -1;
    }
    
    sf::Text text("Use Parallel Processing? (Y/N)", font, 24);
    text.setFillColor(sf::Color::White);
    text.setPosition(50, GRID_SIZE * CELL_SIZE / 2 - 20);
    
    bool user_selected = false;
    while (window.isOpen() && !user_selected) {
        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Y) {
                    use_parallel = true;
                    user_selected = true;
                } else if (event.key.code == sf::Keyboard::N) {
                    use_parallel = false;
                    user_selected = true;
                }
            }
        }
    }
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
        
        auto start = chrono::high_resolution_clock::now();
        if (use_parallel) {
            next_generation_parallel(grid);
        } else {
            next_generation_sequential(grid);
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        cout << (use_parallel ? "Parallel: " : "Sequential: ") << elapsed.count() << " seconds" << endl;
        
        render_grid(window, grid);
    }
    return 0;
}
