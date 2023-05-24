NAME := mini_serv

all : $(NAME)

$(NAME) : Makefile mini_serv.c
	cc -Wall -Wextra -Werror mini_serv.c -o $(NAME)

run : 
	$(MAKE) all
	./$(NAME) 8080