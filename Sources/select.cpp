#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#define SERVER_PORT 8080

int main (int ac, char **av)
{
	(void)ac;
	(void)av;
	int    current_sd, rc, flags, on = 1;
	int    listen_sd, max_sd, new_sd;
	int    desc_ready, end_server = false;
	int    close_conn;
	char   buffer[30000];
	struct sockaddr_in addr;
	struct timeval      timeout;
	fd_set              master_set, working_set;
	listen_sd = socket(AF_INET, SOCK_STREAM, 0); /* Create an AF_INET stream socket to receive incoming */
	if (listen_sd < 0)
	{
		//perror("socket() failed");
		exit(1);
	}
	rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); /* Allow socket descriptor to be reuseable */
	if (rc < 0)
	{
		//perror("setsockopt() failed");
		close(listen_sd);
		exit(1);
	}
	flags = fcntl(listen_sd, F_GETFL, 0); /* Set socket to be nonblocking. all sockets will inherit that state from the listening socket */
	fcntl(listen_sd, F_SETFL, flags | O_NONBLOCK);
	if (rc < 0)
	{
		//perror("fcntl() failed");
		close(listen_sd);
		exit(1);
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(SERVER_PORT);
	rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr)); /* Bind the socket */
	if (rc < 0)
	{
		//perror("bind() failed");
		close(listen_sd);
		exit(1);
	}
	rc = listen(listen_sd, 32); /* Set the listen back log to 32 pending connections */
	if (rc < 0)
	{
		//perror("listen() failed");
		close(listen_sd);
		exit(1);
	}
	FD_ZERO(&master_set); /* Initialize the master fd_set */
	max_sd = listen_sd;
	FD_SET(listen_sd, &master_set);
	/* Initialize the timeval struct to 3 minutes.  If no activity after 3 minutes this program will end */
	timeout.tv_sec  = 3 * 60;
	timeout.tv_usec = 0;
	/* Loop waiting for incoming connects or for incoming data on any of the connected sockets */
	while (end_server == false)
	{	
		memcpy(&working_set, &master_set, sizeof(master_set)); /* Copy the master fd_set over to the working fd_set */
		rc = select(max_sd + 1, &working_set, NULL, NULL, 0); /* Select the first READY file descriptor from the working fd_set */
		if (rc < 0) /* Check to see if the select call failed */
		{
			//perror("  select() failed");
			break ;
		}
		/* Check to see if the 3 minute time out expired. */
		if (rc == 0)
		{
			printf("select() timed out. End program.\n");
			exit(1);
		}
		/* One or more descriptors are readable.  Need to determine which ones they are. */
		desc_ready = rc;
		for (current_sd = 0; current_sd <= max_sd  &&  desc_ready > 0; ++current_sd)
		{
			if (FD_ISSET(current_sd, &working_set)) /* Check to see if this descriptor is ready */
			{
				desc_ready -= 1;
				if (current_sd == listen_sd) /* Check to see if this is the listening socket */
				{
					/* Accept all incoming connections */
					while (1)
					{
						new_sd = accept(listen_sd, NULL, NULL);
						if (new_sd < 0)
						{
							//if (errno != EWOULDBLOCK)
							//	end_server = true;
							break ;
						}
						FD_SET(new_sd, &master_set); /* Add the new incoming connection to the master read set */
						if (new_sd > max_sd)
							max_sd = new_sd;
					}
				}
				
				else /* This is not the listening socket, therefore an existing connection must be readable */
				{
					
					close_conn = false; /* Receive all incoming data on this socket */
					while (1)
					{
						rc = read(current_sd, buffer, 30000); /* Read data on this connection */
						//DEBUG TO SEE THE HEADER REQUEST//
						//printf("\x1B[32m[[DATA RECEIVED]]\x1B[0m\n\n%s", buffer);
						if (rc < 0)
						{
							close_conn = true;
							break ;
						}
						if (rc == 0) /* Check to see if the connection has been closed by the client */
						{
							close_conn = true;
							break ;
						}
						/* Echo the data to the client */
						const char *hello = "HTTP/1.1 200OK\r\n\r\n<p>Hello World!</p><script>let nb = 0; console.log(nb);</script>";
						rc = write(current_sd, hello, strlen(hello));
						if (rc < 0)
						{
							//perror("write failed:");
							break ;
						}
					}
					if (close_conn) /* If the close_conn flag was turned on, we need to clean up this active connection */
					{
						close(current_sd);
						FD_CLR(current_sd, &master_set);
						if (current_sd == max_sd)
						{
							while (FD_ISSET(max_sd, &master_set) == false)
								max_sd -= 1;
						}
					}
				}
			}
		}
	}
	for (current_sd = 0; current_sd <= max_sd; ++current_sd) /* Clean up all of the sockets that are open */
	{
		if (FD_ISSET(current_sd, &master_set))
			close(current_sd);
	}
}
