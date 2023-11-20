
S_NAME = webserv

C_NAME = client

SERVER = srcs/WebServer.cpp server.cpp srcs/http/HttpRequest.cpp srcs/http/ParseRequest.cpp \
			srcs/http/BodyRead.cpp \
			srcs/http/StaticHandler.cpp \
			srcs/http/CGIHandler.cpp srcs/Config/Config.cpp\
			srcs/Config/Config_directives.cpp srcs/Config/Config_exceptions.cpp \
			srcs/Config/Config_tools.cpp \
			libraries/sServ.cpp libraries/sRequest.cpp \
			libraries/sLocation.cpp libraries/sHttpMethods.cpp \
			srcs/utils.cpp \
			srcs/http/AutoIndex.cpp

CLIENT = srcs/Client.cpp cli.cpp

S_OBJS=${SERVER:.cpp=.o}

C_OBJS=${CLIENT:.cpp=.o}

CLASS_DIR = srcs/

$(S_NAME): $(S_OBJS) $()
	@echo "building server in ./"$(S_NAME)
	@c++ -g -Wall -Wextra -Werror -std=c++98 $(S_OBJS) -o $(S_NAME)

$(C_NAME): $(C_OBJS) $()
	@echo "building client in ./"$(C_NAME)
	@c++ -g -Wall -Wextra -Werror -std=c++98 $(C_OBJS) -o $(C_NAME)

all: $(S_NAME) $(C_NAME)

re: fclean $(S_NAME)

clean:
	@echo "removing all"
	@rm -f ${S_NAME} $(C_NAME) *.o */*.o */*/*.o */*/*/*.o

fclean: clean

sanitize: $(S_OBJS) $()
	c++ -g -fsanitize=address -Wall -Wextra -Werror -std=c++98 ${S_OBJS} -o $(S_NAME)

%.o: %.cpp
	@c++ -Wall -Wextra -Werror -std=c++98 -c $< -o $@

.PHONY: all re clean fclean .SET_UP_TESTING

# Colors
RESET					:= "\033[0m"
BLACK					:= "\033[30m"
RED						:= "\033[31m"
GREEN					:= "\033[32m"
YELLOW					:= "\033[33m"
BLUE					:= "\033[34m"
MAGENTA					:= "\033[35m"
CYAN					:= "\033[36m"
WHITE					:= "\033[37m"
BOLDBLACK				:= "\033[1m\033[30m"
BOLDRED					:= "\033[1m\033[31m"
BOLDGREEN				:= "\033[1m\033[32m"
BOLDYELLOW				:= "\033[1m\033[33m"
BOLDBLUE				:= "\033[1m\033[34m"
BOLDMAGENTA				:= "\033[1m\033[35m"
BOLDCYAN				:= "\033[1m\033[36m"
BOLDWHITE				:= "\033[1m\033[37m"
