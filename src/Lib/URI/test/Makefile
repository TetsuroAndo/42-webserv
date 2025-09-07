NAME = uri_test

SRCS = main.cpp ../decodeURI.cpp ../encodeURI.cpp ../decodeURIComponent.cpp ../encodeURIComponent.cpp
OBJS = $(SRCS:.cpp=.o)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(NAME)

test: $(NAME)
	node test.js
	$(MAKE) clean
