# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hirwatan <hirwatan@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/14 19:04:56 by teando            #+#    #+#              #
#    Updated: 2025/06/30 11:52:51 by hirwatan         ###   ########.fr        #
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
HTML_DIR = html

# ソースファイル
SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/server.cpp

# オブジェクトファイル
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# ヘッダーファイル
INCS = -I$(INC_DIR)

# ルール
all: $(HTML_DIR) $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) created!"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

$(HTML_DIR):
	@mkdir -p $(HTML_DIR)
	@echo "<!DOCTYPE html>\n<html>\n<head>\n  <title>WebServ Test Page</title>\n</head>\n<body>\n  <h1>Welcome to WebServ!</h1>\n  <p>This is a default test page.</p>\n</body>\n</html>" > $(HTML_DIR)/index.html
	@echo "HTML directory and default index.html created!"

clean:
	rm -rf $(OBJ_DIR)
	@echo "Object files removed!"

fclean: clean
	rm -f $(NAME)
	rm -rf $(HTML_DIR)
	@echo "$(NAME) and HTML files removed!"

re: fclean all

.PHONY: all clean fclean re