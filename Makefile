# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/25 13:31:17 by teando            #+#    #+#              #
#    Updated: 2025/09/06 16:01:37 by teando           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= webserv

SRC		:= $(shell find $(SRC_DIR) -name '*.cpp')

UNAME_S 		:= $(shell uname -s)
ifeq ($(UNAME_S),Darwin) # MacOS
	CONF			:= $(CONF_DIR)/default.conf
else # Linux
	CONF			:= $(CONF_DIR)/default.conf
endif

CXX			:= c++
CXXFLAG		:= -Wall -Wextra -Werror -std=c++98 -pedantic
OPT			:= -O3
RM			:= rm -rf
IDFLAG		:= -I$(INC_DIR)
DEFINE		:= -D_GLIBCXX_USE_CXX11_ABI=0

ROOT_DIR		:= .
SRC_DIR			:= $(ROOT_DIR)/src
INC_DIR			:= $(ROOT_DIR)/inc
OBJ_DIR			:= $(ROOT_DIR)/obj
CONF_DIR		:= $(ROOT_DIR)/config
CONF			:= $(CONF_DIR)/default.conf

OBJ		:= $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# =============== 42 RULES ==============

all:
	$(MAKE) $(NAME) -j $(shell nproc)

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

# =========== ORIGINAL RULES ============

# Build and run
run:
	./$(NAME) $(CONF)

# Aliases
c:
	$(RM) $(OBJ_DIR)
f: c
	$(RM) $(NAME)
r: f all

# Debug build
debug: OPT		:= -g -O1 -fno-omit-frame-pointer -fsanitize=address
debug: DEFINE	:= -DDEBUG_MODE=DEBUG_ALL
debug: fclean
	$(MAKE) $(NAME) -j $(shell nproc)

# ============= BUILD RULES =============

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAG) $(OPT) $(IDFLAG) $(LFLAG) $(DEFINE) -o  $@ $^
	@echo "====================="
	@echo "== Build Complete! =="
	@echo "====================="
	@echo "[Executable]: $(NAME)"
	@echo "[OS/Arch]: $(UNAME_S)"
	@echo "[Config]: $(CONF)"
	@echo "[Include]: $(INC_DIR)"
	@echo "[Compiler flags/CXXFLAG]: $(CXXFLAG)"
	@echo "[Linker flags/LFLAG]: $(LFLAG)"
	@echo "[Optimizer flags/OPT]: $(OPT)"
	@echo "[DEFINE]: $(DEFINE)"
	@echo "====================="

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAG) $(OPT) $(IDFLAG) $(DEFINE) -fPIC -MMD -MP  -c $< -o $@

# ================ MISC =================

nm:
	@nm $(OBJ) | grep ' U ' | awk '{print $$2}' | sort | uniq

nmbin:
	@nm $(NAME) | grep ' U ' | awk '{print $$2}' | sort | uniq

printsrc:
	@echo $(SRC) | tr ' ' '\n' | sort

printobj:
	@echo $(OBJ) | tr ' ' '\n' | sort

fill:
	./tools/fillEmptyDir.sh

view:
	./tools/rowCodeViewer.sh

help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all		Build all targets"
	@echo "  run		Run the program"
	@echo "  clean		Clean object files"
	@echo "  fclean		Fully clean (clean + remove executable)"
	@echo "  re			Rebuild (fclean + all)"
	@echo "  debug		Build with debug flags"
	@echo "  nm			List undefined symbols in object files"
	@echo "  nmbin		List undefined symbols in the executable"
	@echo "  printsrc	Print source files"
	@echo "  printobj	Print object files"
	@echo "  fill		Fill empty directories"
	@echo "  view		View source code"
	@echo "  help		Print this help message"

.PHONY:
