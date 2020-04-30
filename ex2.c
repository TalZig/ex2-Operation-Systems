#include <stdio.h>
#include <stdlib.h>
#include "signal.h"
#include <unistd.h>
#include "string.h"
#include "sys/types.h"
#include "sys/wait.h"
#include <sys/stat.h>
typedef struct {
  pid_t pid;
  int isEnded;
  char *str[100];
  char *buff;
} build;

//function that removes the last slash.
void removeSlash(char *c) {
  int i = 0;
  int maxSlash;
  while (c[i] != '\0') {
    if (c[i] == '/')
      maxSlash = i;
    i++;
  }
  c[maxSlash] = '\0';
}
//function that handles jobs
void jobsFunction(build builds[], int end, int fatherID) {
  int i;
  int k;
  for (i = 0; i < end; i++) {
    //printf("%d kill is %d\n",builds[i].pid, kill(builds[i].pid,0));;
    if (builds[i].pid != fatherID && (kill(builds[i].pid, 0) == 0)) {

      printf("%ld %s\n", (long) builds[i].pid, builds[i].buff);

    }
  }
}

//function that handles history.
void history(build builds[], int end) {
  int i;
  for (i = 0; i <= end; i++) {
    //printf("%d kill is %d\n",builds[i].pid, kill(builds[i].pid,0));;
    if ((kill(builds[i].pid, 0) == 0)) {
      printf("%ld %s RUNNING\n", (long) builds[i].pid, builds[i].buff);
    } else
      printf("%ld %s DONE\n", (long) builds[i].pid, builds[i].buff);
  }
}
//function that handles cd
char *cdFunction(build builds[], int end, char *fatherPath) {
  struct stat stat1;
  char *temp = (char *) malloc(100);
  getcwd(temp, 100);
  char *path = (char *) malloc(100);
  int i = end;
  int status;
  //cd ~
  if (builds[i].str[1] == NULL || builds[i].str[1][0] == '~') {
    strcpy(path, "/home/");
    builds[i].str[1]++;
    if (builds[i].str[1] != NULL)
      strcat(path, builds[i].str[1]);
    //check for too many arguments
    if (builds[i].str[2] != NULL) {
      fprintf(stderr, "Error: Too many arguments\n");
      return fatherPath;
    }
    //check for directory does'nt exist.
    stat(path, &stat1);
    if (!S_ISDIR(stat1.st_mode)) {
      fprintf(stderr, "Error: No such file of directory\n");
      return fatherPath;
    }
    status = chdir(path);
  }
    // cd -
  else if (strcmp(builds[i].str[1], "-") == 0) {
    getcwd(path, 100);
    if (builds[i].str[2] != NULL) {
      fprintf(stderr, "Error: Too many arguments\n");
      return fatherPath;
    }
    stat(path, &stat1);
    if (!S_ISDIR(stat1.st_mode)) {
      fprintf(stderr, "Error: No such file of directory\n");
      return fatherPath;
    }
    status = chdir(fatherPath);
    if(status == 0 && strcmp(fatherPath,path) != 0)
      printf("%s\n",fatherPath);
  }

    //cd ..
  else if (strcmp(builds[i].str[1], "..") == 0) {
    if (builds[i].str[2] != NULL) {
      sleep(1);
      fprintf(stderr, "Error: Too many arguments\n");
      return fatherPath;
    }
    getcwd(path, 100);
    removeSlash(path);
    status = chdir(path);
  } else {
    if (builds[i].str[2] != NULL) {
      fprintf(stderr, "Error: Too many arguments\n");
      return fatherPath;
    }
    getcwd(path, 100);
    strcat(path, "/");
    strcat(path, builds[i].str[1]);
    stat(path, &stat1);
    if (!S_ISDIR(stat1.st_mode)) {
      fprintf(stderr, "Error: No such file of directory\n");
      return fatherPath;
    }
    status = chdir(path);
  }
  if (status != 0) {
    fprintf(stderr, "Error in system call\n");
    return fatherPath;
  }
  strcpy(fatherPath, temp);
  return fatherPath;
}
int main() {
  //variables
  build builds[100];
  pid_t pid;
  int i = 0;
  int fatherID = getpid();
  int j;
  int status;
  char *fatherPath = (char *) malloc(100);
  getcwd(fatherPath, 100);

  //start of while
  while (1) {
    char *buffer = (char *) malloc(100);
    char *token;
    int flag = 0;
    int historyJumpFlag = 0, exitJumpFlag = 0, cdJumpFlag = 0, jobsJumpFlag = 0, jumpFlag = 0;
    j = 0;
    char dest[110];

    printf("> ");
    builds[i].buff = (char *) malloc(100);
    fgets(builds[i].buff, 100, stdin);
    strcpy(buffer, builds[i].buff);
    strtok(builds[i].buff, "\n");


    if (strcmp(builds[i].buff, "exit") == 0) {
      builds[i].str[0] = "exit";
      exitJumpFlag = 1;
      jumpFlag = 1;
    }
    if (strcmp(builds[i].buff, "history") == 0) {
      builds[i].str[0] = "history";
      historyJumpFlag = 1;
      jumpFlag = 1;
    }
    if (strcmp(builds[i].buff, "jobs") == 0) {
      builds[i].str[0] = "jobs";
      jobsJumpFlag = 1;
      jumpFlag = 1;
    }
    //if (strchr(buffer, '&') != NULL)
    //  flag = 1;
    strtok(buffer, "\n");
    token = strtok(buffer, " ");

    //strtok while, split by space
    while (token != NULL && jumpFlag == 0) {
      builds[i].str[j] = token;
      j++;
      token = strtok(NULL, " ");
    }
    if (strcmp(builds[i].str[0], "cd") == 0) {
      cdJumpFlag = 1;
      jumpFlag = 1;
    }
    if (builds[i].str[j-1] != NULL && strcmp(builds[i].str[j - 1], "&") == 0)
      flag = 1;
    if(strcmp(builds[i].str[0],"echo")== 0){
      if(builds[i].str[1] != NULL && builds[i].str[1][0] == 34 &&
      ((!flag && builds[i].str[j-1][strlen(builds[i].str[j-1]) - 1] == 34) ||
      (flag && builds[i].str[j-2][strlen(builds[i].str[j-2]) - 1] == 34))){
        builds[i].str[1] = strstr(builds[i].buff,"\"");
        builds[i].str[1]++;
        char *e= strchr(builds[i].str[1],'\"');

        int index = (int) (e - builds[i].str[1]);
        builds[i].str[1][index] = '\0';
        builds[i].str[2] = NULL;
      }
    }

    builds[i].pid = fork();
    if (builds[i].pid < 0) {
      fprintf(stderr, "Error in system call");
    }

      // child
    else if (builds[i].pid == 0) {
      if (jumpFlag)
        break;

      if (flag)
        builds[i].str[j - 1] = NULL;
      else
        builds[i].str[j] = NULL;
      status = execvp(builds[i].str[0], builds[i].str);
      fprintf(stderr, "Error in system call");
      break;
    } else {
      signal(SIGCHLD, SIG_IGN);
      if(builds[i].pid > 0)
        printf("%d\n", builds[i].pid);
      if (jobsJumpFlag) {
        jobsFunction(builds, i, fatherID);
        flag = 1;
      }
      if (historyJumpFlag) {
        history(builds, i);
        flag = 1;
      }
      if (exitJumpFlag) {
        exit(1);
      }
      if (cdJumpFlag) {
        fatherPath = cdFunction(builds, i, fatherPath);
      }

      if (!flag) {
        waitpid(builds[i].pid, &status, 0);
      }
      flag = 0;
      i++;
    }
  }
}
