MAKEFLAGS += --no-print-directory
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror 
TARGET = run_tests

SRCS = main.cpp \
	   HttpRequestTest.cpp ../HttpRequest.cpp \
	   HttpResponseTest.cpp ../HttpResponse.cpp \
	   HttpStatusTest.cpp ../HttpStatus.cpp \
	   ../../../Lib/StringOps/StringOps.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f *.o
fclean: clean
	@rm -f $(TARGET)

test: $(TARGET)
	@./$(TARGET)
	@$(MAKE) fclean
