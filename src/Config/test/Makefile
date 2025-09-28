NAME = config_test

SRCS = main.cpp ../Config.cpp ../../Lib/MyYAML/MyYAML.cpp ../../Lib/MyYAML/Node.cpp ../../Lib/StringOps/StringOps.cpp
OBJS = $(SRCS:.cpp=.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror

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
