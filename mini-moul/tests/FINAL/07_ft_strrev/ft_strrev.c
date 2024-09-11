#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../../ex03/ft_strrev.c"
#include "../../../utils/constants.h"

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
            .desc = "Reverse a normal string",
            .input = "Hello, World!",
            .expected = "!dlroW ,olleH",
        },
        {
            .desc = "Reverse a single character string",
            .input = "A",
            .expected = "A",
        },
        {
            .desc = "Reverse a string with numbers and symbols",
            .input = "12345!@#$%",
            .expected = "%$#@!54321",
        },
        {
            .desc = "Reverse an empty string",
            .input = "",
            .expected = "",
        },
        {
            .desc = "Reverse a palindrome string",
            .input = "racecar",
            .expected = "racecar",
        },
        {
            .desc = "Reverse a string with spaces",
            .input = "foo bar",
            .expected = "rab oof",
        },
        {
            .desc = "Reverse a string with trailing spaces",
            .input = "abc   ",
            .expected = "   cba",
        }
    };
    int count = sizeof(tests) / sizeof(tests[0]);

    return run_tests(tests, count);
}

int run_tests(t_test *tests, int count)
{
    int i;
    int error = 0;

    for (i = 0; i < count; i++)
    {
        char *input_copy = strdup(tests[i].input); // Duplicate input string
        char *result = ft_strrev(input_copy);

        if (strcmp(result, tests[i].expected) != 0)
        {
            printf("    " RED "[%d] %s Expected \"%s\", got \"%s\"\n", i + 1, tests[i].desc, tests[i].expected, result);
            error -= 1;
        }
        else
        {
            printf("  " GREEN CHECKMARK GREY " [%d] %s Expected \"%s\", got \"%s\"\n" DEFAULT, i + 1, tests[i].desc, tests[i].expected, result);
        }

        free(input_copy); // Free the duplicated string after the test
    }

    return error;
}