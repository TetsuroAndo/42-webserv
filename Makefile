# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/25 13:31:17 by teando            #+#    #+#              #
#    Updated: 2025/06/05 04:01:19 by teando           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:= webserv
CC			:= c++
CFLAGS		:= -Wall -Wextra -Werror -std=c++98
RM			:= rm -rf

# Project PATH
ROOT_DIR		:= .
SRC_DIR			:= $(ROOT_DIR)/src
INC_DIR			:= $(ROOT_DIR)/inc
OBJ_DIR			:= $(ROOT_DIR)/obj
CONF_DIR		:= $(ROOT_DIR)/config
CONF			:= $(CONF_DIR)/default.conf

UNAME_S 		:= $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CONF			:= $(CONF_DIR)/default.conf
else
	CONF			:= $(CONF_DIR)/default.conf
endif

# FLAGS
IDFLAGS		:= -I$(INC_DIR)

SRC		:= $(shell find $(SRC_DIR) -name '*.cpp')
OBJ		:= $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

# =======================
# == Targets ============
# =======================
all:
	$(MAKE) __build -j $(shell nproc)

run:
	./$(NAME) $(CONF)

c:
	$(RM) $(OBJ_DIR)
f: c
	$(RM) $(NAME)
r: f all

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

# =======================
# ==== Debug Targets ====
# =======================

debug:
	$(MAKE) __debug -j $(shell nproc)

__debug: CFLAGS		+= -g -fsanitize=address -O1 -fno-omit-frame-pointer
__debug: DEFINE		+= -DDEBUG_MODE=DEBUG_ALL
__debug: $(NAME)

# =======================
# ==== Build Targets ====
# =======================
__build: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(IDFLAGS) $(LFLAGS) $(DEFINE) -o $(NAME)
	@echo "====================="
	@echo "== Build Complete! =="
	@echo "====================="
	@echo "[Executable]: $(NAME)"
	@echo "[UNAME_S]: $(UNAME_S)"
	@echo "[Config]: $(CONF)"
	@echo "[IncludeDir]: $(INC_DIR)"
	@echo "[Compiler flags/CFLAGS]: $(CFLAGS)"
	@echo "[Linker flags/LFLAGS]: $(LFLAGS)"
	@echo "[DEFINE]: $(DEFINE)"
	@echo "====================="

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(IDFLAGS) $(DEFINE) -fPIC -MMD -MP  -c $< -o $@

# =======================
# == Dev Tool Targets ===
# =======================

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

.PHONY: all clean fclean re help
