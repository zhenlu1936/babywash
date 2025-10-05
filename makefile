CXX = g++
CXXFLAGS = -std=c++20 -Wall -O0 -g

BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test
BR_DIR = bf
BW_DIR = bw

TARGET_INITIALIZER = $(BUILD_DIR)/babywash_initializer
SRC_INITIALIZER = $(SRC_DIR)/babywash_initializer.cpp
TARGET_WASHER = $(BUILD_DIR)/babyfuckwasher
SRC_WASHER = $(SRC_DIR)/babyfuckwasher.cpp
TARGET_MAIN = $(BUILD_DIR)/babywash
SRC_MAIN = $(SRC_DIR)/babywash_jit.cpp
TARGET_FUCKER = $(BUILD_DIR)/babywashfucker
SRC_FUCKER = $(SRC_DIR)/babywashfucker.cpp

PROGRAM = echo
BR_PROGRAM = $(TEST_DIR)/$(BR_DIR)/$(PROGRAM).bf
WASH_PROGRAM = $(TEST_DIR)/$(BW_DIR)/$(PROGRAM).bw
MEM_BIN = $(BUILD_DIR)/mem.bin
BIN_CODE = $(BUILD_DIR)/binarycodes.bin
INITIALIZER_BIN = $(BUILD_DIR)/initialize.bin

COMPILER_MODE = 0 # interpreter

all: $(BUILD_DIR) $(TARGET_INITIALIZER) $(TARGET_WASHER) $(TARGET_MAIN) $(TARGET_FUCKER)

run: $(BUILD_DIR) $(TARGET_INITIALIZER) $(TARGET_WASHER) $(TARGET_MAIN) $(TARGET_FUCKER)
	./$(TARGET_INITIALIZER) > $(INITIALIZER_BIN)
	./$(TARGET_WASHER) < $(BR_PROGRAM) > $(WASH_PROGRAM)
	./$(TARGET_MAIN) $(MEM_BIN) ${WASH_PROGRAM} < $(INITIALIZER_BIN)
	./$(TARGET_FUCKER) $(MEM_BIN) $(BIN_CODE)

$(BUILD_DIR):
	mkdir -p build

$(TARGET_INITIALIZER): $(SRC_INITIALIZER)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_WASHER): $(SRC_WASHER)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TARGET_FUCKER): $(SRC_FUCKER)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -r build