#define MYSHELL_RL_BUFSIZE 1024
#define MYSHELL_TOK_BUFSIZE 64
#define MYSHELL_TOK_DELIM " \t\r\n\a"

/*
  Function Declarations for shell internal commands:
 */
int myshell_cd(char **args);
int myshell_help(char **args);
int myshell_quit(char **args);
int myshell_clr(char **args);
int myshell_environ(char **args);
int myshell_pause(char **args);
int myshell_execute(char **args);
char *getcwdstr(char *buffer, int size);
char *stripath(char *pathname);
