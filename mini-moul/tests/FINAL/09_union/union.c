#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct s_test
{
    char *desc;
    char *input1;
    char *input2;
    char *expected;
} t_test;

int run_tests(t_test *tests, int count);

int main(void)
{
    t_test tests[] = {
        {
            .desc = "No duplicate characters, different order",
            .input1 = "zpadinton",
            .input2 = "paqefwtdjetyiytjneytjoeyjnejeyj",
            .expected = "zpadintoqefwjy\n",
        },
        {
            .desc = "Mixed characters with duplicates",
            .input1 = "ddf6vewg64f",
            .input2 = "gtwthgdwthdwfteewhrtag6h4ffdhsd",
            .expected = "df6vewg4thras\n",
        },
        {
            .desc = "Simple phrases",
            .input1 = "rien",
            .input2 = "cette phrase ne cache rien",
            .expected = "rienct phas\n",
        },
        {
            .desc = "No arguments",
            .input1 = "",
            .input2 = "",
            .expected = "\n",
        },
        {
            .desc = "Single character arguments",
            .input1 = "a",
            .input2 = "b",
            .expected = "ab\n",
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
            execl("./union", "./union", tests[i].input1, tests[i].input2, NULL); // Execute union
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
