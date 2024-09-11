#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "list.h"  // Include the header for t_list and function prototypes

typedef struct s_test
{
    char *desc;
    int *input;
    int input_size;
    int *expected;
    int expected_size;
} t_test;

int run_tests(t_test *tests, int count);

int main(void)
{
    t_test tests[] = {
        {
            .desc = "Sort list in ascending order",
            .input = (int[]) {5, 3, 1, 4, 2},
            .input_size = 5,
            .expected = (int[]) {1, 2, 3, 4, 5},
            .expected_size = 5,
        },
        {
            .desc = "Already sorted list",
            .input = (int[]) {1, 2, 3, 4, 5},
            .input_size = 5,
            .expected = (int[]) {1, 2, 3, 4, 5},
            .expected_size = 5,
        },
        {
            .desc = "List with negative numbers",
            .input = (int[]) {-1, -3, -2, -5, -4},
            .input_size = 5,
            .expected = (int[]) {-5, -4, -3, -2, -1},
            .expected_size = 5,
        },
        {
            .desc = "Single element list",
            .input = (int[]) {42},
            .input_size = 1,
            .expected = (int[]) {42},
            .expected_size = 1,
        },
        {
            .desc = "Empty list",
            .input = (int[]) {},
            .input_size = 0,
            .expected = (int[]) {},
            .expected_size = 0,
        },
    };

    int count = sizeof(tests) / sizeof(tests[0]);
    return run_tests(tests, count);
}

int run_tests(t_test *tests, int count)
{
    int i;
    int error = 0;
    int pipefd[2];

    for (i = 0; i < count; i++)
    {
        // Create a pipe to capture output
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return -1;
        }

        // Fork to execute the program
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {
            // Child process
            close(pipefd[0]); // Close reading end
            dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe

            // Create a list from the input array
            t_list *lst = NULL;
            for (int j = 0; j < tests[i].input_size; j++) {
                t_list *new_node = (t_list *)malloc(sizeof(t_list));
                new_node->data = tests[i].input[j];
                new_node->next = lst;
                lst = new_node;
            }

            // Sort the list
            t_list *sorted = sort_list(lst, ascending);

            // Output the sorted list
            while (sorted) {
                printf("%d ", sorted->data);
                sorted = sorted->next;
            }
            printf("\n");

            close(pipefd[1]); // Close writing end
            exit(0);
        } else {
            // Parent process
            close(pipefd[1]); // Close writing end
            wait(NULL); // Wait for child process to finish

            // Read output from pipe
            char buffer[1024];
            int n = read(pipefd[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0'; // Null-terminate the string

                // Convert buffer to int array
                int output[1024];
                int size = 0;
                char *token = strtok(buffer, " \n");
                while (token) {
                    output[size++] = atoi(token);
                    token = strtok(NULL, " \n");
                }

                // Compare the result with expected output
                if (size != tests[i].expected_size ||
                    memcmp(output, tests[i].expected, sizeof(int) * size) != 0) {
                    printf("    [ERROR] [%d] %s: Expected [", i + 1, tests[i].desc);
                    for (int k = 0; k < tests[i].expected_size; k++) {
                        printf("%d", tests[i].expected[k]);
                        if (k != tests[i].expected_size - 1) printf(", ");
                    }
                    printf("], got [");
                    for (int k = 0; k < size; k++) {
                        printf("%d", output[k]);
                        if (k != size - 1) printf(", ");
                    }
                    printf("]\n");
                    error -= 1;
                } else {
                    printf("    [OK] [%d] %s: Expected output matches.\n", i + 1, tests[i].desc);
                }
            }
            close(pipefd[0]); // Close reading end
        }
    }

    return error;
}
