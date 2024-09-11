#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
            .desc = "Single argument",
            .input = "j'aime le savon",
            .expected = "j'aime le savon\n",
        },
        {
            .desc = "Multiple arguments",
            .input = "zaz", "mange", "des", "chats",
            .expected = "chats\n",
        },
        {
            .desc = "No arguments",
            .input = "",
            .expected = "\n",
        },
        {
            .desc = "Single empty argument",
            .input = "",
            .expected = "\n",
        },
        {
            .desc = "Arguments with spaces",
            .input = "hello world", "goodbye world",
            .expected = "goodbye world\n",
        },
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
            execl("./aff_last_param", "./aff_last_param", tests[i].input, NULL); // Execute aff_last_param
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
