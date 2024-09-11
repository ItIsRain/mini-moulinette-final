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
            .desc = "Mixed case with punctuation",
            .input = "L'eSPrit nE peUt plUs pRogResSer s'Il staGne et sI peRsIsTent VAnIte et auto-justification.",
            .expected = "l'EspRIT Ne PEuT PLuS PrOGrESsER S'iL STAgNE ET Si PErSiStENT vaNiTE ET AUTO-JUSTIFICATION.\n",
        },
        {
            .desc = "Simple case change with spaces",
            .input = "S'enTOuRer dE sECreT eSt uN sIGnE De mAnQuE De coNNaiSSanCe.  ",
            .expected = "s'ENtoUrER De SecREt EsT Un SigNe dE MaNqUe dE COnnAIssANcE.  \n",
        },
        {
            .desc = "Numbers and mixed case",
            .input = "3:21 Ba  tOut  moUn ki Ka di KE m'en Ka fe fot",
            .expected = "3:21 bA  ToUT  MOuN KI kA DI ke M'EN kA FE FOT\n",
        },
        {
            .desc = "No arguments",
            .input = "",
            .expected = "\n",
        },
        {
            .desc = "Argument with only numbers",
            .input = "1234567890",
            .expected = "1234567890\n",
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
            execl("./ulstr", "./ulstr", tests[i].input, NULL); // Execute ulstr
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
