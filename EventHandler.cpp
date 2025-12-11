// EventHandler.cpp  
#include "EventHandler.h"
#include <iostream>

EventHandler::EventHandler(Arena& arena) : arena_(arena) {}

std::vector<RadarObj> EventHandler::scanRadar(int robot_id, int direction) {
    std::vector<RadarObj> radar_results;
    
    if (direction < 0 || direction > 8) {return radar_results;}  // Invalid direction
    
    // Get robot position
    const auto& robot_positions = arena_.getRobotPositions();
    if (robot_id < 0 || robot_id >= robot_positions.size()) {return radar_results;}  // Invalid robot ID
    
    int robot_row = robot_positions[robot_id].row;
    int robot_col = robot_positions[robot_id].col;
    
    // Direction 0: 8 squares immediately surrounding the robot
    if (direction == 0) {
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;  // Skip robot itself
                
                int target_row = robot_row + dr;
                int target_col = robot_col + dc;
                
                // Check bounds
                if (target_row >= 0 && target_row < arena_.getRows() &&
                    target_col >= 0 && target_col < arena_.getCols()) {
                    
                    char cell_content = arena_.getCell(target_row, target_col);
                    radar_results.emplace_back(cell_content, target_row, target_col);
                }
            }
        }
        return radar_results;
    }
    
    // Directions 1-8: 3-wide ray to edge of arena
    // Get direction vector from RobotBase's directions array
    int dir_row = directions[direction].first;
    int dir_col = directions[direction].second;
    
    // For 3-wide ray, we need perpendicular offsets
    // For cardinal directions (1,3,5,7), offset is perpendicular
    // For diagonal directions (2,4,6,8), offset is along both axes
    
    std::vector<std::pair<int, int>> width_offsets;
    
    if (dir_row == 0 || dir_col == 0) {
        // Cardinal direction (up/down/left/right)
        if (dir_row == 0) {  // Horizontal (left/right)
            width_offsets = {{-1, 0}, {0, 0}, {1, 0}};  // Up, center, down
        } else {  // Vertical (up/down)
            width_offsets = {{0, -1}, {0, 0}, {0, 1}};  // Left, center, right
        }
    } else {
        // Diagonal direction
        width_offsets = {{-1, -1}, {0, 0}, {1, 1}};  // Both axes
    }
    
    // Trace each of the 3 rays
    for (const auto& offset : width_offsets) {
        int current_row = robot_row + dir_row + offset.first;
        int current_col = robot_col + dir_col + offset.second;
        
        while (current_row >= 0 && current_row < arena_.getRows() &&
               current_col >= 0 && current_col < arena_.getCols()) {
            if (current_row == robot_row && current_col == robot_col) {current_row += dir_row; current_col += dir_col; continue;}
            
            char cell_content = arena_.getCell(current_row, current_col);
            radar_results.emplace_back(cell_content, current_row, current_col);
            
            // Continue along ray
            current_row += dir_row;
            current_col += dir_col;
        }
    }
    return radar_results;
}

bool EventHandler::processMovement(int robot_id, int direction, int requested_distance) {
    std::cout << "  [MOVE] Robot " << robot_id << " moving dir " << direction 
              << " dist " << requested_distance << std::endl;
    
    const auto& robot_positions = arena_.getRobotPositions();
    if (robot_id < 0 || robot_id >= robot_positions.size()) {
        return false;
    }
    
    const auto& robot_info = robot_positions[robot_id];
    auto robot = robot_info.robot;
    
    // Check pit
    if (robot->get_move_speed() == 0) {
        std::cout << "  [MOVE] Robot in pit, cannot move" << std::endl;
        return false;
    }
    
    // Cap speed
    int max_move = robot->get_move_speed();
    int actual_distance = (requested_distance > max_move) ? max_move : requested_distance;
    
    if (direction < 1 || direction > 8) {
        return false;
    }
    
    int dir_row = directions[direction].first;
    int dir_col = directions[direction].second;
    
    int current_row = robot_info.row;
    int current_col = robot_info.col;
    bool current_on_flame = robot_info.on_flamethrower;
    if (current_on_flame) {
        int damage = 30 + (std::rand() % 21);
            std::cout << "  [Robot really ended it's turn on a flamethrower??? Robot takes " << damage << " damage!" << std::endl;
            robot->take_damage(damage);
    }
    int steps_taken = 0;
    
    for (int step = 1; step <= actual_distance; step++) {
        int next_row = current_row + dir_row;
        int next_col = current_col + dir_col;
        
        // Check bounds
        if (next_row < 0 || next_row >= arena_.getRows() ||
            next_col < 0 || next_col >= arena_.getCols()) {
            break;
        }
        
        char cell_content = arena_.getCell(next_row, next_col);
        
        // Handle cell types
        if ((cell_content != '.') && (cell_content != 'F') && (cell_content != 'P')) {
            break;  // Stop at mounds/robots
        }
        else if (cell_content == 'P') {
            // Move onto pit
            current_row = next_row;
            current_col = next_col;
            steps_taken = step;
            robot->disable_movement();  // Trap in pit
            break;
        }
        else if (cell_content == 'F') {
            // Move onto flamethrower
            current_row = next_row;
            current_col = next_col;
            current_on_flame = true;
            steps_taken = step;
            
            // Take damage
            int damage = 30 + (std::rand() % 21);
            std::cout << "  [MOVE] Robot takes " << damage << " flamethrower damage!" << std::endl;
            robot->take_damage(damage);
            
            continue;  // Can continue moving from flamethrower
        }
        else if (cell_content >= 'A' && cell_content <= 'Z') {
            break;  // Stop at other robots
        }
        else if (cell_content == '.') {
            current_row = next_row;
            current_col = next_col;
            current_on_flame = false;  // Not on flamethrower anymore
            steps_taken = step;
            continue;
        }
        else {
            break;  // Unknown cell
        }
    }
    
    // If robot moved, update position
    if (steps_taken > 0) {
        // Clear old position
        if (robot_info.on_flamethrower) {
            arena_.setCell(robot_info.row, robot_info.col, 'F');  // Restore flamethrower
        } else {
            arena_.setCell(robot_info.row, robot_info.col, '.');  // Restore empty
        }
        
        // Update robot tracking (need mutable access)
        // We'll need to add a method to Arena to update robot position
        bool success = arena_.updateRobotPosition(robot_id, current_row, current_col, current_on_flame);
        
        if (success) {
            std::cout << "  [MOVE] Moved to (" << current_row << "," << current_col << ")";
            if (current_on_flame) std::cout << " (on flamethrower)";
            std::cout << std::endl;
            return true;
        }
    }
    
    return false;
}

