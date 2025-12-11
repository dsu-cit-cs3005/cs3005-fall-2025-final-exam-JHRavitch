# RobotWarz Makefile
# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -I. -fPIC
LDFLAGS = -ldl -lpthread

# Project structure
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib

# Source files
MAIN_SRC = main.cpp Arena.cpp EventHandler.cpp RobotBase.cpp
MAIN_OBJ = $(addprefix $(OBJ_DIR)/, $(MAIN_SRC:.cpp=.o))

ROBOT_SRCS = Robot_Ratboy.cpp Robot_Flame_e_o.cpp
ROBOT_OBJS = $(addprefix $(OBJ_DIR)/, $(ROBOT_SRCS:.cpp=.o))
ROBOT_SOS = $(addprefix $(LIB_DIR)/, $(ROBOT_SRCS:.cpp=.so))

TEST_SRC = test_robot.cpp
TEST_OBJ = $(OBJ_DIR)/test_robot.o

# Headers
HEADERS = RobotBase.h Arena.h EventHandler.h Config.h RadarObj.h

# Targets
TARGET = $(BIN_DIR)/robotwarz
TEST_TARGET = $(BIN_DIR)/test_robot

# Default target
all: directories $(TARGET) robots

# Create necessary directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)

# Main executable
$(TARGET): $(MAIN_OBJ) $(OBJ_DIR)/RobotBase.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Test executable
$(TEST_TARGET): $(TEST_OBJ) $(OBJ_DIR)/RobotBase.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile main source files
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile RobotBase first (needed by all)
$(OBJ_DIR)/RobotBase.o: RobotBase.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -c RobotBase.cpp -o $(OBJ_DIR)/RobotBase.o

# Compile test robot
$(OBJ_DIR)/test_robot.o: test_robot.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -c test_robot.cpp -o $(OBJ_DIR)/test_robot.o

# Build robot shared libraries
robots: $(ROBOT_SOS)

# Pattern rule for robot shared libraries
$(LIB_DIR)/%.so: %.cpp $(OBJ_DIR)/RobotBase.o RobotBase.h
	$(CXX) $(CXXFLAGS) -shared -o $@ $< $(OBJ_DIR)/RobotBase.o

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)
	rm -f *.so
	rm -f *.o

# Run the game
run: all
	@echo "Copying robot .so files to current directory..."
	@cp $(LIB_DIR)/*.so . 2>/dev/null || true
	@echo "Starting RobotWarz..."
	@$(TARGET)

# Test a specific robot
test: $(TEST_TARGET) robots
	@echo "Testing Robot_Ratboy..."
	@$(TEST_TARGET) Robot_Ratboy.cpp

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean all

# Release build
release: CXXFLAGS += -O3
release: clean all

# Phony targets
.PHONY: all clean run test debug release robots directories

# Dependencies
$(OBJ_DIR)/main.o: main.cpp Arena.h EventHandler.h Config.h RobotBase.h
$(OBJ_DIR)/Arena.o: Arena.cpp Arena.h RobotBase.h Config.h
$(OBJ_DIR)/EventHandler.o: EventHandler.cpp EventHandler.h Arena.h RobotBase.h RadarObj.h