COMPILER := g++
CFLAGS := -O3 -std=c++17 -Werror -Wall

SRC_DIR := ./src/
OBJ_DIR := ./obj/
BIN_DIR := ./bin/
TMP_DIR := ./tmp/

vpath %.cpp $(SRC_DIR)
vpath %.h $(SRC_DIR)
vpath %.o $(OBJ_DIR)
vpath %.d $(TMP_DIR)
vpath % $(BIN_DIR)

SRC := $(notdir $(wildcard $(SRC_DIR)*.cpp))
OBJ := $(patsubst %.cpp, %.o, $(SRC))
D_FILES := $(addprefix $(TMP_DIR), $(patsubst %.cpp, %.d, $(SRC)))

RESULT := search_server

.PHONY : all
all: $(RESULT)

$(RESULT): $(OBJ) | $(BIN_DIR)
	$(COMPILER) $(CFLAGS) $(addprefix $(OBJ_DIR), $(notdir $^)) -o $(addprefix $|, $@)

$(OBJ): %.o: $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(COMPILER) $(CFLAGS) -c $< -o $(addprefix $(OBJ_DIR), $@)

include $(D_FILES)

$(D_FILES): $(TMP_DIR)%.d: $(SRC_DIR)%.cpp | $(TMP_DIR)
	$(COMPILER) -MM $< -MF $@

$(BIN_DIR) :
	-mkdir $@
$(OBJ_DIR) :
	-mkdir $@
$(TMP_DIR) :
	-mkdir $@
.PHONY: clean
clean:
	-rm -r $(OBJ_DIR) $(BIN_DIR) $(TMP_DIR)
