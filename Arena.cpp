#include "Arena.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

Arena::Arena(const GameConfig& config, const std::vector<std::shared_ptr<RobotBase>>& robots) 
    : rows_(config.rows), cols_(config.cols), 
      grid_(config.rows, std::vector<char>(config.cols, '.')),
      show_grid_numbers_(config.show_grid_numbers) {
    
    std::cout << "Initializing Arena " << rows_ << "x" << cols_ << std::endl;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    placeObstacles(config);
    
    // Add all robots passed from main
    for (auto& robot : robots) {
        addRobot(robot);
    }
}

Arena::~Arena() {
    std::cout << "Cleaning up Arena..." << std::endl;
}

void Arena::placeObstacles(const GameConfig& config) {
    std::cout << "Generating obstacles: " 
              << config.mounds << " mounds, "
              << config.pits << " pits, "
              << config.flamethrowers << " flamethrowers" << std::endl;
    
    int counter = 0;
    while (counter < config.mounds) {
        int r = std::rand() % rows_; int c = std::rand() % cols_;
        if (grid_[r][c] == '.') {
            grid_[r][c] = 'M';
            counter++;
        }
    }
    counter = 0;
    while (counter < config.pits) {
        int r = std::rand() % rows_; int c = std::rand() % cols_;
        if (grid_[r][c] == '.') {
            grid_[r][c] = 'P';
            counter++;
        }
    }
    counter = 0;
    while (counter < config.flamethrowers) {
        int r = std::rand() % rows_; int c = std::rand() % cols_;
        if (grid_[r][c] == '.') {
            grid_[r][c] = 'F'; 
            counter++;
        }
    }
}

void Arena::addRobot(std::shared_ptr<RobotBase> robot) {
    if (!robot) return;
    int r = std::rand() % rows_; int c = std::rand() % cols_;
    while (grid_[r][c] != '.') {r = std::rand() % rows_; c = std::rand() % cols_;}
    robot->set_boundaries(rows_, cols_);
    robot->move_to(r, c);
    RobotInfo info;
    info.id = robots_.size();
    info.row = r; info.col = c; info.on_flamethrower = false;
    info.robot = robot;
    robot_positions_.push_back(info);
    robots_.push_back(robot);
    std::cout << "Placed robot " << robot->m_name 
              << " at (" << r << ", " << c << ")"
              << " with character '" << robot->m_character << "'" << std::endl;
    
    grid_[r][c] = robot->m_character;
}

void Arena::printArena() const {
    std::cout << "\n=== ARENA STATE ===\n";
    
    // Column headers - each column number takes 3 spaces
    std::cout << "  ";
    for (int c = 0; c < cols_; ++c) {std::cout << std::setw(3) << c;}
    std::cout << "\n";
    
    // Top border
    std::cout << "  +";
    for (int c = 0; c < cols_; ++c) {std::cout << "---";}
    std::cout << "+\n";
    
    // Grid
    for (int r = 0; r < rows_; ++r) {
        std::cout << std::setw(2) << r << "|";
        for (int c = 0; c < cols_; ++c) {
            char cell = grid_[r][c];
            
            // Check if there's a robot at this position
            bool is_robot = false;
            bool is_alive = false;
            bool on_fire = false;
            if ((cell != '.') && (cell != 'M') && (cell != 'P') && (cell != 'F')) {
                for (const auto& info : robot_positions_) {
                    if (info.row == r && info.col == c) {
                        is_robot = true;
                        is_alive = (info.robot->get_health() > 0);
                        on_fire = info.on_flamethrower;
                        break;
                    }
                }
            }
            
            // Display
            // Change this section in printArena():
            if (is_robot) {
                if (is_alive) {
                    if (on_fire) {std::cout << "f" << cell << "f";  // Robot on flamethrower
                    } else {std::cout << "<" << cell << ">";}  // Normal robot
                } else {std::cout << "x" << cell << "x";}  // Dead robot
            } else {std::cout << " " << cell << " ";}
        }
        std::cout << "|" << r << "\n";
    }

    // Bottom border
    std::cout << "  +";
    for (int c = 0; c < cols_; ++c) {
        std::cout << "---";
    }
    std::cout << "+\n";
    std::cout << "  ";
    for (int c = 0; c < cols_; ++c) {std::cout << std::setw(3) << c;}
    std::cout << "\n";
}

void Arena::printRobotInfo() const {
    std::cout << "\n=== ROBOT STATUS ===\n";
    for (size_t i = 0; i < robots_.size(); ++i) {
        const auto& robot = robots_[i];
        const auto& pos = robot_positions_[i];
        
        std::cout << "Robot " << i << ": " << robot->m_name << ", Char: '" << robot->m_character << "'\n";
        std::cout << " Stats: " << robot->print_stats() << "\n";
        std::cout << std::endl;
    }
}

void Arena::setCell(int row, int col, char val) {grid_[row][col] = val;}
char Arena::getCell(int row, int col) const {return grid_[row][col];}

bool Arena::updateRobotPosition(int robot_id, int new_row, int new_col, bool on_flamethrower) {
    if (robot_id < 0 || robot_id >= robot_positions_.size()) {return false;}
    
    if (new_row < 0 || new_row >= rows_ || new_col < 0 || new_col >= cols_) {return false;}
    
    auto& robot_info = robot_positions_[robot_id];
    
    // Update robot's internal location
    robot_info.robot->move_to(new_row, new_col);
    
    // Update tracking info
    robot_info.row = new_row;
    robot_info.col = new_col;
    robot_info.on_flamethrower = on_flamethrower;
    
    // Update grid
    grid_[new_row][new_col] = robot_info.robot->m_character;
    
    return true;
}