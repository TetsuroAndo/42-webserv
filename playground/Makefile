# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/14 19:04:56 by teando            #+#    #+#              #
#    Updated: 2025/06/14 19:12:32 by teando           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

# コンパイラとフラグ
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# ディレクトリ
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj

# ソースファイル
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/server.cpp

# オブジェクトファイル
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# ヘッダーファイル
INCS = -I$(INC_DIR)

# ルール
all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) created!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "Object files removed!"

fclean: clean
	rm -f $(NAME)
	@echo "$(NAME) removed!"

re: fclean all

.PHONY: all clean fclean re
