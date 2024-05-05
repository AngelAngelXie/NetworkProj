#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>

#define SERVER "127.0.0.1"
#define SERVER_PORT 5000

#define MAXMSG 1400
#define OP_SIZE 20

#define SEQ0 0
#define SEQ1 1

#define REGISTER "REGISTER"
#define UPDATE "UPDATE"
#define QUERY "QUERY"
#define RESPONSE "RESPONSE"
#define FINISH "FINISH"
#define ACK "ACK"
#define GET "GET"
#define EXIT "EXIT"

#define TIMEOUT 500000		/* 1000 ms */


/* This structure can be used to pass arguments */
struct ip_port {
	unsigned int ip;
	unsigned short port;
};

struct id_node {
	unsigned int ID;
	unsigned int ip;
	unsigned short port;
	struct id_node *next;
};
int client_cnt = -1;


int set_timeout(int sockfd, int usec) {
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usec; /* 100 ms */
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
			(struct timeval *)&tv, sizeof(struct timeval));
	if (ret == SO_ERROR) {
		return -1;
	}
	return 0;
}


int unset_timeout(int sockfd) {
	return set_timeout(sockfd, 0);
}


int rdt3_send(int sockfd, struct sockaddr_in servaddr, char ack_num, char *buffer, unsigned len) {
	int waiting = 1;
	char noack_num = ack_num;
	struct sockaddr_in recv_addr;
	unsigned int recv_len;
	char recv_buf[MAXMSG];

	char seq;
	char op[OP_SIZE];
	char remain[MAXMSG];

	sendto(sockfd, (const char *)buffer, len,
		0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

	set_timeout(sockfd, TIMEOUT);

	int parse_idx = 0;

	while (waiting) {

		/***********************************************
		 * You should receive and parse the response here.
		 * Then, you should check whether it is the ACK
		 * packet you are waiting for by comparing the
		 * sequence number and checking packet type.
		 * Waiting until receiving the right ACK.
		 *
		 * START YOUR CODE HERE
		 **********************************************/

		ssize_t num_bytes = recvfrom(sockfd, recv_buf, MAXMSG, 0, (struct sockaddr *)&recv_addr, &recv_len);
    	if (num_bytes < 0) {
			sendto(sockfd, (const char *)buffer, len,
				0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
		}else{
			// parse the ACK response
			recv_buf[num_bytes]='\0';
			seq = recv_buf[0];
			// ACK starts at the 3rd idx
			if(seq == noack_num && strncmp(recv_buf+2, ACK, strlen(ACK)) == 0){
				// if ACK sequence number matches and pkt type is ACK, we complete the process - stop waiting 
				waiting = 0;
			}
		}


		/***********************************************
		 * END OF YOUR CODE
		 **********************************************/

		bzero(recv_buf, MAXMSG);
	}

	unset_timeout(sockfd);

	return 0;
}


/* Send query to the server */
int send_register(int sockfd, struct sockaddr_in servaddr, unsigned int ip, unsigned short port) {
	char buffer[MAXMSG];
	
	bzero(buffer, MAXMSG);

	char seq = SEQ1;

	/* Compose send buffer: REGISTER IP Port */
	int total_len = 0;

	memcpy(buffer, &seq, sizeof(seq));
	total_len ++; /* add a seq */

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, REGISTER, strlen(REGISTER));
	total_len += strlen(REGISTER);

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, &ip, sizeof(ip));
	total_len += sizeof(ip);

	memcpy(buffer + total_len, &port, sizeof(port));
	total_len += sizeof(port);

	buffer[total_len] = '\0';

	rdt3_send(sockfd, servaddr, seq, buffer, total_len);

	printf("REGISTER finished !\n");
	
	return 0;
}


