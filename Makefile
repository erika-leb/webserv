NAME = webserv

SRCS = ./sources/main.cpp \
	./sources/Server.cpp \
	./sources/utils.cpp \
	./sources/Client.cpp \
	./sources/Request.cpp

INC_DIR = includes
OBJS = $(SRCS:./sources/%.cpp=./objects/%.o)
DEPS = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)

CC = c++
CFLAGS = -Wall -Wextra -Werror -ggdb -std=c++98 -I$(INC_DIR) -MMD -MP
DEBUG_FLAG = -DDEBUG


all: $(NAME)

./objects:
	mkdir -p ./objects

./objects/%.o: ./sources/%.cpp | ./objects
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(NAME)

-include $(DEPS)

clean:
	@$(RM) -r ./objects

fclean: clean
	@$(RM) $(NAME)

re: fclean all

debug:
	@$(MAKE) CFLAGS="$(CFLAGS) $(DEBUG_FLAG)" re

.PHONY: all clean fclean re debug
