/* Program Name: enc-server.c
** Author: Adam Oberg
** Description:
** Date: 11/29/2020
*/

/*
This program is the encryption server and will run in the background as a daemon.

      *Its function is to perform the actual encoding, as described above in the
       Wikipedia quote.
 --This program will listen on a particular port/socket, assigned when it is
  first ran (see syntax below).

 --Upon execution, enc_server must output an error if it cannot be run due to a
  network error, such as the ports being unavailable.

 --When a connection is made, enc_server must call accept to generate the socket
  used for actual communication, and then use a separate process to handle the
  rest of the servicing for this client connection (see below), which will occur
   on the newly accepted socket.

 --This child process of enc_server must first check to make sure it is
  communicating with enc_client (see enc_client, below).

 --After verifying that the connection to enc_server is coming from enc_client,
  then this child receives plaintext and a key from enc_client via the connected
  socket.

 --The enc_server child will then write back the ciphertext to the enc_client
    process that it is connected to via the same connected socket.

    *Note that the key passed in must be at least as big as the plaintext.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXCHAR 150001
// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

char *encryptMessage(char* buffer, char* key){
  char ciphertext[MAXCHAR];
  int a, b;
  for (size_t i = 0; i < strlen(buffer); i++) {
     a = buffer[i]; //Get ascii value for A
     b = key[i]; //Get ascii value for b
     a = abs(a) - 65; //Subtract 65 from the abs of a
     b = abs(b) - 65; //Subtract 65 from the abs of b
     a += b;  //Add key and buffer value
     a = a % 26; // take mod 26 of a
     buffer[i] = a + 65; // put ascii value of a + 65 back into array at i
  }
  strcpy(ciphertext, buffer);
}


int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[MAXCHAR], key[MAXCHAR];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);


  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket,
                (struct sockaddr *)&clientAddress,
                &sizeOfClientInfo);
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    printf("SERVER: Connected to client running at host %d port %d\n",
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', MAXCHAR);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, MAXCHAR - 1, 0);
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    //==========================================================================
    char *saveptr;
    // The first token is the title
    char *temp = strtok_r(buffer, "\n", &saveptr);
    char *key = strtok_r(saveptr, "\n", &saveptr);
    char* ciphertext;
    //this whole snipit separates key and buffer recived from server
    strcpy(buffer, temp);
    ciphertext = encryptMessage(buffer, key);
    //==========================================================================
    int sizeLeft = strlen(ciphertext);
    int charsSent = 0;
    int i = 0;
    // Send a Success message back to the client
    printf("Message to send to Client: %s\n", ciphertext);
    while (i < strlen(ciphertext)) {
      charsSent = send(connectionSocket, ciphertext, sizeLeft , 0);
      i += charsSent;
      sizeLeft -= charsSent;
    }
    if (charsSent < 0){
      error("ERROR writing to socket");
    }

    // Close the connection socket for this client
    close(connectionSocket);
  }
  // Close the listening socket
  close(listenSocket);
  return 0;
}
