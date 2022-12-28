#ifndef HW2_SERVERTHREAD_H
#define HW2_SERVERTHREAD_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <common.h>

extern int task_list_size;
extern int task_list_size;
extern int * task_list; // TODO - how do we allocate the right memory for this?
extern pthread_mutex_t task_list_mutex;
extern pthread_cond_t task_list_cond;

void * handle_connection(void * arg);
void login(char * id, char * password, char * response);
void read_grade(char* id, char * response);
void grade_list(char * response);
void update_grade(char* id, char* grade, char * response);
void logout(char * response);

#endif //HW2_SERVERTHREAD_H
