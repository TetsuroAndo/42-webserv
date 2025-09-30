NAME = log_test.out

SRCS = $(shell find ../ -name "*.cpp") \
	   $(shell find ../../../Http/Core -name "*.cpp") \
	   $(shell find ../../StringOps -name "StringOps.cpp") \

OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -I../

all: $(NAME) logdir

logdir:
	mkdir -p test_log

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

test: $(NAME) logdir
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME)
	$(MAKE) clean

printsrc:
	@echo $(SRCS)