/* Send query to the server */
int send_update(int sockfd, struct sockaddr_in servaddr, unsigned int ip, unsigned short port, unsigned file_map) {
	char buffer[MAXMSG];
	
	bzero(buffer, MAXMSG);

	char seq = SEQ1;

	/***********************************************
	 * You should follw the send_register pattern to
	 * implement a function to send UPDATE message.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	// compose send buffer: seq, space, UPDATE, space, IP, Port, bitmap
	int total_len = 0;

	memcpy(buffer, &seq, sizeof(seq));
	total_len ++; /* add a seq */

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, UPDATE, strlen(UPDATE));
	total_len += strlen(UPDATE);

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, &ip, sizeof(ip)); // encode its own ip number to be sent to server
	total_len += sizeof(ip);

	memcpy(buffer + total_len, &port, sizeof(port)); // encode its own port number to be sent to server
	total_len += sizeof(port);

	memcpy(buffer+ total_len, &file_map, sizeof(file_map));
	total_len += sizeof(file_map);

	buffer[total_len] = '\0';

	// sends out the composed buffer
	rdt3_send(sockfd, servaddr, seq, buffer, total_len);

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/
	
	printf("UPDATE finished !\n");

	return 0;
}


// =====================BONUS CODE========================
// Initialize an empty linked list
struct id_node* init_id_node_list() {
    return NULL; // Return NULL indicating an empty list
}

void insert_id_node(struct id_node **head, unsigned ip, unsigned short port, unsigned ID){
	struct id_node * newnode = (struct id_node*)malloc(sizeof(struct id_node));
	if(newnode == NULL){
		printf("Memory allocation failed.\n");
		return;
	}
	newnode->ip = ip;
	newnode->port = port;
	newnode->ID = ID;
    newnode->next = NULL;

	// If the list is empty, set the new node as the head
    if (*head == NULL) {
		printf("inserted first node...\n");
        *head = newnode;
        return;
    }

	// Traverse the list to find the last node
    struct id_node* current = *head;
    while (current->next != NULL) {
		printf("searching last node\n");
        current = current->next;
    }

    current->next = newnode;
}

int find_id_node(struct id_node **head, unsigned ip, unsigned short port){
	struct id_node* current = *head;
	while(current != NULL){
		printf("looking for id_node\n");
		if(current->ip == ip && current->port == port){
			return current->ID;
		}
		current = current->next;
	}
	return -15;
}

int query_id_node(struct id_node **head, unsigned ID, unsigned * ip, unsigned short * port){

	struct id_node* current = *head;
	while(current != NULL){
		if(current->ID == ID){
			*ip = current->ip;
			*port = current->port;
			return 1;
		}
		current = current->next;
	}

	return -1;
}

// // helper:
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard characters until newline or EOF */ }
}

// =======================================================

