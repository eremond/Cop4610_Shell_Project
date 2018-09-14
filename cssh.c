/*

COP4610

Project 1 - Implementing a Shell

Group Members:
John Sanchez
Emanuel Gonzalez
Redden Money

___________________________________

Requirements we still need:

• Environment Variables
• Adding '~' path to home directory
• I/O Redirection - (Works but we need to make identical to bash)
• Piping

*/


#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#define TOK_DELIMS " \t\r\n\a"
#define TOK_BUFF 64

char prompt[] = "USER@MACHINE :: PWD -> ";



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

int cssh_launch(char **args) {
  pid_t p, wp;
  int stat, fd0, fd1, i;
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
    do {
      wp = waitpid(p, &stat, WUNTRACED);
    } while(!WIFEXITED(stat) && WIFSIGNALED(stat));
  }
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
    tokens[position] = token;
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

void cssh_loop(char* user) {
  char * line;
  char ** args;
  int status;

  do {
    line = readline(user);
    args = cssh_split(line);
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
  return 0;
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
    long num;                     // Prompt Set-Up
    int num2;
    char *b1 ;
    char *b2[100];
    char *path;

    char *user = getenv("USER");      // Get User
    strcat(user, "@");

    num2 = gethostname(b2, sizeof(b2));   //  Get Machine

    path = getcwd(b1, (size_t)num);     // Get Path

    strcat(user , b2);
    strcat(user , " :: ");
    strcat(user, path);
    strcat(user, " -> ");


    cssh_loop(user);              // Main Loop

    return EXIT_SUCCESS;          // Exit
}
