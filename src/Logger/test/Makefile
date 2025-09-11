NAME = logger_test

SRCS = $(shell find ../ -name "*.cpp")

OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -I../

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

test: $(NAME)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME)
	$(MAKE) clean

printsrc:
	@echo $(SRCS)
