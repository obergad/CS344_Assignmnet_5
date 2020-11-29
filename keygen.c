/* Program Name: main.c
** Author: Adam Oberg
** Description:
** Date:
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

char* keygen(int keyLength){
  int i = 0;
  char* key = calloc(keyLength + 1, sizeof(char));
  int randNum;
  for (i = 0; i < keyLength; i++) {
    randNum = 65 + rand() % 27; //Add random uppercase value;
    if(randNum == 91) key[i] = ' '; // if 91 make space instead of [
    else key[i] = randNum; //Add random uppercase value;
  }
  key[i + 1] = '\n'; // Add last char = \n
  return key;
}

int main(int argc, char const *argv[]) {
  char* key;
  srand(time(NULL));
  if (argc <= 2 ) {
    int keyLength = atoi(argv[1]);
    key = keygen(keyLength);
  }
  else{
    printf("Error: No keyLength given\n");
    return EXIT_FAILURE;
  }
  printf("%s\n", key );
  free(key);
  return EXIT_SUCCESS;
}