bool EventHandler::processShot(int shooter_id, int target_row, int target_col) {
    std::cout << "  TODO: Robot " << shooter_id 
              << " shoots at (" << target_row << ", " << target_col << ")" << std::endl;
    return false;
}

void EventHandler::processRobotTurn(int robot_id, int round_number) {
    std::cout << "\n  Processing turn for robot " << robot_id << std::endl;
    
    // 1. Get radar direction
    int radar_dir = 0;
    auto& robot = arena_.getRobots()[robot_id];
    robot->get_radar_direction(radar_dir);
    
    // 2. Scan radar
    auto radar_results = scanRadar(robot_id, radar_dir);
    
    // 3. Process radar results
    robot->process_radar_results(radar_results);
    
    // 4. Get shot location
    int shot_row = 0, shot_col = 0;
    if (robot->get_shot_location(shot_row, shot_col)) {
        processShot(robot_id, shot_row, shot_col);
    } else {
        // 5. Get movement
        int move_dir = 0, move_dist = 0;
        robot->get_move_direction(move_dir, move_dist);
        if (move_dir != 0) {
            processMovement(robot_id, move_dir, move_dist);
        }
    }
}

bool EventHandler::checkForWinner() const {
    return (countAliveRobots() <= 1);
}

int EventHandler::countAliveRobots() const {
    int alive = 0;
    for (const auto& robot : arena_.getRobots()) {
        if (robot->get_health() > 0) {
            alive++;
        }
    }
    return alive;
}


void EventHandler::printRoundHeader(int round_number, int max_rounds) const {
    std::cout << "\n╔══════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    ROUND " << std::setw(3) << round_number 
              << " / " << std::setw(3) << max_rounds << "                    ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════╝" << std::endl;
}

std::string EventHandler::formatRobotStats(RobotBase& robot) const {
    std::stringstream ss;
    ss << robot.m_name << " '" << robot.m_character << "'";
    ss << " | H:" << std::setw(3) << robot.get_health();
    ss << " A:" << std::setw(1) << robot.get_armor();
    ss << " M:" << std::setw(1) << robot.get_move_speed();
    ss << " W:" << robot.get_weapon();
    if (robot.get_grenades() > 0) {
        ss << " G:" << robot.get_grenades();
    }
    
    // Get robot position
    int row, col;
    robot.get_current_location(row, col);
    ss << " @(" << row << "," << col << ")";
    
    return ss.str();
}

void EventHandler::printRobotStatus(int robot_id) const {
    const auto& robots = arena_.getRobots();
    const auto& positions = arena_.getRobotPositions();
    
    if (robot_id < 0 || robot_id >= robots.size()) {
        return;
    }
    
    const auto& robot = robots[robot_id];
    const auto& pos = positions[robot_id];
    
    std::cout << "  Robot " << robot_id << ": " << formatRobotStats(*robot);
    
    if (robot->get_health() <= 0) {
        std::cout << " [DEAD]";
    } else if (robot->get_move_speed() == 0) {
        std::cout << " [TRAPPED IN PIT]";
    } else if (pos.on_flamethrower) {
        std::cout << " [ON FLAMETHROWER]";
    }
    
    std::cout << std::endl;
}

void EventHandler::printGameState(int round_number) const {
    // Print arena
    arena_.printArena();
    
    std::cout << "\n════════════════════ ROBOT STATUS ════════════════════" << std::endl;
    std::cout << "Round: " << round_number;
    std::cout << " | Alive: " << countAliveRobots() << "/" << arena_.getRobots().size();
    std::cout << std::endl;
    
    for (size_t i = 0; i < arena_.getRobots().size(); ++i) {
        printRobotStatus(i);
    }
    std::cout << std::endl;
}