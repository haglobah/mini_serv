# My version of mini_serv

Built upon nenieiri's version ([here](https://github.com/nenieiri-42Yerevan/Mini_Serv_Exam_Rank_06)), with some hacks and improvements of mine.

Good to know:
- It uses a linked list as data structure for managing the clients.
- The Mac version (depends on the server OS your exams are run on) uses `FD_ZERO(&lala)` as an abbreviation of `bzero(&lala, sizeof(lala))`.
- The newlines in the functions are intentional for better chunking.

## Setup

`make run` compiles `mini_serv.c` and starts the server. The server, although, isn't supposed to do anything on its own.
All it's doing is broadcasting incoming messages to all clients that are already connected. So, to test whether everything's working:

1. Open 3 terminal windows.
2. In one of them, run `make run`.
3. In the other (at least) two, run `nc 127.0.0.1 8080`. 

You should see something similar to this:

<table>
<tr>
<td>

```bash
$ make run
/Applications/Xcode.app/Contents/Developer/usr/bin/make all
cc -Wall -Wextra -Werror mini_serv.c -o mini_serv
./mini_serv 8080

```

</td>
<td>
    
```bash
$ nc 127.0.0.1 8080
server: client 1 just arrived
lala
client 1: hohoho
^C
$ 
```
</td>
<td>
    
```bash
$ nc 127.0.0.1 8080
client 0: lala
hohoho
server: client 0 just left

```
</td>
</tr>
</table>

---

# Subject<br>
Assignment name  : mini_serv<br>
Expected files   : mini_serv.c<br>
Allowed functions: write, close, select, socket, accept, listen, send, recv, bind, strstr, malloc, realloc, free, calloc, bzero, atoi, sprintf, strlen, exit, strcpy, strcat, memset<br>

--------------------------------------------------------------------------------

Write a program that will listen for client to connect on a certain port on 127.0.0.1 and will let clients to speak with each other

This program will take as first argument the port to bind to<br>
If no argument is given, it should write in stderr "Wrong number of arguments" followed by a \n and exit with status 1<br>
If a System Calls returns an error before the program start accepting connection, it should write in stderr "Fatal error" followed by a \n and exit with status 1<br>
If you cant allocate memory it should write in stderr "Fatal error" followed by a \n and exit with status 1<br>
<br>
Your program must be non-blocking but client can be lazy and if they don't read your message you must NOT disconnect them...<br>
<br>
Your program must not contains #define preproc<br>
Your program must only listen to 127.0.0.1<br>
The fd that you will receive will already be set to make 'recv' or 'send' to block if select hasn't be called before calling them, but will not block otherwise.<br>
<br>
When a client connect to the server:<br>
- the client will be given an id. the first client will receive the id 0 and each new client will received the last client id + 1<br>
- %d will be replace by this number<br>
- a message is sent to all the client that was connected to the server: "server: client %d just arrived\n"<br>
<br>
Clients must be able to send messages to your program.<br>
- message will only be printable characters, no need to check<br>
- a single message can contains multiple \n<br>
- when the server receive a message, it must resend it to all the other client with "client %d: " before every line!<br>
<br>
When a client disconnect from the server:<br>
- a message is sent to all the client that was connected to the server: "server: client %d just left\n"<br>
<br>
Memory or fd leaks are forbidden<br>
<br>
To help you, you will find the file main.c with the beginning of a server and maybe some useful functions. (Beware this file use forbidden functions or write things that must not be there in your final program)<br>
<br>
Warning our tester is expecting that you send the messages as fast as you can. Don't do un-necessary buffer.<br>
<br>
Evaluation can be a bit longer than usual...<br>
<br>
Hint: you can use nc to test your program<br>
Hint: you should use nc to test your program<br>
Hint: To test you can use fcntl(fd, F_SETFL, O_NONBLOCK) but use select and NEVER check EAGAIN (man 2 send)<br>
