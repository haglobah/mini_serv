NAME := mini_serv

all : $(NAME)

$(NAME) : Makefile mini_serv.c
	cc -Wall -Wextra -Werror mini_serv.c -o $(NAME)
