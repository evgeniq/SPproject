#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>

#define SERVERPORT 8080
#define MAX_LINE_LENGTH 1500

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;


void intHandler(int dummy) {
}   



char* remove_spaces(char* str)
{
	int d =0;
	
	for (int i = 0; str[i]; i++)
	{
		if (str[i] != ' ' && str[i] != '\n')
		{
			str[d++] = str[i]; 
		}
	}
	
	str[d] = '\0';
	
	return str;
}

int main()
{
	//----------Conection to the server-------------------------
    int sockfd, connfd;
    SA_IN server_addr, client_addr;
    char file_name[20];
   
  signal(SIGINT, intHandler);
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
	{
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    bzero(&server_addr, sizeof(server_addr));
  
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(SERVERPORT);
  
  
    if (connect(sockfd, (SA*)&server_addr, sizeof(server_addr)) != 0) 
	{
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
	
	//--------------------Menu-----------------------------------
	char buff[50];
	char *line_buff;
	char options[10];
	char *name;
	size_t size_name = 50;;
	char id[5];
	char lines[10] = {0};
	
	size_t size = 10;
	name = (char *) malloc (50);
	line_buff = (char *) malloc (MAX_LINE_LENGTH);

	
	
	int return_status = 0;
	while (1)
	{ 
		printf("Enter a request (or write HELP):\n");
		scanf("%s", buff); 
		write(sockfd, buff, sizeof(buff));
		
		if(strcmp(buff, "HELP") == 0 || strcmp(buff, "help") == 0){
			printf("First you can choose:\n \
			 - LIST - to list avaliable books;\n \
			 - USE - to use a book\n");
			printf("When you select USE then you can choose between:\n \
			 - READ - show the entire book;\n \
			 - READ 20 - show first 20 lines;\n \
			 - READ 20,20 - show from 20th line 20 lines;\n \
			 - LINES - show how many lines the book have;\n");
		}
		else if(strcmp(buff, "LIST") == 0 || strcmp(buff, "list") == 0){
			printf("-------------------------------------------------\n");
			while(1)
			{
				read(sockfd, name, sizeof(char)*50);
				if(strstr(name, "exit") == NULL)
				{
					printf("BooK:%s \n", name);
				} 
				else
				{
 					break;
 				}
			}
			printf("-------------------------------------------------\n");
		} else if (strcmp(buff, "USE")  == 0  || strcmp(buff, "use") == 0){
			//scanf("%s", name);
			getline(&name, &size_name , stdin);
			strcpy(name,remove_spaces(name));
			
			write(sockfd, name, sizeof(char)*50);
			memset(buff, 0 , sizeof(buff));

			read(sockfd, buff, sizeof(buff));

			if(strcmp(buff,"+OK!") == 0) 
			{
				printf("%s\n", buff);
			} 
			else if(strcmp(buff, "We don't have this book! Sorry!") == 0)
			{
				printf("%s\n", buff);
				continue;
				
			}
			
			while(1){
				printf("Choose READ or LINES or BACK\n");
				scanf("%s", buff);
					if (strcmp(buff, "READ")  == 0  || strcmp(buff, "read") == 0)
					{

						if (scanf ("%8[^\n]%*c", options) < 1) 
						{
							printf("Sent READ ALL\n");
							write(sockfd, buff, sizeof(buff));
							memset(buff, 0 , sizeof(buff));
						} 
						else
						{
							strcpy(options,remove_spaces(options));
							
						}
						printf("Option: <%s>\n", options);
						write(sockfd, buff, sizeof(buff));
						write(sockfd, options, sizeof(options));
						
						memset(buff, 0, sizeof(buff));
						
						printf("-------------------------------------------------\n");
						while(1)
						{			
							read(sockfd, line_buff, sizeof(char)*MAX_LINE_LENGTH);
							
							if(strcmp(line_buff,"exit") == 0)
							{							
								break;
							}

							printf("%s", line_buff);
						}
						printf("-------------------------------------------------\n");
						continue;
					
					} 
					else if (strcmp(buff, "LINES") == 0  || strcmp(buff, "lines") == 0)
					{
						write(sockfd, buff, sizeof(buff)); // send lines to socket
						printf("BUFF: %s\n", buff);
						memset(buff, 0 , sizeof(buff));
						read(sockfd, buff, sizeof(buff));
						printf("The book:\"%s\" - has %s lines\n", name, buff);
					} 
					else if (strcmp(buff, "BACK") == 0  || strcmp(buff, "back") == 0)
					{
						write(sockfd, buff, sizeof(buff));
						break;

					} 
					else 
					{
						printf("[CLIENT]: Plese use only READ or LINES\n");
					}
			}
		}
		else
		{
			read(sockfd, buff, sizeof(buff));
			printf("%s\n", buff);
			//printf("[CLIENT]: Plese use only LIST or USE or HELP\n");
		}



  	}

    close(sockfd);

}



