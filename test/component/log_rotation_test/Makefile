NAME = log_rotation_test_app

SRCS = main.cpp \
       ../../src/Config/Config.cpp \
       ../../src/Config/ConfigBuilder.cpp \
       ../../src/Config/ConfigParser.cpp \
       ../../src/Config/ConfigLocationParser.cpp \
       ../../src/Config/ConfigLogParser.cpp \
       ../../src/Lib/MyYAML/MyYAML.cpp \
       ../../src/Lib/MyYAML/Node.cpp \
       ../../src/Lib/StringOps/StringOps.cpp \
       ../../src/Lib/Logger/AccessLog/AccessLogger.cpp \
       ../../src/Lib/Logger/Sink/FileSink.cpp \
       ../../src/Lib/Logger/Sink/LogSink.cpp \
       ../../src/Lib/Logger/Sink/ConsoleSink.cpp \
       ../../src/Lib/Logger/Form/ElfForm.cpp \
       ../../src/Lib/Logger/Form/JsonForm.cpp \
       ../../src/Lib/Logger/Form/LogForm.cpp \
       ../../src/Lib/Logger/ErrorLog/Logger.cpp \
       ../../src/Lib/Logger/ErrorLog/LogBuilder.cpp \
       ../../src/Http/Core/HttpRequest.cpp \
       ../../src/Http/Core/HttpResponse.cpp \
       ../../src/Http/Core/HttpStatus.cpp \
       ../../src/Lib/Time/TimeCache.cpp \
       ../../src/Lib/Time/TimeFormatter.cpp

OBJS = $(SRCS:.cpp=.o)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I../../src

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
