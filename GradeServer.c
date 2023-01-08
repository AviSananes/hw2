#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREADS 5
#define MAX_TASKS 1000


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
void process_task(task_t task);
void *process_tasks(void *arg);


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
    close(sockfd);
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

void delete_task(task_t task)
{
    pthread_mutex_lock(&task_mutex);

    for (int i = 0; i < num_tasks; i++)
    {
        if (strcmp(tasks[i].message, task.message) == 0)
        {
            // Shift all tasks after the specified index one position to the left
            for (int j = i; j < num_tasks - 1; j++)
                tasks[j] = tasks[j + 1];

            num_tasks--;
            break;
        }
    }

    pthread_mutex_unlock(&task_mutex);
}

void process_task(task_t task)
{
    // Do something with the task
    char* command[2000];
    strcpy(command, task.message);

    switch (command) {
        // TODO - should we validate no additional arguments?

        case "Login":
            login(arg1, arg2, response);
            break;

        case "ReadGrade":
            read_grade(arg1, response);
            break;

        case "GradeList":
            grade_list(response);
            break;

        case "UpdateGrade":
            update_greade(arg1, arg2, response);
            break;

        case "Logout":
            logout(response);
            break;

        default:
            response = "Wrong Input";
    }



    send(client_sock, task.message, strlen(task.message), 0);
}

void *process_tasks(void *arg)
{
    while (1)
    { 
        task_t task = get_task();
        process_task(task);
        delete_task(task);
        memset(&task, 0, sizeof(task));
    }

    return NULL;
}