#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include<string.h>
#define NAME_LENGTH 100
#define BUFFER_SIZE 1024

// Function prototypes for clarity
void get_input_line(int argc, char* argv[], char *username, char *password, char *folder, int *messageNum, int *tls, char *command, char *serverName);
int establish_connection(char* serverName, char *port);
void send_command(int sockfd, const char *command);
void receive_response(int sockfd, char *buffer);
void logging_on(int sockfd, char *username, char* password);
void select_folder(int sockfd, const char *folder);


void get_input_line(int argc, char* argv[], char *username, char *password, char *folder, int *messageNum, char *command, char *serverName){
    *tls =0;//Transport Layer Security
    *messageNum = -1;  // Default invalid message number
    strcpy(folder, "INBOX");  // Default folder


    for (int i = 1; i < argc; i+=2){
        // username
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc){
            strcpy(username, argv[i + 1]);
        }// password
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc){
            strcpy(password, argv[i + 1]);
        }// folder
        else if(strcmp(argv[i], "-f") == 0 && i + 1 < argc){
            strcpy(folder, argv[i + 1]);
        }
        // messageNum
        else if(strcmp(argv[i], "-n") == 0 && i + 1 < argc){
            *messageNum = atoi(argv[i + 1]);
        }// 这里应该是先看是否启用TLS， 非TLS的话，port是143， 否则是993（esatablish_connection函数中）
        else if (strcmp(argv[i], "-t") == 0) {
            *tls = 1;
        }
    }

    // The last two arguments : command and serverName
    strcpy(command, argv[argc - 2]);
    strcpy(serverName, argv[argc - 1]);
}

//Create an IPv6 socket (or fall back to IPv4 if IPv6 is not supported) on port 143 (or a TLS socket on port
//993 if you are doing the extension task and -t was specified) to the host named on the command line.

int establish_connection(char* serverName, int port){
    struct addrinfo hints, *res;
    int sockfd;
    char *port = tls ? "993" : "143";


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;// // AF_UNSPEC allows for IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(serverName, port, &hints, &res) != 0){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    return sockfd;
}



void send_command(int sockfd, const char *command) {
    if (send(sockfd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }
}

void receive_response(int sockfd, char *buffer) {
    ssize_t bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';
}


void logging_on(int sockfd, char *username, char* password){
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    sprintf(buffer, "A01 LOGIN %s %s\r\n", username, password);//RFC 3501

    // Check if login was successful
    if (strstr(buffer, "A01 OK") == NULL) {
        fprintf(stderr, "Login failure\n");
        exit(EXIT_FAILURE);
    }
}

void select_folder(int sockfd, const char *folder) {
    char buffer[BUFFER_SIZE];

    // Send SELECT command to server
    sprintf(buffer, "A02 SELECT %s\r\n", folder);
    send_command(sockfd, buffer);// prepared command  sent through the socket, connected to  IMAP 
    receive_response(sockfd, buffer);

    // Check if folder selection was successful
    if (strstr(buffer, "A02 OK") == NULL) {
        fprintf(stderr, "Folder not found\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    char username[NAME_LENGTH];
    char password[NAME_LENGTH];
    char folder[NAME_LENGTH];
    int messageNum;
    char command[NAME_LENGTH];
    char serverName[NAME_LENGTH];
    int tls;

    get_input_line(argc, argv, username, password, folder, &messageNum, command, serverName);
    logging_on(sockfd, username, password)
    select_folder(sockfd, folder);

    
    int sockfd = establish_connection(serverName, tls);

    logging_on(sockfd, username, password);
    select_folder(sockfd, folder);

    close(sockfd);
}
