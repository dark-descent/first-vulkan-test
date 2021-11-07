OUT_DIR = out
ENGINE_NAME = nova-engine
OUT_FILE = $(OUT_DIR)/$(ENGINE_NAME)
TEST_GAME_OUT = test-game-build

TEST_GAME_SCRIPTS = $(foreach dir,$(shell find test-game/src -type d),$(wildcard $(dir)/*.ts))
TEST_GAME_SCRIPTS_OUT = $(patsubst test-game/src/%.ts,out/assets/scripts/%.js,$(TEST_GAME_SCRIPTS))

V8_DIR = ~/src/v8/v8/out.gn/x64.release/obj

_LIBS = glfw vulkan dl v8
_INCLUDE_DIRS = /usr/include/v8 include
_DEFINES = DEBUG
LIBS = $(patsubst %,-l%,$(_LIBS))
INCLUDE_DIRS = $(patsubst %,-I%,$(_INCLUDE_DIRS))
DEFINES = $(patsubst %,-D%,$(_DEFINES))

# when the return type is bool but forgot to return true or false we can get in an undefined state
# thats why we always wanna treat the warning as an error ans stop compilation when we find this
CC_WARNING_ERRORS = return-type

DISABLED_CC_WARNINGS = unknown-pragmas unused-function unused-variable unused-result

CC = g++
CFLAGS = $(patsubst %,-Werror=%,$(CC_WARNING_ERRORS)) -Wall $(patsubst %,-Wno-%,$(DISABLED_CC_WARNINGS)) -std=c++17 -O2 $(LIBS) $(INCLUDE_DIRS) $(DEFINES) -pthread -lpthread
LDFLAGS = -L$(V8_DIR) $(LIBS) $(DEFINES) -pthread  -lpthread

DIRS := $(shell find src -type d)

SRCS = $(foreach dir,$(DIRS),$(wildcard $(dir)/*.cpp))
SRCS_OUT = $(patsubst %.cpp,%.o,$(SRCS))
OBJS = $(patsubst src/%,$(OUT_DIR)/%,$(SRCS_OUT))
INCLUDES = $(wildcard include/*.hpp)

PCH_NAME = framework.hpp.pch

PCH_SRC = include/framework.hpp
PCH_OUT = $(OUT_DIR)/framework.hpp.pch

SHADER_VERT_SRCS = $(wildcard shaders/*.vert)
SHADER_FRAG_SRCS = $(wildcard shaders/*.frag)
SHADER_VERT_OUT = $(patsubst %.vert,out/assets/%.vert.spv,$(SHADER_VERT_SRCS))
SHADER_FRAG_OUT = $(patsubst %.frag,out/assets/%.frag.spv,$(SHADER_FRAG_SRCS))

test-game: $(OUT_FILE) compile-shaders test-game-scripts
	@mkdir -p $(TEST_GAME_OUT)
	@cp $(OUT_FILE) $(TEST_GAME_OUT)/$(ENGINE_NAME)
	@rm -rf $(TEST_GAME_OUT)/assets
	@cp -a $(OUT_DIR)/assets $(TEST_GAME_OUT)/assets

$(OUT_FILE): $(PCH_OUT) $(OBJS) $(INCLUDES)
	@echo "Building engine..."
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -include $(PCH_SRC) -o $@ $(OBJS) $(LDFLAGS)

.PHONY: test clean

compile-shaders: $(SHADER_VERT_OUT) $(SHADER_FRAG_OUT)

$(PCH_OUT): $(PCH_SRC)
	@mkdir -p $(@D)
	@$(CC) $(CCFLAGS) $(INCLUDE_DIRS) $^ -o $@
	@echo "Recompiled PCH!"
	
out/%.o: src/%.cpp $(INCLUDES)
	@echo "Compiling $<..."
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -include $(PCH_SRC) -c $< -o $@

out/assets/%.vert.spv: %.vert
	@echo "Compiling $<..."
	@mkdir -p $(@D)
	glslc $< -o $@

out/assets/%.frag.spv: %.frag
	@echo "Compiling $<..."
	@mkdir -p $(@D)
	glslc $< -o $@

run: test-game
	@echo "[starting game]\n"
	DRI_PRIME=0 $(TEST_GAME_OUT)/$(ENGINE_NAME) Game.js
	@echo "\n[game stopped]"

watch-ts:
	@cd test-game && tsc --watch

test-game-scripts: $(TEST_GAME_SCRIPTS)
	@echo "Compiling game scripts..."
	@cd test-game && tsc

clean:
	rm -rf out/**
	rm -rf test-game-build
