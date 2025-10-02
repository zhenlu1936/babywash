CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -g

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test

TARGET_WASHER = $(BUILD_DIR)/babyfuckwasher
SRC_WASHER = $(SRC_DIR)/babyfuckwasher.cpp
TARGET_MAIN = $(BUILD_DIR)/babywash
SRC_MAIN = $(SRC_DIR)/babywash.cpp
TARGET_FUCKER = $(BUILD_DIR)/babywashfucker
SRC_FUCKER = $(SRC_DIR)/babywashfucker.cpp

BR_PROGRAM = $(TEST_DIR)/helloworld.bw
WASH_PROGRAM = $(BUILD_DIR)/wash_program.txt
MEM_BIN = $(BUILD_DIR)/mem.bin
BIN_CODE = $(BUILD_DIR)/binarycodes.bin

all: $(BUILD_DIR) $(TARGET_WASHER) $(TARGET_MAIN) $(TARGET_FUCKER)

run: $(BUILD_DIR) $(TARGET_WASHER) $(TARGET_MAIN) $(TARGET_FUCKER)
	./$(TARGET_WASHER) < $(BR_PROGRAM) > $(WASH_PROGRAM)
	./$(TARGET_MAIN) $(MEM_BIN) < $(WASH_PROGRAM)
	./$(TARGET_FUCKER) $(MEM_BIN) $(BIN_CODE)

$(BUILD_DIR):
	mkdir -p build

$(TARGET_WASHER): $(SRC_WASHER)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_FUCKER): $(SRC_FUCKER)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -r build