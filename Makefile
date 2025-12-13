# NAME = webserv

# SRCS = ./sources/main.cpp \
# 	./sources/Server.cpp \
# 	./sources/utils.cpp \
# 	./sources/Client.cpp \
# 	./sources/Request/Request.cpp \
# 	./sources/Request/Request_init.cpp \
# 	./sources/Request/Request_error.cpp \
# 	./sources/GlobalConfig.cpp \
# 	./sources/Directive.cpp \
# 	./sources/ServerConfig.cpp \
# 	./sources/LocationConfig.cpp \
# 	./sources/Cgi.cpp

# INC_DIR = includes
# OBJS = $(SRCS:./sources/%.cpp=./objects/%.o)
# DEPS = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)

# CC = c++
# CFLAGS = -Wall -Wextra -Werror -ggdb -std=c++98 -I$(INC_DIR) -MMD -MP
# DEBUG_FLAG = -DDEBUG


# all: $(NAME)

# ./objects:
# 	mkdir -p ./objects

# ./objects/%.o: ./sources/%.cpp | ./objects
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(NAME): $(OBJS)
# 	$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

# -include $(DEPS)

# clean:
# 	@$(RM) -r ./objects

# fclean: clean
# 	@$(RM) $(NAME)

# re: fclean all

# debug:
# 	@$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAG)" re

# .PHONY: all clean fclean re debug


NAME = webserv

SRCS = ./sources/main.cpp \
	./sources/Core/Server.cpp \
	./sources/utils.cpp \
	./sources/Core/Client.cpp \
	./sources/Request/Request_action.cpp \
	./sources/Request/Request_conf.cpp \
	./sources/Request/Request_path.cpp \
	./sources/Request/Request_init.cpp \
	./sources/Request/Request_parse.cpp \
	./sources/Request/Request_error.cpp \
	./sources/Configuration/GlobalConfig.cpp \
	./sources/Configuration/Directive.cpp \
	./sources/Configuration/ServerConfig.cpp \
	./sources/Configuration/LocationConfig.cpp \
	./sources/Core/Cgi.cpp

INC_DIR = includes

OBJ_DIR = ./objects
OBJS = $(SRCS:./sources/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

CC = c++
CFLAGS = -Wall -Wextra -Werror -ggdb -std=c++98 -I$(INC_DIR) -MMD -MP
DEBUG_FLAG = -DDEBUG

all: $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Règle générique pour créer aussi les sous-dossiers dans objects
$(OBJ_DIR)/%.o: ./sources/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

-include $(DEPS)

clean:
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

debug:
	@$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAG)" re

.PHONY: all clean fclean re debug
