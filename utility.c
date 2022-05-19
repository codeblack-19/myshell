#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "myshell.h"


//check through user input to find redirection input
int redirectInput(char **args, char **inputfile) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Look for the < character
    if(args[i][0] == '<') {
        
      // Read the filename
      if(args[i+1] != NULL) {
	    *inputfile = args[i+1];
      } else {
	    return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	    args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

//check through user input to find redirection output
int redirectOutput(char **args, char **outputfile) {
  int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Looking for the > character
    if(args[i][0] == '>' && args[i][1] == '\0') {
      // Get the filename 
      if(args[i+1] != NULL) {
	    *outputfile = args[i+1];
      } else {
	    return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	    args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

//check through user input to find redirection output to append
int redirectOuputAppend(char **args, char **outputfile){
    int i;
  int j;

  for(i = 0; args[i] != NULL; i++) {

    // Looking for the > character
    if(args[i][0] == '>' && args[i][1] == '>') {
      // Get the filename 
      if(args[i+1] != NULL) {
	    *outputfile = args[i+1];
      } else {
	    return -1;
      }

      // Adjust the rest of the arguments in the array
      for(j = i; args[j-1] != NULL; j++) {
	    args[j] = args[j+2];
      }

      return 1;
    }
  }

  return 0;
}

//background execution by checking for ampersand token
int ampersand(char **args){
    int i;

  for(i = 1; args[i] != NULL; i++){
    if(args[i - 1][0] == '&') {
        free(args[i - 1]);
        args[i - 1] = NULL;
        return 1;
    } else {
        return 0;
    }
  
  }
  return 0;
}

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
    "cd",
    "help",
    "quit",
    "clr",
    "environ",
    "pause"
};

int (*builtin_func[])(char **) = {
    &myshell_cd,
    &myshell_help,
    &myshell_quit,
    &myshell_clr,
    &myshell_environ,
    &myshell_pause,
};

// function to get the number of internal shell commands
int myshell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}


// Implementation of internal shell commands

/**
   @brief Internal command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int myshell_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "myshell: expected argument to \"cd\" \n");
    }else{
        if(chdir(args[1]) != 0 ){
            perror("myshell");
        }
    }

    return 1;
}

/**
   @brief Internal command: help -> display the user manual using the more filter.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int myshell_help(char **args){
    int output;
    int outputAppend;
    char *outputfile;
    char buf[MYSHELL_TOK_BUFSIZE];

    output = redirectOutput(args, &outputfile);
    outputAppend = redirectOuputAppend(args, &outputfile);

    if(output){
        snprintf(buf, sizeof(buf), "more < readme.parse > %s", outputfile);
        system(buf);
    }

    if(outputAppend){
        snprintf(buf, sizeof(buf), "more < readme.parse >> %s", outputfile);
        system(buf);
    }

    if(!output && !outputAppend){
        system("more readme.parse");
    }

    return 1;
}

/**
   @brief internal command: quit the shell.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int myshell_quit(char **args){
    return 0;
}

/**
   @brief internal command: clr -> clear the screen.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int myshell_clr(char **args){
    system("clear");
    return 1;
}

/**
   @brief internal command: pause -> pause operation of the shell until 'Enter' is pressed.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int myshell_pause(char **args){
    getpass("myshell Paused, Press Enter to continue............");
    return 1;
}

/**
   @brief internal command: environ -> list all the environment strings.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int myshell_environ(char **args){
    system("printenv");
    return 1;
}

//Funtion to launch a program amd wait for it to terminate
int myshell_launch(char **args, int block, int input, char *inputfile, 
                    int output, char *outputfile, 
                    int outputAppend
                )
{
    pid_t pid;
    int result;
    int status;

    pid = fork();
    if (pid == 0){
        //setting up redirection in child process
        if(input){
            freopen(inputfile, "r", stdin);
        }

        if(output){
            freopen(outputfile, "w+", stdout);
        }

        if(outputAppend){
            freopen(outputfile, "a+", stdout);
        }
        
        // Child process
        result = execvp(args[0], args);
        if ( result == -1){
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    }
    if(block){
        // printf("Waiting for Background, pid = %d\n", pid);
        result = waitpid(pid, &status, 0);
    }

    return 1;
}


//Function to execute shell built-in
int myshell_execute(char **args){
    int i;
    int input;
    int output;
    int outputAppend;
    char *outputfile;
    char *inputfile;
    int block;

    if (args[0] == NULL){
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < myshell_num_builtins(); i++){
        if (strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    //check for ampersand(&) token for backgroud execution
    block = (ampersand(args) == 0);

    //check for redirected input
    input = redirectInput(args, &inputfile);
    switch(input) {
    case -1:
      printf("Syntax error!\n");
      return 1;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting input from: %s\n", inputfile);
      break;
    }

    //check for redirection output
    output = redirectOutput(args, &outputfile);
    switch(output) {
    case -1:
      printf("Syntax error!\n");
      return 1;
      break;
    case 0:
      break;
    case 1:
      printf("Redirecting output to: %s\n", outputfile);
      break;
    }

    //check for redirection output to append
    outputAppend = redirectOuputAppend(args, &outputfile);
    switch(outputAppend) {
    case -1:
      printf("Syntax error!\n");
      return 1;
      break;
    case 0:
      break;
    case 1:
      printf("Appeding output to: %s\n", outputfile);
      break;
    }

    return myshell_launch(args, block, input, inputfile, output, outputfile, outputAppend);
}

//return start of buffer containing current working directory pathname
char *getcwdstr(char *buffer, int size)
{
  if (!getcwd(buffer, size))
  { // read current working directory
    fprintf(stderr, "getcwd");
    perror(NULL);
    buffer[0] = 0;
  }
  return buffer;
}

//strip path from file name
char *stripath(char *pathname)
{
  char *filename = pathname;

  if (filename && *filename)
  {                                    // non-zero length string
    filename = strrchr(filename, '/'); // look for last '/'
    if (filename)                      // found it
      if (*(++filename))               //  AND file name exists
        return filename;
      else
        return NULL;
    else
      return pathname; // no '/' but non-zero length string
  }                    // original must be file name only
  return NULL;
}