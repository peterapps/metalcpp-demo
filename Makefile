SRC_DIR := ./src
INC_DIR := ./inc
OBJ_DIR := ./obj
BUILD_DIR := ./build

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
METAL_FILES := $(wildcard $(SRC_DIR)/*.metal)

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
AIR_FILES := $(patsubst $(SRC_DIR)/%.metal,$(OBJ_DIR)/%.air,$(METAL_FILES))

DBG_OPT_FLAGS=-O2
#DBG_OPT_FLAGS=-g
ASAN_FLAGS=
#ASAN_FLAGS=-fsanitize=address

CC=clang++
CFLAGS=-Wall -std=c++17 -I $(INC_DIR) -fno-objc-arc $(DBG_OPT_FLAGS) $(ASAN_FLAGS)
LDFLAGS=-framework Metal -framework Foundation
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
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $^ $(FRAMEWORKS)

$(BUILD_DIR)/default.metallib: ${AIR_FILES}
	${METALLD} -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	${CC} $(CFLAGS) -c $< -o $@
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	${CC} $(CFLAGS) -c $< -o $@
$(OBJ_DIR)/%.air: $(SRC_DIR)/%.metal
	${METALCC} -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BUILD_DIR)/* *.dSYM
