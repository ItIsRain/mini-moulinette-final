#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef struct s_test
{
    char *desc;
    char *expected;
} t_test;

int run_tests(t_test *tests, int count);

int main(void)
{
    t_test tests[] = {
        {
            .desc = "Basic FizzBuzz output test",
            .expected = 
                "1\n2\nfizz\n4\nbuzz\nfizz\n7\n8\nfizz\nbuzz\n"
                "11\nfizz\n13\n14\nfizzbuzz\n16\n17\nfizz\n19\nbuzz\n"
                "fizz\n22\n23\nfizz\nbuzz\n26\nfizz\n28\n29\nfizzbuzz\n"
                "31\n32\nfizz\n34\nbuzz\nfizz\n37\n38\nfizz\nbuzz\n"
                "41\nfizz\n43\n44\nfizzbuzz\n46\n47\nfizz\n49\nbuzz\n"
                "fizz\n52\n53\nfizz\nbuzz\n56\nfizz\n58\n59\nfizzbuzz\n"
                "61\n62\nfizz\n64\nbuzz\nfizz\n67\n68\nfizz\nbuzz\n"
                "71\nfizz\n73\n74\nfizzbuzz\n76\n77\nfizz\n79\nbuzz\n"
                "fizz\n82\n83\nfizz\nbuzz\n86\nfizz\n88\n89\nfizzbuzz\n"
                "91\n92\nfizz\n94\nbuzz\nfizz\n97\n98\nfizz\nbuzz\n"
        }
    };

    int count = sizeof(tests) / sizeof(tests[0]);
    return run_tests(tests, count);
}

int run_tests(t_test *tests, int count)
{
    int i;
    int error = 0;
    char buffer[2048];
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
            execl("./fizzbuzz", "./fizzbuzz", NULL); // Execute fizzbuzz
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
                    printf("    [ERROR] [%d] %s: Expected output does not match.\n", i + 1, tests[i].desc);
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
