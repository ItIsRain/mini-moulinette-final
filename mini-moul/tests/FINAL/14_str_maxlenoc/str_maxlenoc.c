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
            .desc = "Normal input",
            .input = "hello world",
            .expected = "hello world\n",
        },
        {
            .desc = "Input with spaces",
            .input = "a b c d e f g h i j k l m n o p q r s t u v w x y z",
            .expected = "a b c d e f g h i j k l m n o p q r s t u v w x y z\n",
        },
        {
            .desc = "Special characters",
            .input = "1234567890 !@#$%^&*()",
            .expected = "1234567890 !@#$%^&*()\n",
        },
        {
            .desc = "Empty input",
            .input = "",
            .expected = "\n",
        },
        {
            .desc = "Long string",
            .input = "This is a very long string with multiple words and characters to test the maximum length functionality.",
            .expected = "This is a very long string with multiple words and characters to test the maximum length functionality.\n",
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
            execl("./str_maxlenoc", "./str_maxlenoc", tests[i].input, NULL); // Execute str_maxlenoc
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
