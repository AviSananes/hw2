#include "common.h"


void separate_strings(char* input, char* first, char* second, char* third) {
    /// Separate strings received as input into a maximum of 3 words ///
    int i;

    // Initialize first string to empty string
    first[0] = '\0';

    // Iterate through input string
    for (i = 0; input[i] != '\0'; i++) {
        // If space is found, terminate the first string and start
        // copying characters to the second string
        if (input[i] == ' ') {
            first[i] = '\0';
            strcpy(second, input + i + 1);
            break;
        }

        // Otherwise, copy the character to the first string
        first[i] = input[i];
    }

    // If no space is found, set the second and third strings to empty strings
    if (input[i] == '\0') {
        second[0] = '\0';
        third[0] = '\0';
        return;
    }

    // Iterate through the second string
    for (i = 0; second[i] != '\0'; i++) {
        // If space is found, terminate the second string and start
        // copying characters to the third string
        if (second[i] == ' ') {
            second[i] = '\0';
            strcpy(third, second + i + 1);
            return;
        }
    }

    // If no space is found in the second string, set the third string to an empty string
    third[0] = '\0';

}
