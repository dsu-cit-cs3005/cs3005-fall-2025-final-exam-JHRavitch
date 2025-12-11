#include "Arena.h"
#include "Config.h"
#include "RobotBase.h"
#include "EventHandler.h"
#include <iostream>
#include <memory>
#include <vector>
#include <dlfcn.h>
#include <filesystem>
#include <chrono>
#include <thread>

namespace fs = std::filesystem;

int main() {
    std::cout << "=== ROBOTWARZ - LOADING ROBOTS FROM .so FILES ===\n" << std::endl;
    
    // Use DEFAULT config
    GameConfig config;
    
    std::cout << "Config: " << config.rows << "x" << config.cols << " arena" << std::endl;
    std::cout << "Looking for robot .so files in: " << config.robot_directory << std::endl;
    
    std::vector<std::shared_ptr<RobotBase>> robots;
    
    // Load .so files
    try {
        for (const auto& entry : fs::directory_iterator(config.robot_directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".so") {
                std::string so_file = entry.path().string();
                std::cout << "Loading: " << so_file << std::endl;
                
                void* handle = dlopen(so_file.c_str(), RTLD_LAZY);
                if (!handle) {
                    std::cerr << "  ERROR: " << dlerror() << std::endl;
                    continue;
                }
                
                dlerror(); // Clear errors
                RobotFactory create_robot = (RobotFactory)dlsym(handle, "create_robot");
                
                if (dlerror()) {
                    std::cerr << "  ERROR: No create_robot() function" << std::endl;
                    dlclose(handle);
                    continue;
                }
                
                RobotBase* robot_ptr = create_robot();
                if (robot_ptr) {
                    robots.push_back(std::shared_ptr<RobotBase>(robot_ptr, 
                        [handle](RobotBase* rb) { delete rb; dlclose(handle); }));
                    std::cout << "  SUCCESS: Loaded " << robot_ptr->m_name << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Directory error: " << e.what() << std::endl;
    }
    
    if (robots.empty()) {
        std::cerr << "\nERROR: No robots loaded. Place robot .so files in: " 
                  << config.robot_directory << std::endl;
        return 1;
    }
    
    std::cout << "\nSuccessfully loaded " << robots.size() << " robot(s)" << std::endl;
    
    // Create Arena and EventHandler
    std::cout << "\nInitializing Arena and EventHandler..." << std::endl;
    std::cout << "══════════════════════════════════════════════════════" << std::endl;
    
    Arena arena(config, robots);
    EventHandler event_handler(arena);
    
    // Display initial state
    std::cout << "\n=== INITIAL STATE ===" << std::endl;
    event_handler.printGameState(0);
    
    // Game loop
    int max_rounds = config.max_rounds;
    bool watch_live = config.watch_live;
    
    for (int round = 1; round <= max_rounds; round++) {
        // Print round header using EventHandler
        event_handler.printRoundHeader(round, max_rounds);
        
        // Process each robot's turn
        for (size_t i = 0; i < robots.size(); i++) {
            // Skip dead robots
            if (robots[i]->get_health() <= 0) {
                continue;
            }
            
            event_handler.processRobotTurn(i, round);
        }
        
        // Display game state after all robots have moved
        event_handler.printGameState(round);
        
        // Check for winner
        if (event_handler.checkForWinner()) {
            std::cout << "\n════════════════════ GAME OVER ════════════════════" << std::endl;
            std::cout << "Winner detected! Game ended on round " << round << std::endl;
            break;
        }
        
        // Pause between rounds if watching live
        if (watch_live && round < max_rounds) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config.turn_delay_ms));
        }
    }
    
    // Final state
    std::cout << "\n════════════════════ FINAL STATE ════════════════════" << std::endl;
    event_handler.printGameState(max_rounds);
    
    // Winner announcement
    int alive_count = event_handler.countAliveRobots();
    if (alive_count == 1) {
        // Find the winner
        for (size_t i = 0; i < robots.size(); i++) {
            if (robots[i]->get_health() > 0) {
                std::cout << "\n🏆 WINNER: Robot " << i << " - " << robots[i]->m_name 
                          << " (" << robots[i]->m_character << ")!" << std::endl;
                break;
            }
        }
    } else if (alive_count == 0) {
        std::cout << "\n💀 DRAW: All robots destroyed!" << std::endl;
    } else {
        std::cout << "\n⏱️  TIMEOUT: Multiple robots still alive after " << max_rounds << " rounds" << std::endl;
    }
    
    return 0;
}