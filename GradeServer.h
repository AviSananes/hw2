#ifndef HW2_GRADESERVER_H
#define HW2_GRADESERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <commmon.h>


#define MAX_TASKS 100
#define MAX_MESSAGE_LENGTH 2000
#define NUM_THREADS 5
#define ASSISTANTS_FILE "assistants.txt"
#define STUDENTS_FILE "students.txt"
#define MAX_LINE_LENGTH 100  // maximum length of a line in files
#define ID_LENGTH 10 // ID is a 9-digit string


typedef struct {
    char id[ID_LENGTH];
    char password[MAX_LINE_LENGTH];  // password can be any length
    int userType;
    int grade; // TODO - should this be in different struct?
} User;

typedef struct task_t {
    char message[MAX_MESSAGE_LENGTH];
} task_t;

void* handle_connection(void* arg);
void add_task_to_queue(int connfd);
void sigint_handler(int signum);
void add_task(task_t task);
task_t get_task(void);
void delete_task(task_t task);
void process_task(task_t task);
void *process_tasks(void *arg);

#endif
