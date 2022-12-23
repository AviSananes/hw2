#ifndef GRADECLIENT_H
#define GRADECLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 256

void login(int sockfd, char* id, char* password);
void read_grade(int sockfd, char* id);
void grade_list(int sockfd);
void update_grade(int sockfd, char* id, char* grade);
void logout(int sockfd, char* id);

#endif
