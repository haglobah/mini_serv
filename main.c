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

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main() {
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created..\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n");
	if (listen(sockfd, 10) != 0) {
		printf("cannot listen\n"); 
		exit(0); 
	}
	len = sizeof(cli);
	connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
	if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n");
}