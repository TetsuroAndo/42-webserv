# HERE TO PROJECT NAME
NAME		:= Base64Test

CXX		 := c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -pedantic
OPT		 := -O2
RM		  := rm -rf
DEFINE	  := -DDEBUG_MODE=DEBUG_NONE

ROOT_DIR	:= .
SRC_DIR	 := $(ROOT_DIR)
INC_DIR	 := ../
OBJ_DIR	 := $(ROOT_DIR)/obj

IDFLAGS	 := -I$(INC_DIR)

# HERE TO SRC FILES
SRC :=	Base64Test.cpp \
		../Base64.cpp
	

OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

all:
	$(MAKE) __build -j $(shell nproc)

re:
	$(MAKE) fclean
	$(MAKE) __build -j $(shell nproc)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

__build: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJ) $(IDFLAGS) $(DEFINE) -o $(NAME)
	@echo "====================="
	@echo "== Build Complete! =="
	@echo "====================="

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OPT) $(IDFLAGS) $(DEFINE) -MMD -MP -c $< -o $@

# ============ DEBUG BUILDS ============
debug:
	$(MAKE) __debug -j $(shell nproc)

__debug: OPT	:= -g -fsanitize=address -O1 -fno-omit-frame-pointer
__debug: DEFINE := -DDEBUG_MODE=DEBUG_ALL
__debug: $(NAME)

# ============ CHECKS ============
valgrind: $(NAME)
	valgrind -q --leak-check=full --track-origins=yes ./$(NAME) $(ARGS)

forbidden:
	@echo "Checking for forbidden functions (malloc, free, printf)..."
	@find $(SRC_DIR) -type f \( -name "*.cpp" -o -name "*.h*" \) -exec grep -H --color -E '\b(malloc|calloc|realloc|free|printf|using|namespace|friend)\b' {} \; || true

check: forbidden valgrind

.PHONY: all clean fclean re debug valgrind forbidden check