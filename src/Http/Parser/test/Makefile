MAKEFLAGS += --no-print-directory
CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror

TARGET = run_tests

SRCS = main.cpp \
       RequestBodyParserTest.cpp ../RequestBodyParser.cpp \
       RequestHeaderParserTest.cpp ../RequestHeadParser.cpp \
       RequestLineParserTest.cpp ../RequestLineParser.cpp \
       ../../Core/HttpRequest.cpp \
       ../../Core/HttpStatus.cpp \
	   ../../../Lib/StringOps/StringOps.cpp \
	   ../../URI/URI.cpp \
	   ../../../Config/Config.cpp \
	   ../../../Config/ConfigBuilder.cpp \
	   ../../../Config/ConfigParser.cpp \
	   ../../../Config/ConfigLocationParser.cpp \
	   ../../../Config/ConfigLogParser.cpp \
	   ../../../Lib/MyYAML/MyYAML.cpp \
	   ../../../Lib/MyYAML/Node.cpp \
	   ../../../Lib/Logger/ErrorLog/Logger.cpp \
	   ../../../Lib/Logger/ErrorLog/LogBuilder.cpp \
	   ../../../Lib/Logger/AccessLog/AccessLogger.cpp \
	   ../../../Lib/Logger/Sink/ConsoleSink.cpp \
	   ../../../Lib/Logger/Sink/FileSink.cpp \
	   ../../../Lib/Logger/Sink/LogSink.cpp \
	   ../../../Lib/Logger/Form/LogForm.cpp \
	   ../../../Lib/Logger/Form/ElfForm.cpp \
	   ../../../Lib/Logger/Form/JsonForm.cpp \
	   ../../../Lib/Time/TimeCache.cpp \
	   ../../../Lib/Time/TimeFormatter.cpp \
	   ../../Core/HttpResponse.cpp

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
