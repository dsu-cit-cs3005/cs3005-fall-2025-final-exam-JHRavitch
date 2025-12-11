#pragma once

#include "RobotBase.h"
#include "Config.h"
#include <vector>
#include <memory>

class Arena {
private:
    // Grid dimensions
    int rows_;
    int cols_;
    
    // Display grid ('.' = empty, 'M'/'P'/'F' = obstacles, letters = robots)
    std::vector<std::vector<char>> grid_;
    
    // Display setting from config
    bool show_grid_numbers_;
    
    // Robot tracking
    struct RobotInfo {
        int id;
        int row;
        int col;
        bool on_flamethrower;
        std::shared_ptr<RobotBase> robot;
    };
    std::vector<RobotInfo> robot_positions_;
    std::vector<std::shared_ptr<RobotBase>> robots_;

public:
    // Constructor takes config AND pre-loaded robots
    Arena(const GameConfig& config, const std::vector<std::shared_ptr<RobotBase>>& robots);
    ~Arena();
    
    // Display
    void printArena() const;
    
    // Getters
    bool updateRobotPosition(int robot_id, int new_row, int new_col, bool on_flamethrower);
    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
    char getCell(int row, int col) const;
    void setCell(int row, int col, char val);
    const std::vector<std::shared_ptr<RobotBase>>& getRobots() const { return robots_; }
    const std::vector<RobotInfo>& getRobotPositions() const { return robot_positions_; }
    void printRobotInfo() const;

private:
    // Internal methods
    void placeObstacles(const GameConfig& config);
    void addRobot(std::shared_ptr<RobotBase> robot);
};