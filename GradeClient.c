#include "GradeClient.h"

char * connected_user_id = NO_USER_CONNECTED_CODE;
int connected_user_type = NO_USER_TYPE;

pthread_mutex_t logged_in_user_mutex = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *argv[])
{
    int sockfd, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char first[100], second[100], third[100];

    // Check command line arguments
    if (argc < 3) {
        fprintf(stderr,"Usage: %s hostname port\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[2]);

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Look up the server's IP address and port number
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    // Create a pipe for communication between the command line interpreter and communication process
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("ERROR creating pipe");
        exit(1);
    }

    // Create the communication process
    pid_t pid = fork();
    if (pid == -1) {
        perror("ERROR creating child process");
        exit(1);
    }

    // Command line interpreter process
    if (pid == 0) {

        close(pipefd[0]);
        
        while(1) { 
            printf("> ");
            fgets(buffer, BUFFER_SIZE, stdin);
            if (strcmp(buffer, "Exit") == 0){
                write(pipefd[1], "kill", 4);
                break;
            }
            send(sockfd, buffer, strlen(buffer), 0);
            recv(sockfd, message, BUFFER_SIZE, 0);
            write(pipefd[1], message, strlen(message));
            sleep(1);
        }
    } else {
        while(1){
            close(pipefd[1]);
            char response[BUFFER_SIZE];
            int n = read(pipefd[0], response, sizeof(response));
            if (strcmp(response,"kill") == 0){
                break;
            }
            response[n] = '\0';
            printf("%s\n", response);
        }
    }
    
    close(sockfd);

    return 0;
}
