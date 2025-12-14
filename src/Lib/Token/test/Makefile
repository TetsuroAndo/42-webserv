NAME = hash_test
SPEED_NAME = speed_test

SRCS = main.cpp ../Token.cpp ../Random.cpp
SPEED_SRCS = speed.cpp ../Token.cpp ../Random.cpp
OBJS = $(SRCS:.cpp=.o)
SPEED_OBJS = $(SPEED_SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -I../

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(SPEED_NAME): $(SPEED_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SPEED_OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(SPEED_OBJS) $(NAME) $(SPEED_NAME)

test: $(NAME)
	bash -c "time ./$(NAME)"
	$(MAKE) clean

speed-test: $(SPEED_NAME)
	./$(SPEED_NAME)
	$(MAKE) clean
