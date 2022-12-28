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

#define NUM_THREADS 5
#define ASSISTANTS_FILE "assistants.txt"
#define STUDENTS_FILE "students.txt"
#define MAX_LINE_LENGTH 100  // maximum length of a line in files
#define ID_LENGTH 10 // ID is a 9-digit string

void separate_strings(char* input, char* first, char* second, char* third);
void* handle_connection(void* arg);
void sigint_handler(int sig);

typedef struct {
    char id[ID_LENGTH];
    char password[MAX_LINE_LENGTH];  // password can be any length
} User;

#endif