int receive_query(int sockfd, struct sockaddr_in servaddr, struct id_node** head) {
	struct sockaddr_in recv_addr;
	unsigned int recv_len;
	char buffer[MAXMSG];

	int n = 0;
	unsigned parse_idx = 0;
	char seq;

	char send_buf[MAXMSG];
	unsigned send_idx = 0;

	unset_timeout(sockfd);

	printf("Receiving query ...\n");

	char unfinished = 1;
	while (unfinished) {

		n = recvfrom(sockfd, (char *)buffer, MAXMSG,
				MSG_WAITALL, ( struct sockaddr *) &recv_addr, &recv_len);
		if (n < 0) {
			continue;
		}

		buffer[n] = '\0';

		seq = buffer[0];
		parse_idx += 2; /* skip seq and blank */

		/* If receive FINISH signal, stop receiving. */
		if (strncmp(buffer + parse_idx, FINISH, strlen(FINISH)) == 0) {
			unfinished = 0;

		/* Receive RESPONSE */
		} else if (strncmp(buffer + parse_idx, RESPONSE, strlen(RESPONSE)) == 0) {
			// Receive and parse packet
			unsigned int ip;
			unsigned short port;

			parse_idx += strlen(RESPONSE);
			parse_idx ++; /*skip blank */

			memcpy(&ip, buffer + parse_idx, sizeof(ip));
			parse_idx += sizeof(ip);

			memcpy(&port, buffer + parse_idx, sizeof(port));
			parse_idx += sizeof(port);

			struct in_addr addr;
			addr.s_addr = ip;
			char *ip_str = inet_ntoa(addr);

			// BONUS code
			// store the ip_str and port into the dictionary with corresponding unique ID
			// check if has already existed
			int ID = find_id_node(head, ip, port);
			// if the node does not exists, add the node in
			if(ID == -15){
				// assign a new ID to the new client. 
				client_cnt++;
				ID = client_cnt;
				// add client to id_node list
				insert_id_node(head, ip, port, ID);
			}
			printf("%s : %d : %u\n", ip_str, port, ID);

		} else {
			printf("Unknown operation: %s\n", buffer + 2);
			goto nooperation;
		}


		// Compose and send ACK packet
		memcpy(send_buf, &seq, sizeof(seq));
		send_idx += 2; /* seq and blank */

		memcpy(send_buf + send_idx, ACK, strlen(ACK));
		send_idx += strlen(ACK);

		int res = sendto(sockfd, (const char *)send_buf, send_idx,
				0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

		if (res < 0) { 
		}

	nooperation:
		bzero(buffer, MAXMSG);
		bzero(send_buf, MAXMSG);
		parse_idx = 0;
		send_idx = 0;
	}

	return 0;
}


/* Send query to the server */
int send_query(int sockfd, struct sockaddr_in servaddr, char *filename, int len, struct id_node ** head) {
	char buffer[MAXMSG];

	bzero(buffer, sizeof(buffer));

	char seq = SEQ1;

	/* Compose send buffer: REGISTER IP Port */
	int total_len = 0;

	memcpy(buffer, &seq, sizeof(seq));
	total_len ++; /* add a seq */

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, QUERY, strlen(QUERY));
	total_len += strlen(QUERY);

	buffer[total_len] = ' ';
	total_len ++; /* add a blank */

	memcpy(buffer + total_len, filename, len);
	total_len += len;

	buffer[total_len] = '\0';

	rdt3_send(sockfd, servaddr, seq, buffer, total_len);

	printf("QUERY finished !\n");

	/*sleep(1); [> begin to receive queried messages <]*/
	
	receive_query(sockfd, servaddr, head);

	return 0;
}


unsigned int get_file_map() {
	DIR *dir;
    struct dirent *entry;
	unsigned int file_map = 0U;

    // Open the current directory
    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // Enumerate files in the directory
    while ((entry = readdir(dir)) != NULL) {
		unsigned int bit = (1U << 31);

		char file_idx = (entry->d_name[0] - '0') * 10 + (entry->d_name[1] - '0');
		if (file_idx < 0 || file_idx > 31) {
			continue;
		}


		file_map |= (bit >> file_idx);
    }

    // Close the directory
    closedir(dir);
	return file_map;
}


// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size) {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}


// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}


