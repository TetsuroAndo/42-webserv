NAME = logger_test

SRCS = main.cpp \
	   ../ConsoleSink.cpp \
	   ../FileSink.cpp \
	   ../LogBuilder.cpp \
	   ../Logger.cpp
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
	./$(NAME)
	$(MAKE) clean
