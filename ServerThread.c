#include <ServerThread.h>
// TODO - make the users global

User connected_user_id;

void login(char * id, char * password, char * response) {
    // Iterate through the array of users (TODO - add both students and TA)
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].id, id)) {  // If the id matches
            if (strcmp(users[i].password, password) == 0) {  // If the password matches
                response = (users[i].user_type == STUDENT_USER_TYPE) ? "Welcome Student" : "Welcome TA";
                // TODO - add locks
                connected_user_id = users[i];
                return;
            } else {
                response = "Incorrect password\n";
                return;
            }
        }
    }
    response = "Wrong user information\n";
}

void read_grade(char* id, char * response) {
    // TODO - should this validation be in the client or server?
    pthread_mutex_lock(&logged_in_user_mutex);
    if (connected_user_type == ASSISTANT_USER_TYPE) {
        if (id == NULL) {
            printf("Missing argument")
            pthread_mutex_unlock(&logged_in_user_mutex);
            return
        }
    }
    if (id != NULL && connected_user_type == STUDENT_USER_TYPE) {
        printf("Action not allowed")
        pthread_mutex_unlock(&logged_in_user_mutex);
        return
    }

    char * requested_id = (connected_user_type == STUDENT_USER_TYPE ? connected_user_id : id)
    pthread_mutex_unlock(&logged_in_user_mutex);
    for (int i = 0; i < num_students; i++) {
        if (strcmp(students[i].id, requested_id) == 0) {
            response = students[i].grade;
        }
    }
};

// Comparison function to sort the array of students based on their id attribute
int compare_students_by_id(const void * a, const void * b) {
    User * ua = (Student*)a;
    User * ub = (Student*)b;
    return ua->id - ub->id;
}

void grade_list(char * response) {
    qsort(students, num_students, sizeof(User), compare_students_by_id);
    for (int i = 0; i < num_students; i++) {
        // TODO - should this be print or somehow else?
        printf("%s: %d\n", students[i].id, students[i].grade);
    }
};

void update_grade(char* id, char* grade, char * response);
    for (int i = 0; i < num_students; i++) {
        if (strcmp(students[i].id, id) == 0) {
            students[i].grade = atoi(grade);
            return
        }
    }
    // A student with a corresponding ID was not found - adding it to the list
    User new_student = {id, "DefaultPass", STUDENT_USER_TYPE, atoi(grade)};
    // TODO - append the new user to the existing ones
}


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
