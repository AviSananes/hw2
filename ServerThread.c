#include <ServerThread.h>

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
        switch (command) {
            case "Login":
                // TODO - continue
                ...
        }

        // Send a response back to the client
        char* response = "Request received and processed.";
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
