NAME = cgi_module_test

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic

INCLUDES = -I../../../src
# -----------------------------------------------------------

SRCS = main.cpp
SRCS += DummyHttpReq.cpp
SRCS += ../CgiEnvironmentBuilder.cpp
SRCS += ../ParseCgiResponse.cpp
SRCS += ../../Http/Core/HttpRequest.cpp
SRCS += ../../Http/Core/HttpResponse.cpp
SRCS += ../../Http/Core/HttpStatus.cpp

SRCS += ../../Lib/StringOps/StringOps.cpp

# -----------------------------------------------------------

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Removing object files..."
	$(RM) $(OBJS) $(NAME)

re: clean all

test: re
	@echo "----------------------------------------"
	@echo "Running CGI Unit Tests..."
	@./$(NAME)
	@echo "----------------------------------------"
	$(MAKE) clean

.PHONY: all clean re test
