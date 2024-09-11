#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
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
            .desc = "Basic count",
            .input = "abbcc",
            .expected = "1a, 2b, 2c\n",
        },
        {
            .desc = "Mixed case",
            .input = "aA bB cC",
            .expected = "1a, 1b, 1c\n",
        },
        {
            .desc = "Ignore non-alphabetic",
            .input = "abc123!@#",
            .expected = "1a, 1b, 1c\n",
        },
        {
            .desc = "Empty input",
            .input = "",
            .expected = "\n",
        },
        {
            .desc = "All characters",
            .input = "abcdefghijklmnopqrstuvwxyz",
            .expected = "1a, 1b, 1c, 1d, 1e, 1f, 1g, 1h, 1i, 1j, 1k, 1l, 1m, 1n, 1o, 1p, 1q, 1r, 1s, 1t, 1u, 1v, 1w, 1x, 1y, 1z\n",
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
            execl("./count_alpha", "./count_alpha", tests[i].input, NULL); // Execute count_alpha
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

                // Compare the result with expected output
                if (strcmp(buffer, tests[i].expected) != 0) {
                    printf("    [ERROR] [%d] %s: Expected '%s', got '%s'\n", i + 1, tests[i].desc, tests[i].expected, buffer);
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
