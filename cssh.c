/*

COP4610

Project 1 - Implementing a Shell

Group Members:
John Sanchez
Emanuel Gonzalez
Redden Money

___________________________________

Requirements we still need:

• I/O Redirection - (Works but we need to make identical to bash)
• Piping (Refining the functionality)
• Background Processing

*/


#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define TOK_DELIMS " \t\r\n\a"
#define TOK_BUFF 64

/*

PROMPT

*/

void fullPrompt() {
  long num;
  char *b;
  char *path; 
  path = getcwd(b, (size_t)num);
  if(path == NULL)
    printf("%s@%s :: %s -> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));
  else
    printf("%s@%s :: %s -> ", getenv("USER"), getenv("MACHINE"), path);
}

static const char prompt[] = "";

struct timespec start, stop;

int shell_cd(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_ls(char **args);
int shell_io(char **args);

char *builtin[] = {
  "cd",
  "exit",
  "pwd",
  "ls",
  "io"
};

int num_builtins() {
  return sizeof(builtin) / sizeof(char *);
}

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_exit,
  &shell_pwd,
  &shell_ls,
  &shell_io
};
/*

Environment Variable Handler:

Takes in Environment Variable argument
and returns variable value

*/

char* env_var_switch(char* var){
  char *a;

  if (strcmp(var,"$PATH") == 0){
    a = getenv("PATH");
    return(a);      // Get Path
  } else if (strcmp(var,"$HOME") == 0 || strcmp(var,"~") == 0){
    a = getenv("HOME");
    return(a);      // Get Home Path
  }
 else if (strcmp(var,"$USER") == 0){
   a = getenv("USER");
   return(a);      // Get User
} else if (strcmp(var,"$SHELL") == 0){
  a = getenv("SHELL");
  return(a);      // Get Shell
} else if (strcmp(var,"$PWD") == 0){
  a = getenv("PWD");
  return(a);      // Get Current Path
}

return var;

}

int shell_launch(char **args) {
  pid_t p, wp;
  int stat, fd0, fd1, i, fd[2];
  int bg_flag = 0;
  char buf;
  p = fork();
  if(p == 0) {
    for(i = 0; args[i] != '\0'; i++) {
      if(strcmp(args[i],"<") == 0) {
	if(args[i+1] == NULL) {
	  fprintf(stderr, "Missing name for redirect\n");
	  return 1;
        }
	else {
	  if((fd0 = open(args[i+1], O_RDONLY)) < 0) {
            perror("cant open file");
            exit(0);
          }
        }
	dup2(fd0, 0);
	close(fd0);
      }
      if(strcmp(args[i],">") == 0) {
	if(args[i+1] == NULL) {
          fprintf(stderr, "Missing name for redirect\n");
          return 1;
        }
	else {
          if((fd1 = open(args[i+1], O_WRONLY | O_CREAT, 0644)) < 0) {
	    perror("cant open file\n");
	    exit(0);
	  }
	}
	dup2(fd1, STDOUT_FILENO);
        close(fd1);
      }
      if(strcmp(args[i],"|") == 0) {
	pipe(fd);
	if((p = fork()) == 0) {
	  dup2(fd[1], 1);
	  close(fd[0]);
	  close(fd[1]);
	  execvp(args[i-1], args);
        }
	else {
	  dup2(fd[0], 0);
	  close(fd[0]);
	  close(fd[1]);
	  execvp(args[i+1], args);
	}
      }
      if(strcmp(args[i], "&") == 0) {
	if(i != 0 && i == sizeof(args-1)) {
      	  bg_flag = 1;
	  break;
	}
      }
    }
    if(execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  }
  else if(p < 0) {
    perror("shell");
  }
  else {
    //if(!bg_flag) {
     /*
     if (args[0] == "io")
       printf("background pid: %d\n", p);
    */
      do {
        wp = waitpid(p, &stat, WUNTRACED);
      } while(!WIFEXITED(stat) && WIFSIGNALED(stat));
    //}
    /*else {
      printf("background pid: %d\n", p);
    }*/
  }
  /*wp = waitpid(-1, &stat, WNOHANG);
  while (wp > 0) {
    printf("completed bg pid: %d\n", wp);
  }*/
  return 1;
}

char * tok_prep(char * line) {
  int pos = 0;
  while(pos != strlen(line) - 1) {
    if(line[pos] == ' ' || line[pos] == '\t') {
      line[pos] = '\a';
    }
    else if(line[pos] == '\'') {
      line[pos] = '\a';
      while(line[pos] != '\'') {
	pos++;
      }
      line[pos] = '\a';
    }
    pos++;
  }
  return line;
}

char **shell_split(char *line) {
  char * _line = tok_prep(line);
  int buff = TOK_BUFF;
  int position = 0;
  char **tokens = malloc(buff * sizeof(char*));
  char *token;
  if(!tokens) {
    fprintf(stderr, "shell: alloc error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(_line, "\a");
  while(token != NULL) {
    char * temp;
    temp = token;
    char * temp2 = env_var_switch(temp);
    tokens[position] = temp2;
    position++;
    if(position >= buff) {
      buff += TOK_BUFF;
      tokens = realloc(tokens, buff * sizeof(char*));
      if(!tokens) {
       fprintf(stderr, "shell: alloc error\n");
       exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, "\a");
  }
  tokens[position] = NULL;
  return tokens;
}

/*

MAIN LOOP

*/ 

void shell_loop() {
  char * line;
  char ** args;
  int status;
  clock_gettime(CLOCK_REALTIME, &start);
  do {
    fullPrompt();
    line = readline(prompt);
    args = shell_split(line);
    int i = 0;
    status = shell_execute(args);

    free(line);
    free(args);
  } while(status);
}

/*
 
BUILT IN: CD

*/

int shell_cd(char **args) {
  if(args[1] == NULL) {
    chdir(getenv("HOME"));
  }
  else {
    if(args[2] != NULL) {
      fprintf(stderr, "cd: Too many arguments\n");
    }
    else if(chdir(args[1]) != 0) {
      perror("cd");
    }
  }
  return 1;
}

/*

BUILT IN: io

*/
int shell_io(char **args) {
  printf("hello");

  return 1;
}
/*

BUILT IN: EXIT

*/

int shell_exit(char **args) {
  clock_gettime(CLOCK_REALTIME, &stop);
  double elapsed2 = (stop.tv_sec - start.tv_sec);
  printf("Exiting...\n\tSession time: %ds \n", (int)elapsed2);
  if(stop.tv_sec != 0)
  //return 0;
    exit(EXIT_SUCCESS);
}

/*

BUILT IN: PWD

*/

int shell_pwd(char **args) {
  char pwd[1024];
  getcwd(pwd, sizeof(pwd));
  printf("%s\n", pwd);
  return 1;
}

/*

BUILT IN: LS

*/

int shell_ls(char **args) {
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  if(args[1] == NULL) {
    if(d) {
      while((dir = readdir(d))) {
        printf("%s  ", dir->d_name);
      }
      printf("\n");
      closedir(d);
    }
    else
      perror("Could not open directory");
  }
  else {

    d = opendir(args[1]);
    if(d) {
      while((dir = readdir(d))) {
        printf("%s  ", dir->d_name);
      }
      printf("\n");
      closedir(d);
    }
    else
      perror("Could not open directory");
  }
  return 1;
}

int shell_execute(char **args) {
  int i;
  if(args[0] == NULL)
    return 1;
  for(i = 0; i < num_builtins(); i++) {
    if(strcmp(args[0], builtin[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return shell_launch(args);
}

/*

MAIN

*/

int main (int argc, char ** argv) {


    shell_loop();              // Main Loop
    printf("poot");
    return EXIT_SUCCESS;          // Exit
}
