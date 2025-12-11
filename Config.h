#pragma once
#include <string>
#include <vector>

struct GameConfig {
    // Arena dimensions
    int rows = 30;
    int cols = 30;
    int area = rows * cols;

    // Game rules
    int max_rounds = 100;
    bool watch_live = true;
    int turn_delay_ms = 500;
    
    // Obstacles
    int mounds = 5*area/100;
    int pits = 2*area/100;
    int flamethrowers = 1*area/100;
    
    // Robot loading
    std::string robot_directory = ".";
    
    // Display
    bool show_grid_numbers = true;
    bool verbose_logging = false;

};