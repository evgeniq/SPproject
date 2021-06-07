#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <dirent.h>

#define SERVERPORT 8080
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BLOCKLOG 5
#define MAX_LINE_LENGTH 1500

pthread_mutex_t lock;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void * handle_connection(void* p_client_socket);
int check(int exp, const char *msg);
void message_to_client(int client_socket, const char * mess);

struct node{
	char name[50];
	char id[5];
	struct node* next;
};

typedef struct node node_t;


void list_books(node_t *head ,int client_socket){
	struct dirent *d;
	DIR *dh = opendir("books");
	char *book_name;
	book_name = (char *) malloc (50);
	char exit[5] = "exit1";

	if(!dh){
		perror("Directory doesn't exist");	
	} 
	
	while((d = readdir(dh)) != NULL){
		if(d->d_name[0] == '.') continue;
		book_name = strtok(d->d_name, ".");

		//printf("%s\n", book_name);
		write(client_socket, book_name, sizeof(char)*50);

	}
	write(client_socket, exit, sizeof(exit));
}

void read_book(char* name, char* argument, int client_socket){
	FILE * fp;
	char *first, *second, *pt;
	char *line;
	int line_count = 0;
	char path[55] = "books/";
	int argument_int = 0; 
	int first_int = 0;
	int second_int = 0;

	line = (char *)malloc(MAX_LINE_LENGTH* sizeof(char));
	first = (char *)malloc(10* sizeof(char));
	second = (char *)malloc(10 * sizeof(char));
	pt = (char *)malloc(10 * sizeof(char));
	
	strcat(path, name);
	strcat(path, ".txt");

    fp = fopen(path, "r");
    
    if (fp == NULL)
    {
    	printf("Book is null");
        exit(EXIT_FAILURE);
	}
	
	 
	if((second = strchr(argument, ',')) != NULL) // two arguments
	{
		pt = strtok (argument,",");
		strcpy(first, pt);

		pt = strtok (NULL,",");
		strcpy(second, pt);

		first_int = atoi(first);
		second_int = atoi(second);
		
		while (fgets(line, MAX_LINE_LENGTH, fp))
		{
			++line_count;
			if(line_count >= first_int && line_count <= second_int)
			{
			   	write(client_socket,line, sizeof(char)*MAX_LINE_LENGTH);     

			}
		}
			
		write(client_socket,"exit", sizeof(argument));
		memset(second, 0, sizeof(second));				
	}
	else // no arguments
	{
		strcpy(first, argument);
		if(strcmp(first, "") == 0)
		{
	
			while (fgets(line, MAX_LINE_LENGTH, fp))
			{
				++line_count;
				write(client_socket,line, sizeof(char)*MAX_LINE_LENGTH);     
			}
			
			write(client_socket,"exit", sizeof(argument));
		}
		else // one argument
		{
			argument_int = atoi(argument);
			while (fgets(line, MAX_LINE_LENGTH, fp))
			{
			
				++line_count;
				if(line_count <= argument_int)
				{
			   		write(client_socket,line, sizeof(char)*MAX_LINE_LENGTH);     

				}
			}
			
			write(client_socket,"exit", sizeof(argument));
		}
		memset(first, 0, sizeof(first));
	}
							
}

void get_lines(char* name, int client_socket){

    FILE * fp;
    char line[MAX_LINE_LENGTH] = {0};
	int line_count = 0;
	char path[55] = "books/";
	
	strcat(path, name);
	strcat(path, ".txt");
	printf("Path: %s\n", path);
    fp = fopen(path, "r");
    if (fp == NULL){
    	printf("Book is null");
        exit(EXIT_FAILURE);
	}
	while (fgets(line, MAX_LINE_LENGTH, fp))
   		{
			++line_count;
		}
	//int conv_line_count = htonl(line_count);
	char str[10];
	sprintf(str, "%d", line_count);
	printf("This book has: %s lines\n", str);
	write(client_socket, str, sizeof(str));

}

//Linked list of books
node_t *add_book(char* name, char* id){
	node_t *result = malloc(sizeof(node_t));
	strcpy(result->name, name);
	strcpy(result->id, id);
	result->next = NULL;
	return result;
	
}