void* p2p_server(void* arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAXMSG] = {0};
    FILE *fp;
    ssize_t bytes_read;

	struct ip_port *ip_port_info = (struct ip_port *) arg;

	int fd_count = 0;
	int fd_size = 100; /* at most 100 sockets */
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    //     perror("setsockopt error");
    //     exit(EXIT_FAILURE);
    // }

    // Bind socket to address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(ip_port_info->port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("listen fails to start");
        exit(EXIT_FAILURE);
    }

	printf("p2p server is listening ...\n");

	/***********************************************
	 * Initialize the pfds here.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	pfds[0].fd = server_fd;   // add server socket as the first pollfd in pfds
	pfds[0].events = POLLIN;  // Monitor for incoming connections: ready to read or not
	fd_count = 1;             // add 1 to indicate there is 1 file descriptor being monitored

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/
 
	while (1) {
		int poll_count = poll(pfds, fd_count, -1);

		if (poll_count == -1) {
			perror("poll error");
			exit(1);
		}

		// Run through the existing connections looking for data to read
		for(int i = 0; i < fd_count; i++) {
			// Check if someone's ready to read
			if (pfds[i].revents & POLLIN) {
				if (pfds[i].fd == server_fd) { /* the server receives a connection request */
					/***********************************************
					 * Add your code here to receive a new connection
					 *
					 * START YOUR CODE HERE
					 **********************************************/

					// accepts the client socket
					struct sockaddr_in client_addr;
					socklen_t client_addrlen = sizeof(client_addr);
					int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen);
					// if something's wrong, report error and skip to the next connection
					if (client_socket == -1) {
						perror("accept");
						continue;
					}
					// add the accepted client socket to the pfds
					add_to_pfds(&pfds, client_socket, &fd_count, &fd_size);
					// add_to_pfds will automatically increment fd_count and resize pfds as needed

					/***********************************************
					 * END OF YOUR CODE
					 **********************************************/


				} else { /* the client receive a message */

					new_socket = pfds[i].fd;

					// Receive file name from client
					if (recv(new_socket, buffer, MAXMSG, 0) < 0) {
						perror("recv");
						exit(EXIT_FAILURE);
					}


					// Open requested file
					fp = fopen(buffer, "rb");
					if (fp == NULL) {
						perror("fopen file error");
						exit(EXIT_FAILURE);
					}

					bzero(buffer, MAXMSG);

					long file_size;
					// Get the file size
					fseek(fp, 0, SEEK_END);
					file_size = ftell(fp);
					fseek(fp, 0, SEEK_SET);

					/***********************************************
					 * Refer to the description, send the file length
					 * and file content to the p2p client.
					 *
					 * START YOUR CODE HERE
					 **********************************************/
					
					// send the corresponding file size / file length (file_size)
					if (send(new_socket, &file_size, sizeof(file_size), 0) < 0) {
						perror("send file size");
						// clean up after failure
						fclose(fp);
						close(new_socket);
						del_from_pfds(pfds, i, &fd_count);
						// do not proceed to send the file content if length failed to send
						continue;
					}
					// send the corresponding file content
					// keep reading maximum possible size of message from file pointer fp
					while ((bytes_read = fread(buffer, 1, MAXMSG, fp)) > 0) {
						// send the read messages
						if (send(new_socket, buffer, bytes_read, 0) < 0) {
							// raise error and end if failed to send
							perror("send file content");
							break;
						}
					}
					// raise error if failed to read
					if (bytes_read < 0) {
						perror("fread");
					}

					/***********************************************
					 * END OF YOUR CODE
					 **********************************************/

					fclose(fp);
					close(new_socket);
					del_from_pfds(pfds, i, &fd_count);

					bzero(buffer, MAXMSG);
				}
			}
		}
	}

    close(server_fd);
	pthread_exit(NULL);
}


