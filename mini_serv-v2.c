#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct s_client
{
	int				fd;
	int				id;
	struct s_client	*next;
}					t_client;

t_client	*clients = NULL;
fd_set		current, read_set, write_set;
int			sockfd, g_id;
char		msg[200000], buf[200040];

void	eprint(char *msg)
{
	write(2, msg, strlen(msg));
}

void	fatal()
{
	eprint("Fatal error\n");
	close(sockfd);
	exit(1);
}

int	get_max_fd()
{
	int			maxfd = sockfd;
	t_client	*tmp = clients;

	while (tmp)
	{
		if (tmp->fd > maxfd)
			maxfd = tmp->fd;
		tmp = tmp->next;
	}
	return (maxfd);
}

int	get_id(int fd)
{
	t_client	*tmp = clients;

	while (tmp)
	{
		if (tmp->fd == fd)
			return (tmp->id);
		tmp = tmp->next;
	}
	return (-10);
}

void	send_to_all(int from)
{
	t_client	*tmp = clients;

	while (tmp)
	{
		int to = tmp->fd;
		if (to != from && FD_ISSET(to, &write_set))
		{
			if (send(to, buf, strlen(buf), 0) == -1)
				fatal();
		}
		tmp = tmp->next;
	}
}

void	add_client()
{
	t_client			*tmp = clients;
	t_client			*new;
	int					client;
	struct sockaddr_in	clientaddr;
	socklen_t			len = sizeof(clientaddr);

	client = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
	if (client == -1)
		fatal();
	
	FD_ZERO(&buf);
	sprintf(buf, "server: client %d just arrived\n", g_id);
	send_to_all(client);
	FD_SET(client, &current);

	new = malloc(sizeof(t_client));
	if (new == NULL)
		fatal();
	new->fd = client;
	new->id = g_id++;
	new->next = NULL;

	if (clients == NULL)
	{
		clients = new;
	}
	else
	{
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new;
	}
}

void	rm_client(int fd)
{
	t_client	*to_del = NULL;
	t_client	*tmp = clients;

	FD_ZERO(&buf);
	sprintf(buf, "server: client %d just left\n", get_id(fd));
	send_to_all(fd);
	FD_CLR(fd, &current);

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
	close(fd);
}

void	extract_msg(int fd)
{
	char	tmp[200000];
	int		i = -1;
	int		j = -1;

	FD_ZERO(&tmp);
	while (msg[++i])
	{
		tmp[++j] = msg[i];
		if (msg[i] == '\n')
		{
			FD_ZERO(&buf);
			sprintf(buf, "client %d: %s", get_id(fd), tmp);
			send_to_all(fd);
			FD_ZERO(&tmp);

			j = -1;
		}
	}
	FD_ZERO(&msg);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		eprint("Wrong number of arguments\n");
		exit(1);
	}

	struct sockaddr_in	servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1]));

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		fatal();
	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
		fatal();
	if (listen(sockfd, 100) == -1)
		fatal();

	FD_ZERO(&current);
	FD_SET(sockfd, &current);
	FD_ZERO(&msg);

	while (1)
	{
		read_set = write_set = current;
		if (select(get_max_fd() + 1, &read_set, &write_set, NULL, NULL) == -1)
			continue ;
		for (int fd = 0; fd <= get_max_fd(); ++fd)
		{
			if (FD_ISSET(fd, &read_set))
			{
				//add
				if (fd == sockfd)
				{
					add_client();
					break ;
				}
				//recv
				int ret = 1;
				while (ret == 1 && msg[strlen(msg) - 1] != '\n')
				{
					ret = recv(fd, msg + strlen(msg), 1, 0);
					if (ret <= 0)
						break ;
				}
				//rm
				if (ret <= 0)
				{
					rm_client(fd);
					break ;
				}
				extract_msg(fd);
			}
		}
	}
	return (0);
}