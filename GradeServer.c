#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "common.c"


#define NUM_THREADS 5
#define MAX_TASKS 1000
#define MAX_LINE_LENGTH 256
#define ID_LENGTH 9
#define NO_USER_TYPE 0
#define ASSISTANT_USER_TYPE 1
#define STUDENT_USER_TYPE 2
int num_assistants;
int num_students;

typedef struct {
  char id[10];
  char password[265];
  int connfd;
  int is_ta;
} Client;

Client* clients;
int num_clients;

pthread_mutex_t clients_mutex;

void init_clients() {
  // Initialize the mutex lock
  pthread_mutex_init(&clients_mutex, NULL);
}

pthread_mutex_t logged_in_user_mutex = PTHREAD_MUTEX_INITIALIZER;


typedef struct {
    char id[ID_LENGTH];
    char password[MAX_LINE_LENGTH];  // password can be any length
    int userType;
    int grade; // TODO - should this be in different struct?
} User;

User assistants[MAX_LINE_LENGTH];
User students[MAX_LINE_LENGTH];

int find_user(User *people, int n, char * id, char * password) {
  for (int i = 0; i < n; i++) {
    if (strcmp(people[i].id, id) == 0 && strcmp(people[i].password, password) == 0) {
      return 0;
    }
  }
  return -1;
}

int client_sock;

pthread_t threads[NUM_THREADS];

typedef struct task {
    char message[2000];
} task_t;

task_t tasks[MAX_TASKS];
int num_tasks = 0;

pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;

void sigint_handler(int signum);
void add_task(task_t task);
task_t get_task(void);
void delete_task(task_t task);
void process_task(task_t task, User* user);
void *process_tasks(void *arg);
void separate_strings(char* input, char* first, char* second, char* third);
char* login(char * id, char * password, char * response, User* user);
char* read_grade(char* id, char * response, User* user);
char* grade_list(char * response);
char* update_grade(char* id, char* grade, char * response, User* user);


int loadUsersFromFile(User * users, char* filename, int user_type) {
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
        if (user_type == STUDENT_USER_TYPE) {
            users[num_users].grade = 0;
        }
        num_users++;
    }

    fclose(file);
    return num_users;
}



int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s port_number\n", argv[0]);
        exit(1);
    }

    int port_number = atoi(argv[1]);

    signal(SIGINT, sigint_handler);
    int socket_desc, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000];

      // Load the assistants and students from files
    num_assistants = loadUsersFromFile(assistants, "assistants.txt", ASSISTANT_USER_TYPE);
    num_students = loadUsersFromFile(students, "students.txt", STUDENT_USER_TYPE);

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port_number);

    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        // print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Listen
    listen(socket_desc, 3);

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, process_tasks, NULL);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    // accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    
    // Receive a message from client and add it to the task list
    while (1)
    {
      while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
      {
        // Add the message to the task list
        task_t task;
        strcpy(task.message, client_message);
        add_task(task);
        // Clear the client_message buffer
        memset(client_message, 0, 2000);
        read_size = 0;
      }
      if (read_size == -1)
      {
        perror("recv failed");
      }
    }
    for (int i = 0; i < NUM_THREADS; i++)
      pthread_join(threads[i], NULL);

    return 0;
}



void sigint_handler(int signum)
{
    printf("Received SIGINT\n");
    // Close the socket and exit
    close(client_sock);
    exit(1);
}

void add_task(task_t task)
{
    pthread_mutex_lock(&task_mutex);

    // Add the task to the list
    tasks[num_tasks++] = task;

    pthread_cond_signal(&task_cond);
    pthread_mutex_unlock(&task_mutex);
}

task_t get_task(void)
{
    pthread_mutex_lock(&task_mutex);

    while (num_tasks == 0)
        pthread_cond_wait(&task_cond, &task_mutex);

    // Get the next task from the list
    // TODO - I think this always takes the last task (LIFO) should this be FIFO?
    task_t task = tasks[--num_tasks];

    pthread_mutex_unlock(&task_mutex);
    return task;
}

void process_task(task_t task, User* user)
{
    // Do something with the task
    char* command[MAX_LINE_LENGTH], arg1[ID_LENGTH], arg2[MAX_LINE_LENGTH], response[MAX_LINE_LENGTH];
    strcpy(response, "\0");
    separate_strings(task.message, command, arg1, arg2);
    printf(task.message);
    if (strcmp(command, "Login") == 0) {
        login(arg1, arg2, response, user);
    } else if (strcmp(command, "ReadGrade") == 0) {
        read_grade(arg1, response, user);
    } else if (strcmp(command, "GradeList") == 0) {
        grade_list(response);
    } else if (strcmp(command, "UpdateGrade") == 0) {
        update_grade(arg1, arg2, response, user);
    } else {
        strcpy(response, "Wrong input");
    }
    printf(command);
    send(client_sock, response, strlen(response), 0);
}

void *process_tasks(void *arg)
{
    User* user;

    while (1)
    { 
        task_t task = get_task();
        process_task(task, user);
        memset(&task, 0, sizeof(task));
    }

    return NULL;
}

char* login(char * id, char * password, char * response, User* user) {
    
    strcpy(response,"");
    int logged_in = 0;
    pthread_mutex_lock(&clients_mutex);


    if (find_user(assistants, num_assistants, id, password) == 0) {
        strcpy(response,"Welcome Student");
        strcpy(user->id,id);
        strcpy(user->password,password);
        user->userType = ASSISTANT_USER_TYPE;
    } else if (find_user(students, num_students, id, password) == 0) {
        strcpy(response,"Welcome TA");
        strcpy(user->id,id);
        strcpy(user->password,password);
        user->userType = STUDENT_USER_TYPE;
    } else {
        strcpy(response,"Wrong user information\n");
    }
    pthread_mutex_unlock(&clients_mutex);
    return response;
}

char* read_grade(char* id, char * response, User* user) {

    strcpy(response,"");
    pthread_mutex_lock(&logged_in_user_mutex);
    if (user->userType == ASSISTANT_USER_TYPE) {
        if (id == NULL) {
            strcpy(response,"Missing argument");
            pthread_mutex_unlock(&logged_in_user_mutex);
        }
    }
    if (id != NULL && user->userType == STUDENT_USER_TYPE) {
        strcpy(response,"Action not allowed");
        pthread_mutex_unlock(&logged_in_user_mutex);
    }

    char * requested_id = (user->userType == STUDENT_USER_TYPE ? user->id : id);
    pthread_mutex_unlock(&logged_in_user_mutex);
    for (int i = 0; i < num_students; i++) {
        if (strcmp(students[i].id, requested_id) == 0) {
            strcat(response,students[i].grade);
        }
    }
    return response;
}

char* grade_list(char * response) {
    
    strcpy(response,"");

    for (int i = 0; i < num_students; i++) {
        // TODO - should this be print or somehow else?
        printf("%s: %d\n", students[i].id, students[i].grade);
        strcat(response,students[i].id);
        strcat(response,": ");
        strcat(response,students[i].grade);
        strcat(response,"\n");
        }
    return response;
}

char* update_grade(char* id, char* grade, char * response, User* user) {
    
    strcpy(response,"");
    if (user->userType == ASSISTANT_USER_TYPE) {
        for (int i = 0; i < num_students; i++) {
            if (strcmp(students[i].id, id) == 0) {
                students[i].grade = atoi(grade);
            }
        }
        strcpy(response, "Grade updated");
    } else {
        strcpy(response, "Action not allowed");
    }
    return response;
}
