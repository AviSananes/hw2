#include <GradeClient.h>

char * connected_user_id = NO_USER_CONNECTED_CODE;
int connected_user_type = NO_USER_TYPE;

pthread_mutex_t logged_in_user_mutex = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *argv[])
{
    int sockfd, port, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];
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
        // Close the write end of the pipe
        close(pipefd[1]);
        
        while(1){ 
            printf("> ");
            fgets(buffer, BUFFER_SIZE, stdin);
            if (strcmp(buffer, 'Exit') == 0) {
                exit(1);
            }
            write(pipefd[0], buffer, strlen(buffer));
        }
    } else {
        close(pipefd[0]);
        char response[BUFFER_SIZE];
        while (1){
            int bytes_read = read(pipefd[1], response, BUFFER_SIZE);
            if (bytes_read == 0) {
                // Child process has closed the pipe
                break;
                } 
            // Send the command to the server
            write(sockfd, response, strlen(response));
            read(sockfd, buffer, BUFFER_SIZE);
            printf(buffer);
        }
    }
    
    close(sockfd);

    return 0;
}

int check_if_user_already_connected() {
    pthread_mutex_lock(&logged_in_user_mutex);
    // Check if a user is already logged in from this client
    int returnValue = (connected_user_id == NO_USER_CONNECTED_CODE) ? NO_USER_CONNECTED_CODE : USER_CONNECTED_CODE;
    pthread_mutex_unlock(&logged_in_user_mutex);
    return returnValue

}

void modify_connected_user(char * id, int user_type) {
    pthread_mutex_lock(&logged_in_user_mutex);
    strcpy(connected_user_id, id);
    connected_user_type = user_type;
    pthread_mutex_unlock(&logged_in_user_mutex);
}


void login(int sockfd, char* id, char* password) {
    // Check if user already connected
    int isUserConnected = check_if_user_already_connected();
    if (isUserConnected == USER_CONNECTED_CODE) {
        printf("There is a user already logged in.")
        return;
    }

    // Send the login request to the server
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "Login %s %s", id, password);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("ERROR sending login request");
        exit(1);
    }

    // Read the response from the server
    memset(buffer, 0, BUFFER_SIZE);
    int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n < 0) {
        perror("ERROR receiving login response");
        exit(1);
    }

    // Parse the response and print the appropriate message
    if (strcmp(buffer, "Welcome Student") == 0) {
        modify_connected_user(id, STUDENT_USER_TYPE);
        printf("Welcome Student %s\n", id);
    } else if (strcmp(buffer, "Welcome TA") == 0) {
        modify_connected_user(id, ASSISTANT_USER_TYPE);
        printf("Welcome TA %s\n", id);
    } else {
        printf("Wrong user information\n");
    }
}

void read_grade(int sockfd, char* id) {
    // Verify user connected
    if (check_if_user_already_connected() == NO_USER_CONNECTED_CODE) {
        printf("Not logged in")
        retrurn
    }

    // TODO - should this validation be in the client or server?
    pthread_mutex_lock(&logged_in_user_mutex);
    if (id == NULL && connected_user_type == ASSISTANT_USER_TYPE) {
        printf("Missing argument")
        pthread_mutex_unlock(&logged_in_user_mutex);
        return
    }
    if (id != NULL && connected_user_type == STUDENT_USER_TYPE) {
        printf("Action not allowed")
        pthread_mutex_unlock(&logged_in_user_mutex);
        return
    }
    pthread_mutex_unlock(&logged_in_user_mutex);

    // Send the read grade request to the server
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "ReadGrade %s", id);
    if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
        perror("ERROR sending read grade request");
        exit(1);
    }

    // Read the response from the server
    memset(buffer, 0, BUFFER_SIZE);
    int n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (n < 0) {
        perror("ERROR receiving read grade response");
        exit(1);
    }

    // Parse the response and print the appropriate message
    if (strncmp(buffer, "Invalid id", 10) == 0) {
        printf("Invalid id\n");
    } else if (strncmp(buffer, "Action not allowed", 18) == 0) {
        printf("Action not allowed\n");
    } else if (strncmp(buffer, "Missing argument", 16) == 0) {
        printf("Missing argument\n");
    } else {
        printf("%s: %s\n", id, buffer);
    }
}
