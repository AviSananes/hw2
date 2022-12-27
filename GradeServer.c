#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define NUM_THREADS 5

int* task_list;
int task_list_size;
pthread_mutex_t task_list_mutex;
pthread_cond_t task_list_cond;
int sockfd;


void separate_strings(char* input, char* first, char* second, char* third);
void* handle_connection(void* arg);
void sigint_handler(int sig);


int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }
  
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = sigint_handler;
  sigaction(SIGINT, &action, NULL);


  // Get the port number from the command line argument
  int port = atoi(argv[1]);

  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Error opening socket");
    return 1;
  }

  // Set up the server address
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  // Bind the socket to the server address
  if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("Error binding socket");
    return 1;
  }

  // Listen for incoming connections
  listen(sockfd, 5);

  // Create a pool of worker threads
  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    int* connfd = malloc(sizeof(int));
    if (connfd == NULL) {
      perror("Error allocating memory for connection descriptor");
      return 1;
    }

    // Accept a new connection and assign it to a worker thread
    *connfd = accept(sockfd, NULL, NULL);
    if (*connfd < 0) {
      perror("Error accepting connection");
      return 1;
    }
    if (pthread_create(&threads[i], NULL, handle_connection, connfd) != 0) {
      perror("Error creating thread");
      return 1;
    }
  }

  // Wait for all threads to finish
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // Close the socket
  close(sockfd);
  return 0;
}




void separate_strings(char* input, char* first, char* second, char* third) {
  int i;

  // Initialize first string to empty string
  first[0] = '\0';

  // Iterate through input string
  for (i = 0; input[i] != '\0'; i++) {
    // If space is found, terminate the first string and start
    // copying characters to the second string
    if (input[i] == ' ') {
      first[i] = '\0';
      strcpy(second, input + i + 1);
      break;
    }

    // Otherwise, copy the character to the first string
    first[i] = input[i];
  }

  // If no space is found, set the second and third strings to empty strings
  if (input[i] == '\0') {
    second[0] = '\0';
    third[0] = '\0';
    return;
  }

  // Iterate through the second string
  for (i = 0; second[i] != '\0'; i++) {
    // If space is found, terminate the second string and start
    // copying characters to the third string
    if (second[i] == ' ') {
      second[i] = '\0';
      strcpy(third, second + i + 1);
      return;
    }
  }

  // If no space is found in the second string, set the third string to an empty string
  third[0] = '\0';
}


void* handle_connection(void* arg) {
  while (1) {
    // Take a task from the task list
    int connfd;
    pthread_mutex_lock(&task_list_mutex);
    while (task_list_size == 0) {
      pthread_cond_wait(&task_list_cond, &task_list_mutex);
    }
    connfd = task_list[--task_list_size];
    pthread_mutex_unlock(&task_list_mutex);

    // Handle the connection here.
    // For example, you may read data from the connection and send a response back.
    // ...

    close(connfd);
  }
  return NULL;
}

void sigint_handler(int sig) {
  printf("Received SIGINT, shutting down...\n");

  // Close the socket and exit
  close(sockfd);
  exit(0);
}


