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
#include <netinet/in.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>


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

char *encryptMessage(char* buffer){
   int a = 0;
   char *key = calloc(MAXCHAR + 1, sizeof(char));
   char *verEnc = calloc(MAXCHAR + 1, sizeof(char));
   char *toEncrypt = calloc(MAXCHAR + 1, sizeof(char));

   int b = 0;

   verEnc = strtok(buffer,"\n");
   if (strcmp(verEnc,"enc ") != 0) {
     return("You cannot connect to that server!\n end \n");
   }
   else{
     toEncrypt = strtok(NULL,"\n" );
     key = strtok(NULL,"\n" );
      for (size_t i = 0; i < strlen(toEncrypt); i++) {
         a = toEncrypt[i]; //Get ascii value for A
         if( a == 32) a = 91;
         b = key[i]; //Get ascii value for b
         if (b == 32) b = 91;
         a = abs(a) - 65; //Subtract 65 from the abs of a
         b = abs(b) - 65; //Subtract 65 from the abs of b
         a += b;  //Add key and buffer value
         a = a % 27; // take mod 26 of a
         a += 65;
         if(a == 91 ) a = 32;
         toEncrypt[i] = a; // put ascii value of a + 65 back into array at i
      }
   }
   strcat(toEncrypt, "\n end \n");
   return(toEncrypt);
}


int main(int argc, char *argv[]){
   int connectionSocket, charsRead;
   char buffer[MAXCHAR], key[MAXCHAR], tempStr[MAXCHAR];
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
      //==============================================================================
      // Lets fork this up
      //==============================================================================

      pid_t spawnpid = -5;
      int childpid = 0;
      int childstatus = 0;

      spawnpid = fork();
      switch(spawnpid){
	 case -1:
	    perror("fork is a no go");
	    // exit(1);
	    break;
	 case 0:
	    //==============================================================================
	    // Done forking this up
	    //==============================================================================

	    // printf("SERVER: Connected to client running at host %d port %d\n",
	    //                       ntohs(clientAddress.sin_addr.s_addr),
	    //                       ntohs(clientAddress.sin_port));

	    // Get the message from the client and display it
	    memset(buffer, '\0', strlen(buffer));
	    // Read the client's message from the socket
      //==============================================================================
      // Recv loop error is here
	    while (strstr(buffer,"\n end \n") == NULL) {
        memset(tempStr, '\0', strlen(tempStr));
	       errno = 0;
         // printf("recv in enc_server\n");
	       charsRead = recv(connectionSocket, tempStr, sizeof(tempStr), 0);
	       if (charsRead < 0){
		         error("SERVER: ERROR reading from socket");
	       }
	       if (buffer[0] == '\0')
		  strcpy(buffer, tempStr);
	       else
		  strcat(buffer, tempStr);
	    }
      //==============================================================================

	    //==========================================================================
	    // char *saveptr;
	    // // The first token is the title
	    // char *temp = strtok_r(buffer, "\n", &saveptr);
	    // char *key = strtok_r(saveptr, "\n", &saveptr);
	    // char* ciphertext = calloc(MAXCHAR + 1, sizeof(char*));
	    // //this whole snipit separates key and buffer recived from server
      fflush(stdout);
      // printf("buffer@enc_serverB4enc:%s\n",buffer );
	    strcpy(buffer,encryptMessage(buffer));
      fflush(stdout);

      // printf("buffer@enc_serverAF4enc:%s\n",buffer );

	    // //==========================================================================
	    int sizeLeft = strlen(buffer);
	    int charsSent = 0;
	    int i = 0;
	    // Send a Success message back to the client
	    while (i < strlen(buffer)) {
        // printf("sending in enc_server\n");
	       charsSent = send(connectionSocket, buffer, sizeLeft , 0);
         if (charsSent < 0) error("ERROR writing to socket");
	       i += charsSent;
	       sizeLeft -= charsSent;
	    }
	    // printf("charsSent:%i\n",charsSent);


	    // Close the connection socket for this client
	    close(connectionSocket);
      exit(0);
   break;
   default:
    waitpid(spawnpid, &childstatus, 1);
   break;
 }
}
// Close the listening socket
close(listenSocket);
return 0;
}
