// EventHandler.h
#pragma once

#include "Arena.h"
#include "RadarObj.h"
#include <vector>
#include <iomanip>

class EventHandler {
private:
    Arena& arena_;
    
public:
    EventHandler(Arena& arena);
    
    // Radar system
    std::vector<RadarObj> scanRadar(int robot_id, int direction);
    
    // Movement system
    bool processMovement(int robot_id, int direction, int distance);
    
    // Combat system  
    bool processShot(int shooter_id, int target_row, int target_col);
    
    // Turn processing
    void processRobotTurn(int robot_id, int round_number);
    
    // Game state
    bool checkForWinner() const;
    int countAliveRobots() const;

    // Display Methods
    void printGameState(int round_number) const;
    void printRobotStatus(int robot_id) const;
    void printRoundHeader(int round_number, int max_rounds) const;
    std::string formatRobotStats(RobotBase& robot) const;
};
