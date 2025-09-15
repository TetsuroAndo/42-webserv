NAME = myYaml_test

SRCS = main.cpp ../MyYAML.cpp ../Node.cpp
OBJS = $(SRCS:.cpp=.o)

CXX = c++
#CXXFLAGS = -Wall -Wextra -Werror
#CXXFLAGS

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
