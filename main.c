/* Program Name: main.c
** Author: Adam Oberg
** Description:
** Date:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <signal.h>

/*
This fucntion is just made to display the creator of the program.
Has no functionality
*/
void start_shell(){
    printf("\n******************"
    "************************");
    printf("\n\n\n\t****Small Shell****");
    printf("\n\n\t -By: Adam Oberg-");
    printf("\n\n\n\n*******************"
    "***********************");
    char* username = getenv("USER");
    printf("\nUSER is: @%s", username);
    printf("\n");
    fflush(stdout);
    sleep(1);
}
//Function to exit the program.
void exitProgram(){
  int i = 0;
  printf("Exiting");
  fflush(stdout);
  while (i != 3) {
    printf(".");
    sleep(1);             //This is for the meme
    fflush(stdout);
    i++;
  }
  sleep(1);
  printf("\n");
  fflush(stdout);
  exit(0);
}



int main(int argc, char const *argv[]) {
  start_shell();
  exitProgram();
  return 0;
}
