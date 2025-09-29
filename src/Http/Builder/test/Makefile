MAKEFLAGS += --no-print-directory
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror

TARGET = run_tests
SRCS = main.cpp \
       ../ResponseBuilder.cpp \
       ../../Core/HttpResponse.cpp \
       ../../Core/HttpStatus.cpp \
       ../../../Lib/Time/TimeCache.cpp

OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -f $(TARGET)

test: $(TARGET)
	@./$(TARGET)
	@$(MAKE) fclean

