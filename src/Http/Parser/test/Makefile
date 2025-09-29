MAKEFLAGS += --no-print-directory
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror

TARGET = run_tests

SRCS = main.cpp \
       RequestBodyParserTest.cpp ../RequestBodyParser.cpp \
       RequestHeaderParserTest.cpp ../RequestHeaderParser.cpp \
       RequestLineParserTest.cpp ../RequestLineParser.cpp \
       ../../Core/HttpRequest.cpp \
       ../../Core/HttpStatus.cpp \
	   ../../../Lib/StringOps/StringOps.cpp \
	   ../../URI/URI.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

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