node_t *insert_at_head(node_t **head, node_t *node_to_insert){
	node_to_insert->next = *head;
	*head = node_to_insert;
	return node_to_insert;

}

node_t *find_book(node_t *head, char* name){
	node_t *tmp = head;

	while(tmp != NULL){
		if(strcmp(tmp->name, name) == 0) return tmp;

		tmp = tmp->next;

	}
	
	return NULL;
}




int main(int argc, char **argv)
{
	int server_socket, client_socket, addr_size;
	SA_IN server_addr, client_addr;

	check((server_socket = socket(AF_INET, SOCK_STREAM,0)), "Failed to create socket");

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr =  inet_addr("127.0.0.1");
	server_addr.sin_port = htons(SERVERPORT);

	check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)), "Bind Failed");
	

	check(listen(server_socket, SERVER_BLOCKLOG), "Listen Failed");

	while(true)
	{
		printf("Waiting for connections... \n");
		addr_size = sizeof(SA_IN);
		check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),"Accept Failed");
		printf("Connedted!\n");
		//handle_connection(client_socket);

		pthread_t t;
		int *pclient = malloc(sizeof(int));
		*pclient = client_socket;
		pthread_create(&t, NULL, handle_connection, pclient); 
		
	}

	
	return 0;
}


int check(int exp, const char *msg)
{
	if(exp == SOCKETERROR){
		perror(msg);	
		exit(1);	
	}
	return exp;
}

void * handle_connection(void* p_client_socket)
{
	int client_socket = *((int*)p_client_socket);
	free(p_client_socket);
	char buffer[BUFSIZE];
	char argument[BUFSIZE];
	size_t bytes_read;
	char name[50];
	bool use_flag = true;

	node_t *head = NULL;
	node_t *tmp;
	
	//Add books
	tmp = add_book("IvanVazov", "1");
	insert_at_head(&head, tmp);
	tmp = add_book("2", "2");
	insert_at_head(&head, tmp);
	tmp = add_book("Откачалки", "3");
	insert_at_head(&head, tmp);
	tmp = add_book("Cirminal", "4");
	insert_at_head(&head, tmp);

	while(1)
	{
		bytes_read = read(client_socket, buffer, sizeof(buffer));
		check(bytes_read, "Failed");

		printf("[REQUEST]: <%s>\n", buffer);

		if(strcmp(buffer, "LIST") == 0 || strcmp(buffer, "list") == 0)
		{
			//print_list(head, client_socket);
			list_books(head, client_socket);
		}
		else if (strcmp(buffer, "USE")  == 0  || strcmp(buffer, "use") == 0)
		{

			read(client_socket, name, sizeof(char)*50);

			if((tmp= find_book(head, name)) == NULL)
			{

				message_to_client(client_socket,"We don't have this book! Sorry!");
				continue;
			}
			else
			{
				message_to_client(client_socket, "+OK!");
			}
			
			while(1) 
			{
				
					memset(buffer, 0, sizeof(buffer));
					
					read(client_socket, buffer, sizeof(buffer));
					printf("[Buffer]: %s\n", buffer);
					
					if (strcmp(buffer, "READ")  == 0  || strcmp(buffer, "read") == 0)
					{
						read(client_socket, argument, sizeof(argument));
						pthread_mutex_lock(&lock);
						read_book(name, argument, client_socket);
						pthread_mutex_unlock(&lock);
						memset(argument, 0, sizeof(argument));
						continue;
						
					} 
					else if (strcmp(buffer, "LINES") == 0  || strcmp(buffer, "lines") == 0)
					{
						pthread_mutex_lock(&lock);
						get_lines(name, client_socket);	
						pthread_mutex_unlock(&lock);
					}
					else if (strcmp(buffer, "BACK") == 0  || strcmp(buffer, "back") == 0)
					{
						break;
						
					} 
					else
					{
						message_to_client(client_socket,"[SERVER]; Plese use only READ or LINES\n");
					}
				
			}
		}
		else 
		{
			message_to_client(client_socket,"[SERVER]: Plese use only LIST or USE\n");
		} 
	}
	close(client_socket);
	

	return NULL;
}


void message_to_client(int client_socket, const char * mess)
{
	char message[50];
	strcpy(message, mess);
	write(client_socket, message, sizeof(message));
}












