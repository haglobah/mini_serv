#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct s_client
{
	int fd;
	int id;
	struct s_client *next;
}	t_client;

t_client 	*clients = NULL;
fd_set		current, readset, writeset;
int 		sockfd, g_id;
char 		msg[200000], buf[200040];

void eprint(char *msg)
{
	write(2, msg, strlen(msg));
}

void fatal()
{
	eprint("Fatal error\n");
	close(sockfd);
	exit(1);
}

int get_max_fd()
{
	int	maxfd = sockfd;
	t_client *tmp = clients;

	while(tmp)
	{
		if (tmp->fd > maxfd)
			maxfd = tmp->fd;
		tmp = tmp->next;	
	}
	return(maxfd);
}

int get_id(int fd)
{
	t_client *tmp = clients;

	while(tmp)
	{
		if (tmp->fd == fd)
			return (tmp->id);
		tmp = tmp->next;
	}
	return (-10);
}

void send_to_all(int from)
{
	t_client *tmp = clients;

	while (tmp)
	{
		if (tmp->fd != from && FD_ISSET(tmp->fd, &writeset))
		{
			if (send(tmp->fd, buf, strlen(buf), 0) == -1)
				fatal();
		}
		tmp = tmp->next;
	}
}

void add_client()
{
	t_client *tmp = clients;
	t_client *new;
	int clientfd;
	struct sockaddr_in 	clientaddr;
	socklen_t			len = sizeof(clientaddr);

	clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
	if (clientfd == -1)
		fatal();
	bzero(&buf, sizeof(buf));
	sprintf(buf, "server: client %d just arrived\n", g_id);
	send_to_all(clientfd);
	FD_SET(clientfd, &current);
	new = malloc(sizeof(t_client));
	if (!new)
		fatal();
	new->fd = clientfd;
	new->id = g_id++;
	new->next = NULL;
	if (!clients)
		clients = new;
	else
	{
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = tmp;
	}
}

void rm_client(int fd)
{
	t_client *to_del = NULL;
	t_client *tmp = clients;

	bzero(&buf, sizeof(buf));
	sprintf(buf, "server: client %d just left\n", get_id(fd));
	send_to_all(fd);
	if (clients && clients->fd == fd)
	{
		to_del = clients;
		clients = clients->next;
	}
	else
	{
		while (tmp && tmp->next && tmp->next->fd != fd)
			tmp = tmp->next;
		if (tmp && tmp->next && tmp->next->fd == fd)
		{
			to_del = tmp->next;
			tmp->next = tmp->next->next;
		}
	}
	if (to_del)
		free(to_del);
	FD_CLR(fd, &current);
	close(fd);
}

void extract_msg(int fd)
{
	char	tmp[20000];
	int		i = -1;
	int		j = -1;

	bzero(&tmp, sizeof(tmp));
	while (msg[++i])
	{
		tmp[++j] = msg[i];
		if (msg[i] == '\n')
		{
			bzero(&buf, sizeof(buf));
			sprintf(buf, "client %d: %s", get_id(fd), tmp);
			send_to_all(fd);
			bzero(&tmp, sizeof(tmp));
			j = -1;
		}
	}
	bzero(&msg, sizeof(msg));
}

int main (int ac, char **av) 	
{
	if (ac != 2)
	{
		eprint("Wrong number of arguments\n");
		exit(1);
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 

	// socket creation and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		fatal();
	if (bind(sockfd, (const struct sockadr *)&servaddr, sizeof(servaddr)) == -1)
		fatal();
	if (listen(sockfd, 100) == -1)
		fatal();

	FD_ZERO(&current);
	FD_SET(sockfd, &current);
	bzero(&msg, sizeof(msg));

	while (1)
	{
		readset = writeset = current;
		if (select(get_max_fd() + 1, &readset, &writeset, NULL, NULL) == -1)
			continue;
		for (int fd = 0; fd <= get_max_fd(); ++fd)
		{
			if (!FD_ISSET(fd, &readset))
			{
				if (fd == sockfd)
				{
					add_client();
					break;
				}
				int ret = 1;
				while (ret == 1 && msg[strlen(msg) - 1] != '\n')
				{
					ret = recv(fd, msg + strlen(msg), 1, 0);
					if (ret <= 0)
						break;
				}
				if (ret <= 0)
				{
					rm_client(fd);
					break;
				}
				extract_msg(fd);
			}
		}
	}
}