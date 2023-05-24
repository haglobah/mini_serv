NAME := mini_serv

SRCS := main.c

all : $(NAME)

$(NAME) : Makefile $(SRCS)
	cc -Wall -Wextra -Werror $(SRCS) -o $(NAME)

run : 
	$(MAKE) all
	./$(NAME) 8080