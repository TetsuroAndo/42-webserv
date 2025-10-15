NAME		:= parse_cookie_field_test

CXX			:= c++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++98 -pedantic
OPT			:= -O2

# Build in this dir
SRC			:= parse_cookie_field_test.cpp
OBJ			:= $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OPT) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OPT) -DUNIT_TEST -c $< -o $@

run: $(NAME)
	./$(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re