int p2p_client(unsigned int ip, unsigned short port, char *file_name) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAXMSG] = {0};
    FILE *fp;
    ssize_t bytes_read;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fails to create");
        return -1;
    }

    // Set server address and port
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ip;
    serv_addr.sin_port = htons(port);

	printf("Connecting to p2p server ...\n");

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect error");
        return -1;
    }

	sleep(5);

    // Send file name to server
    if (send(sock, file_name, strlen(file_name), 0) < 0) {
        perror("send file name error");
        return -1;
    }


    // Receive file contents from server
    fp = fopen(file_name, "wb");
    if (fp == NULL) {
        perror("fopen file error");
        return -1;
    }


	/***********************************************
	 * Refer to the description of file transfer
	 * process to receive and save the file.
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	// receive the file size
	size_t file_size = 0;
	if(recv(sock, &file_size, sizeof(file_size), 0) <= 0){
		// if failed to receive file size, need to raise error and clean up
		perror("recv file size error");
		fclose(fp);
		close(sock);
		return -1;
	}

	// receive the file content
	size_t total_received = 0;
	// keep reading till entire file received
	while (total_received < file_size) {
		// keep reading the current chunk of data
		bytes_read = recv(sock, buffer, MAXMSG, 0);
		if (bytes_read == 0) {
			fprintf(stderr, "Server closed the connection\n");
			break;
		}else if (bytes_read < 0) {
			// if failed to receive content, raise error
			perror("recv file content error");
			fclose(fp);
			close(sock);
			return -1;
		}
		// write the received content from the buffer into the file using fp
		fwrite(buffer, 1, bytes_read, fp);
		// update how many bytes written so far
		total_received += bytes_read;
	}


	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/

	printf("Receive finished !\n");

    fclose(fp);
    close(sock);
    return 0;
}


void clear (void)
{
    while ( getchar() != '\n' );
}


int main() {
	char ip_string[20] = "127.0.0.1";
	unsigned short port = 6000;

	printf("\nInput ip address: ");
	scanf("%s", ip_string);
	unsigned int ip = inet_addr(ip_string);

	printf("\nInput port number: ");
	scanf("%hu", &port);

	/* start p2p server service */
	pthread_t tid;
	struct ip_port arg;
	arg.ip = ip;
	arg.port = port;

	printf("Creating p2p server ...\n");

	/***********************************************
	 * Start the p2p server using pthread
	 *
	 * START YOUR CODE HERE
	 **********************************************/

	if(pthread_create(&tid, NULL, p2p_server, &arg)!=0){
		perror("Unable to create pthread");
		return -1;
	}else{
		printf("successfully created pthread");
	}

	/***********************************************
	 * END OF YOUR CODE
	 **********************************************/

	sleep(1);


	int sockfd;
	struct sockaddr_in servaddr;

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER);
	servaddr.sin_port = htons(SERVER_PORT);

	char command[100];
	char file_name[256];
	unsigned int local_map;

	char register_flag = 0;
	char query_flag = 0;

	// BONUS code
	struct id_node * node_head = init_id_node_list();

	do {
		clear();

		bzero(command, sizeof(command));

		printf("\nInput command: ");
		scanf("%s", command);
		printf("Received command is: %s\n", command);

		if (strncmp(command, REGISTER, strlen(REGISTER)) == 0) {
			send_register(sockfd, servaddr, ip, port);
			register_flag = 1;
			continue;
		}

		if (strncmp(command, UPDATE, strlen(UPDATE)) == 0) {
			if ( register_flag == 0 ) {
				printf("You should first register!");
			} else {
				local_map = get_file_map();
				send_update(sockfd, servaddr, ip, port, local_map);
			}
			continue;
		}

		if (strncmp(command, QUERY, strlen(QUERY)) == 0) {
			printf("\nInput file name: ");
			scanf("%s", file_name);
			send_query(sockfd, servaddr, file_name, strlen(file_name), &node_head);
			query_flag = 1;
			continue;
		}

		if (strncmp(command, GET, strlen(GET)) == 0) {

			if ( query_flag == 0 ) {
				printf("You should first query some files!");
			} else {
				clear_input_buffer();
				printf("\nInput ip port or unique client sequence number(e.g., 127.0.0.1 6001 OR 1): ");
				char input_ip[32];
				unsigned short input_port;
				scanf("%s", input_ip);
				
				if(strchr(input_ip, '.')!=NULL){
					scanf("%hu", &input_port);
					p2p_client(inet_addr(input_ip), input_port, file_name);
				}else{
					printf("A: input ip is currently %s\n", input_ip);
					unsigned ID;
					sscanf(input_ip, "%u", &ID);
					
					unsigned short found_port = 0;
					unsigned found_ip = 0;
					int found = query_id_node(&node_head, ID, &found_ip, &found_port);

					if(found == 1){
						p2p_client(found_ip, found_port, file_name);
					}
					
				}
				sleep(2);

				/* then update */
				local_map = get_file_map();
				send_update(sockfd, servaddr, ip, port, local_map);
			}
			continue;
		}

		if (strncmp(command, EXIT, strlen(EXIT)) == 0) {
			printf("Exit!");
			return 0;
		}

		printf("\nValid commands: REGISTER | UPDATE | QUERY | GET | EXIT\n");

	} while (1);

    // Wait for server thread to finish
    if (pthread_join(tid, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }

	close(sockfd);
	return 0;
}