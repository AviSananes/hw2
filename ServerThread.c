#include <ServerThread.h>
// TODO - make the users global

User connected_user_id;

void login(char * id, char * password, char * response) {
    // Iterate through the array of users (TODO - add both students and TA)
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].id, id)) {  // If the id matches
            if (strcmp(users[i].password, password) == 0) {  // If the password matches
                snprintf(response, 255, "Welcome Student %s\n", users[i].id)
                // TODO - add locks
                connected_user_id = users[i];
                return;
            } else {
                response = "Incorrect password\n";
                return;
            }
        }
    }
    response = "Wrong user information\n"
}

void read_grade(char* id, char * response);
void grade_list(char * response);
void update_grade(char* id, char* grade, char * response);
void logout(char * response);


void* handle_connection(void* arg) {
    while (1) {
        // Take a task from the task list
        int connfd;
        // TODO - the tasks list lock is released only after the cond wait ends?
        pthread_mutex_lock(&task_list_mutex);
        while (task_list_size == 0) {
            pthread_cond_wait(&task_list_cond, &task_list_mutex);
        }
        connfd = task_list[--task_list_size];
        pthread_mutex_unlock(&task_list_mutex);

        // Read input from the client
        // TODO - use const for the length
        char buffer[256];
        int n = read(connfd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            return NULL;
        }
        buffer[n] = '\0';

        char * command, * arg1, * arg2;
        separate_strings(buffer, command, arg1, arg2);

        // Handle the command.
        char * response;
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

        // Send a response back to the client
        n = write(connfd, response, strlen(response));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }

        // Close the connection
        close(connfd);
    }

    return NULL;
}

#include "ServerThread.h"
