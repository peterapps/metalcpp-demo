SRC_DIR := ./src
INC_DIR := ./inc
OBJ_DIR := ./obj
BUILD_DIR := ./build

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
METAL_FILES := $(wildcard $(SRC_DIR)/*.metal)

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
AIR_FILES := $(patsubst $(SRC_DIR)/%.metal,$(OBJ_DIR)/%.air,$(METAL_FILES))

FRAMEWORKS := -framework Foundation -framework Metal

CC=clang++ -Wall -std=c++17 -I $(INC_DIR)
METALCC=xcrun -sdk macosx metal
METALLD=xcrun -sdk macosx metallib

.PHONY: directories

all: directories $(BUILD_DIR)/vecadd $(BUILD_DIR)/default.metallib

directories: ${BUILD_DIR} ${OBJ_DIR}

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

$(BUILD_DIR)/vecadd: ${OBJ_FILES}
	${CC} -o $@ $^ $(FRAMEWORKS)

$(BUILD_DIR)/default.metallib: ${AIR_FILES}
	${METALLD} -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	${CC} -c $< -o $@
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	${CC} -c $< -o $@
$(OBJ_DIR)/%.air: $(SRC_DIR)/%.metal
	${METALCC} -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BUILD_DIR)/* *.dSYM
