#******************************************************************************#
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: teando <teando@student.42tokyo.jp>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/13 00:02:21 by teando            #+#    #+#              #
#    Updated: 2025/01/13 00:04:55 by teando           ###   ########.fr        #
#                                                                              #
#******************************************************************************#

NAME	:= webserv
CC	:= c++
CFLAGS	:= -Wall -Wextra -Werror -std=c++98
ROOT_DIR	:= .
OUT_DIR	:= $(ROOT_DIR)/objs
INCS_DIR	:= $(ROOT_DIR)/inc
RM	:= rm -rf

IDFLAGS	:= -I$(INCS_DIR)

SRCS	:= \
	$(addprefix src/, \
		main.cpp \
		server.cpp \
	)

OBJS	:= $(addprefix $(OUT_DIR)/, $(SRCS:.cpp=.o))
DEPS	:= $(OBJS:.o=.d)

ifeq ($(DEBUG), 1)
	CFLAGS  += -g -fsanitize=address
else
	CFLAGS  += -O2
endif

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OUT_DIR)/%.o: $(ROOT_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP $(IDFLAGS) -c $< -o $@

clean:
	$(RM) $(OUT_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

norm:
	@norminette $(SRCS) $(INCS_DIR)

debug:
	$(MAKE) DEBUG=1

.PHONY: all clean fclean re norm debug

-include $(DEPS)
