#include <GradeServer.h>
#include <ServerThread.h>

int sockfd;


int loadUsersFromFile(User * users, const char* filename, int user_type) {
    // Open the file for reading
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    int num_users = 0;  // number of users read from the file

    // Read each line of the file
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {

        char* id = strtok(line, ":");
        char* password = strtok(NULL, ":");

        // Store the ID and password in the data structure
        strcpy(users[num_users].id, id);
        strcpy(users[num_users].password, password);
        users[num_users].userType = user_type;
        num_users++;
    }

    fclose(file);
    return num_users;
}

void add_task_to_queue(int connfd) {
    // Add the task (connection) to the task list
    pthread_mutex_lock(&task_list_mutex);
    task_list[task_list_size++] = connfd;
    pthread_cond_signal(&task_list_cond);
    pthread_mutex_unlock(&task_list_mutex);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 1;
  }

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigaction(SIGINT, &sa, NULL);

  // Load the assistants and students from files
  Assistant assistants[MAX_LINE_LENGTH];
  int num_assistants = loadUsersFromFile(assistants, ASSISTANTS_FILE, ASSISTANT_USER_TYPE);
  int num_students = loadUsersFromFile(assistants, STUDENTS_FILE, STUDENT_USER_TYPE);
  int student_grades[num_students] = {0};

  // Print the data from the data structure - TEST PRINT! Remove after checking it works
  for (int i = 0; i < num_users; i++) {
     printf("ID: %s, Password: %s\n", assistants[i].id, assistants[i].password);
  }

  // Init task list and locks
  task_list_size = 0;
  pthread_mutex_init(&task_list_mutex, NULL);
  pthread_cond_init(&task_list_cond, NULL);

  // Get the port number from the command line argument
  int port = atoi(argv[1]);

  // Create a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    if (pthread_create(&threads[i], NULL, handle_connection, NULL) != 0) {
      perror("Error creating thread");
      return 1;
    }
  }

  // Infinite loop - accept user's connection and adding task for every new connection
  while (1) {
    // Accept a connection
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    int connfd = accept(sockfd, (struct sockaddr*) &cli_addr, &cli_addr_len);
    if (connfd < 0) {
      perror("Error accepting connection");
      return 1;
    }
      add_task_to_queue(conndf);
  }
  // TODO - this part never gets called because of the while loop before!
  // Wait for the worker threads to finish
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // Close the socket
  close(sockfd);

  // Free memory and destroy synchronization variables
  free(task_list);
  pthread_mutex_destroy(&task_list_mutex);
  pthread_cond_destroy(&task_list_cond);

  return 0;
}


void sigint_handler(int sig) {
  printf("Received SIGINT, shutting down...\n");

  // Close the socket and exit
  close(sockfd);
  exit(0);
}


