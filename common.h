//
// Created by Miki Segall on 28/12/2022.
//

#ifndef HW2_COMMON_H
#define HW2_COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "common.h"

// TODO - should we use enum?
#define NO_USER_TYPE 0
#define ASSISTANT_USER_TYPE 1
#define STUDENT_USER_TYPE 2

void separate_strings(char* input, char* first, char* second, char* third);

#endif //HW2_COMMON_H
