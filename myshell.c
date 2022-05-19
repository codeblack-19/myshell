#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "myshell.h"
#include <signal.h>


//Reading line of input from the standard input (stdin)
char *myshell_read_line(void)
{
    int bufsize = MYSHELL_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer){
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize)
        {
            bufsize += MYSHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                fprintf(stderr, "myshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// sliting shell input into tokens
char **myshell_split_line(char *line)
{
    int bufsize = MYSHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MYSHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += MYSHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "myshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MYSHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void myshell_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        char *gcu;
        gcu = getenv("USER");

        // char *host;
        // host = getenv("NAME");

        signal(SIGINT, SIG_IGN);  // prevent ^C interrupt
        signal(SIGCHLD, SIG_IGN); // prevent Zombie children

        printf("%s@:%s> ", gcu ,cwd);
        line = myshell_read_line();
        args = myshell_split_line(line);
        status = myshell_execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv){
    char *shellpath;
    char cwdbuf[MYSHELL_RL_BUFSIZE];
    // Load config files, if any.

    shellpath = strdup(strcat(strcat(getcwdstr(cwdbuf, sizeof(cwdbuf)), "/"), stripath(argv[0])));

    // set SHELL= environment variable, malloc and store in environment
    if (putenv(strdup(strcat(strcpy(cwdbuf, "SHELL="), shellpath)))){
        fprintf(stderr, "changing SHELL environment failed");
        perror(NULL);
    }

    // Run command loop.
    myshell_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}