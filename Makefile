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
CFLAGS = -Wall -Wextra -Werror -ggdb -std=c++98 -I./includes -MMD -MP

all: $(NAME)

./objects:
	mkdir -p ./objects

./objects/%.o: ./sources/%.cpp | ./objects
	$(CC) $(CFLAGS) -I $(INC_DIR) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -I $(INC_DIR) -o $(NAME)

-include $(DEPS)

clean:
	@$(RM) -r ./objects

fclean: clean
	@$(RM) $(NAME)

re: fclean all

launch: re
	@echo "\nLaunching serv:"
	@./$(NAME)

.PHONY: all clean fclean re launch
