#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct s_test
{
    char *desc;
    char *input;
    char *expected;
} t_test;

int run_tests(t_test *tests, int count);

int main(void)
{
    t_test tests[] = {
        {
            .desc = "No 'z' in input",
            .input = "./aff_z abc",
            .expected = "z\n",
        },
        {
            .desc = "Multiple words, no 'z'",
            .input = "./aff_z 'dubO a POIL'",
            .expected = "z\n",
        },
        {
            .desc = "First character is 'z'",
            .input = "./aff_z 'zaz sent le poney'",
            .expected = "z\n",
        },
        {
            .desc = "No parameters",
            .input = "./aff_z",
            .expected = "z\n",
        },
        {
            .desc = "Input with 'z' in the middle",
            .input = "./aff_z 'hello z world'",
            .expected = "z\n",
        },
        {
            .desc = "Empty string",
            .input = "./aff_z ''",
            .expected = "z\n",
        },
        {
            .desc = "String with multiple 'z'",
            .input = "./aff_z 'zzz'",
            .expected = "z\n",
        }
    };

    int count = sizeof(tests) / sizeof(tests[0]);
    return run_tests(tests, count);
}

int run_tests(t_test *tests, int count)
{
    int i;
    int error = 0;
    char buffer[1024];
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
            system(tests[i].input); // Execute test command
            close(pipefd[1]); // Close writing end
            exit(0);
        } else {
            // Parent process
            close(pipefd[1]); // Close writing end
            wait(NULL); // Wait for child process to finish

            // Read output from pipe
            int n = read(pipefd[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0'; // Null-terminate the string

                // Compare the result with expected output
                if (strcmp(buffer, tests[i].expected) != 0) {
                    printf("    [ERROR] [%d] %s: Expected \"%s\", got \"%s\"\n", i + 1, tests[i].desc, tests[i].expected, buffer);
                    error -= 1;
                } else {
                    printf("    [OK] [%d] %s: Expected \"%s\", got \"%s\"\n", i + 1, tests[i].desc, tests[i].expected, buffer);
                }
            }
            close(pipefd[0]); // Close reading end
        }
    }

    return error;
}
