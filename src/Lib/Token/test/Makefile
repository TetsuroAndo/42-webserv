NAME = hash_test

SRCS = main.cpp ../GenHash.cpp
OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -I../

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

test: $(NAME)
	./$(NAME)
	$(MAKE) clean
