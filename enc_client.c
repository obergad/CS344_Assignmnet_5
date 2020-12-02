/* Program Name: enc-client.c
** Author: Adam Oberg
** Description:
** Date: 11/29/2020
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>


#define MAXCHAR 150001
int bufferLength = 0;
/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

//==============================================================================
// Error function used for reporting issues
//==============================================================================
void error(const char *msg) {
  perror(msg);
  exit(0);
}
//==============================================================================
// Set up the address struct
//==============================================================================
void setupAddressStruct(struct sockaddr_in* address,
                                    int portNumber,
                                    char* hostname){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}
//==============================================================================
// Get the message to send
//==============================================================================
char *getMessage(char* key, char* plaintext){
  char *myString = calloc(MAXCHAR, sizeof(char));
  char *tempStr_pt = calloc(MAXCHAR, sizeof(char));
  char *tempStr_ky = calloc(MAXCHAR, sizeof(char));
  FILE *myFile;
  int keySize, textSize;
  size_t len = MAXCHAR;
  struct stat st;
  strcpy(myString, "enc \n");
  myFile = fopen(plaintext, "r"); //open plaintext file and put
  fgets(tempStr_pt, len, myFile);
  fclose(myFile);
  for (int i = 0; i < strlen(tempStr_pt); i++) {
    if(tempStr_pt[i] != ' ' && (tempStr_pt[i] > 'Z' || tempStr_pt[i] < 'A')){
      if(tempStr_pt[i] != '\n'){
        fprintf(stderr, "Error: The plaintext has an invalid char in it at char [%i] \n", i);
        exit(1);
      }
    }
  }
  // stat(plaintext, &st);
  // len = st.st_size;
  textSize = strlen(tempStr_pt);
  strcat(myString, tempStr_pt); //cat plaintext into string
  bufferLength = strlen(tempStr_pt);
  //==========================================================
  strcat(myString, "\n");
  myFile = fopen(key, "r");
  // stat(plaintext, &st);
  // len = st.st_size;
  fgets(tempStr_ky, len, myFile);

  fclose(myFile);
  for (int i = 0; i < strlen(tempStr_ky); i++) {
    if(tempStr_ky[i] != ' ' && (tempStr_ky[i] > 'Z' || tempStr_ky[i] < 'A')){
      if(tempStr_ky[i] != '\n'){
        fprintf(stderr, "Error: The key has an invalid char in it at char [%i] \n", i);
        exit(1);
      }
    }
  }
  keySize = strlen(tempStr_ky);
  strcat(myString, tempStr_ky); // cat key to string
  strcat(myString, "\n end \n");
  // printf("myString:%s\n",myString );

  if (keySize < textSize) {
    fprintf(stderr, "Error: The text size is larger than the key.\n");
    exit(1);
  }
  return(myString);
}
//==============================================================================
// send the message
//==============================================================================
int sendMessage(int socketFD, char* buffer){
  int charsWritten;
  // Write to the server
  printf("send in enc_client\n");
  charsWritten = send(socketFD, buffer, strlen(buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  return(charsWritten);
}
//==============================================================================
// recive the message
//==============================================================================
void readMessage(int socketFD, char* buffer, int bufferLen){
  int charsRead = 0;
  int sizeLeft = bufferLen;
  int i;
  char* tempStr = calloc(MAXCHAR, sizeof(char));
  memset(buffer, '\0', MAXCHAR);
  // Send a Success message back to the client
  //charsRead < bufferLen
  while (strstr(buffer,"\n end \n") == NULL) {
    memset(tempStr, '\0', strlen(tempStr));
     errno = 0;
     printf("recv in enc_client\n");
     charsRead = recv(socketFD, tempStr, sizeof(tempStr), 0);

     // printf("tempStr@enc_server159:%s\n", tempStr);
     if (charsRead < 0){
         error("SERVER: ERROR reading from socket");
     }
     if (buffer[0] == '\0')
  strcpy(buffer, tempStr);
     else
  strcat(buffer, tempStr);
  }
  // printf("charsread: %i\n", charsRead);
  // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
  fflush(stdout);
  buffer = strtok(buffer, "\n");
  printf("%s\n",buffer);

}
//==============================================================================
// Main
//==============================================================================
int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  char* hostname = "localhost";
  struct sockaddr_in serverAddress;
  char buffer[MAXCHAR], key[MAXCHAR], tempStr[MAXCHAR];
  memset(buffer, '\0', strlen(buffer));

  // Check usage & args
    if (argc < 3) {
      fprintf(stderr,"USAGE: %s  port\n", argv[0]);
      exit(0);
    }
    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0){
      error("CLIENT: ERROR opening socket");
    }
   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), hostname);

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress,
                              sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  // Clear out the buffer array
  // // Remove the trailing \n that fgets adds
  strcpy(buffer, getMessage(argv[2], argv[1]));
  // buffer[strcspn(buffer, "\n")] = '\0';
  // Send message to server
  // Write to the server
  int bufferLen = sendMessage(socketFD, buffer);
  // // Get return message from server
  // // Clear out the buffer again for reuse
  // memset(buffer, '\0', strlen(buffer));
  // // Read data from the socket, leaving \0 at end
  readMessage(socketFD, buffer, MAXCHAR);
  // printf("buffer@client207: %s\n", buffer);
  // Close the socket
  close(socketFD);

  return 0;
}
