/*

COP4610

Project 1 - Implementing a Shell

Group Members:
John Sanchez
Emanuel Gonzalez
Redden Money

___________________________________

Requirements we still need:

• Adding '~' Path Resolution
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

int cssh_cd(char **args);
int cssh_exit(char **args);
int cssh_pwd(char **args);
int cssh_ls(char **args);

char *builtin[] = {
  "cd",
  "exit",
  "pwd",
  "ls"
};

int num_builtins() {
  return sizeof(builtin) / sizeof(char *);
}

int (*builtin_func[]) (char **) = {
  &cssh_cd,
  &cssh_exit,
  &cssh_pwd,
  &cssh_ls
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
  } else if (strcmp(var,"$HOME") == 0){
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

int cssh_launch(char **args) {
  pid_t p, wp;
  int stat, fd0, fd1, i, fd[2];
  int bg_flag = 0;
  char buf;
  p = fork();
  if(p == 0) {
    for(i = 0; args[i] != '\0'; i++) {
      if(strcmp(args[i],"<") == 0) {
	if((fd0 = open(args[i+1], O_RDONLY)) < 0) {
          perror("cant open file");
          exit(0);
        }
	dup2(fd0, 0);
	close(fd0);
      }
      if(strcmp(args[i],">") == 0) {
        if((fd1 = open(args[i+1], O_WRONLY | O_CREAT, 0644)) < 0) {
	  perror("cant open file\n");
	  exit(0);
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
      perror("cssh");
    }
    exit(EXIT_FAILURE);
  }
  else if(p < 0) {
    perror("cssh");
  }
  else {
    //if(!bg_flag) {
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

char **cssh_split(char *line) {
  char * _line = tok_prep(line);
  int buff = TOK_BUFF;
  int position = 0;
  char **tokens = malloc(buff * sizeof(char*));
  char *token;
  if(!tokens) {
    fprintf(stderr, "cssh: alloc error\n");
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
       fprintf(stderr, "cssh: alloc error\n");
       exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, "\a");
  }
  tokens[position] = NULL;
  return tokens;
}

void cssh_loop() {
  char * line;
  char ** args;
  int status;
  clock_gettime(CLOCK_REALTIME, &start);
  do {
    /* long num;                     // Prompt Set-Up
    int num2;
    char *b1;
    char *b2;
    char *path;
    char *name;
    name = getenv("USER");      // Get User
    char *user;
    strcpy(user, name);
    strcat(user, "@");
    num2 = gethostname(b2, sizeof(b2));   //  Get Machine

    path = getcwd(b1, (size_t)num);     // Get Path

    strcat(user , b2);
    strcat(user , " :: ");
    strcat(user, path);
    strcat(user, " -> "); */
    fullPrompt();
    line = readline(prompt);
    args = cssh_split(line);
    //char* var;
    //strcpy(var,args[0]);
    int i = 0;
    /*while(i != sizeof(args)) {
 	//strcpy(var,args[i]);
    	args[i] = env_var_switch(args[i]);
	i++;
    }*/
    status = cssh_execute(args);

    free(line);
    free(args);
  } while(status);
}

int cssh_cd(char **args) {
  if(args[1] == NULL) {
    fprintf(stderr, "cssh: expected argument to \"cd\"\n");
  }
  else {
    if(chdir(args[1]) != 0) {
      perror("cssh");
    }
  }
  return 1;
}

int cssh_exit(char **args) {
  clock_gettime(CLOCK_REALTIME, &stop);
  double elapsed2 = (stop.tv_sec - start.tv_sec);
  printf("Exiting...\n\tSession time: %ds \n", (int)elapsed2);
  if(stop.tv_sec != 0)
  //return 0;
    exit(EXIT_SUCCESS);
}

int cssh_pwd(char **args) {
  char pwd[1024];
  getcwd(pwd, sizeof(pwd));
  printf("%s\n", pwd);
  return 1;
}

int cssh_ls(char **args) {
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

int cssh_execute(char **args) {
  int i;
  if(args[0] == NULL)
    return 1;
  for(i = 0; i < num_builtins(); i++) {
    if(strcmp(args[0], builtin[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return cssh_launch(args);
}

int main (int argc, char ** argv) {


    cssh_loop();              // Main Loop
    printf("poot");
    return EXIT_SUCCESS;          // Exit
}
