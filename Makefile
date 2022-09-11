SRC_DIR := ./src
INC_DIR := ./inc
OBJ_DIR := ./obj
BUILD_DIR := ./build

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
MTL_FILES := $(wildcard $(SRC_DIR)/*.metal)

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
AIR_FILES := $(patsubst $(SRC_DIR)/%.metal,$(OBJ_DIR)/%.air,$(MTL_FILES))

DBG_OPT_FLAGS=-O2
# DBG_OPT_FLAGS=-g
ASAN_FLAGS=
#ASAN_FLAGS=-fsanitize=address

CC=clang++
CFLAGS=-Wall -std=c++17 -I $(INC_DIR) -fno-objc-arc $(DBG_OPT_FLAGS) $(ASAN_FLAGS)
LDFLAGS=-framework Metal -framework Foundation -framework CoreGraphics
LDFLAGS+=-Wl,-sectcreate,metallib,metallib,$(OBJ_DIR)/default.metallib
MTLCC=xcrun -sdk macosx metal
MTLLD=xcrun -sdk macosx metallib

.PHONY: directories

all: directories $(OBJ_DIR)/default.metallib $(BUILD_DIR)/vecadd

directories: ${BUILD_DIR} ${OBJ_DIR}

${BUILD_DIR}:
	mkdir -p ${BUILD_DIR}

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

$(BUILD_DIR)/vecadd: ${OBJ_FILES}
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $^ $(FRAMEWORKS)

$(OBJ_DIR)/default.metallib: ${AIR_FILES}
	${MTLLD} -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	${CC} $(CFLAGS) -c $< -o $@
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	${CC} $(CFLAGS) -c $< -o $@
$(OBJ_DIR)/%.air: $(SRC_DIR)/%.metal
	${MTLCC} -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BUILD_DIR)/* *.dSYM
