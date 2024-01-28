#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 8192
static const int MAXPENDING = 5; // Maximum outstanding connection requests
using namespace std;

void DieWithError(string errorMessage){
  cout<<errorMessage<<endl;
}

/*
* Prints HELLO message to server
*/
void printHELLO(char buffer[]){
  char *token = strtok(buffer, " "); // Split string with the first occurence of " " (space)
  cout<<endl;
  cout<<"Client Message:"<<endl;
  cout<<"Version -> ";
  printf("%s\n", token);
  token = strtok(NULL, " "); // Split string with the second occurence of " " (space)
  cout<<"Type -> ";
  printf("%s\n", token);
  token = strtok(NULL, " "); // Split string with the third occurence of " " (space)
  cout<<"First Name -> ";
  printf("%s\n", token);
  token = strtok(NULL, " "); // Split string with the fourth occurence of " " (space)
  cout<<"Last Name -> ";
  printf("%s\n", token);
}

/*
* Print BYE message to server
*/
void printBYE(char buffer[]){
  char *token = strtok(buffer, " "); // Split string with the first occurence of " " (space)
  cout<<"Received final client message:"<<endl;
  cout<<endl;
  cout<<"ClientMessage:"<<endl;
  cout<<"Version -> ";
  printf("%s\n", token);
  token = strtok(NULL, " "); // Split string with the second occurence of " " (space)
  cout<<"Type -> ";
  printf("%s\n", token);
  token = strtok(NULL, " "); // Split string with the third occurence of " " (space)
  cout<<"Cookie -> ";
  printf("%s\n", token);
}

/*
* Method handles all the sending and receiving of messages from client
*/
void HandleTCPClient(int clntSocket, string cookie){
  //char empty[BUFFSIZE];
  char helloBuffer[BUFFSIZE]; // buffer to receive the string from Client
  char byeBuffer[BUFFSIZE];
  char *p1; // pointer to where the version and type fields are at
  char *p2;
  char ackBase[] = "CS332 ACK"; // Version and type fields of ACK message
  char ackMessage[BUFFSIZE]; // Buffer for combining version and type fields, and cookie
  sprintf(ackMessage, "%s %s\n", ackBase, cookie.c_str()); // Combined "CS332 ACK" plus the cookie into buffer
  ssize_t ackLength = strlen(ackMessage); // Get lenght of ACK message
  ssize_t numBytesRcvd = 0;

  // Receive HELLO message from client
  numBytesRcvd = recv(clntSocket, helloBuffer, BUFFSIZE, 0);
  if(numBytesRcvd < 0)
    DieWithError("recv() failed");

  // Check if message received is in correct format for HELLO message
  p1 = strstr(helloBuffer, "CS332 HELLO");
  p2 = strchr(helloBuffer,'\n');
  // Print Hello message if in correct format
  if(p1 != NULL && p2 != NULL)
    printHELLO(helloBuffer);
  
  helloBuffer[0] = 0;
  for(unsigned int i=0; i < sizeof(helloBuffer); i++){
     helloBuffer[i] = 0;
  }
  //Send ACK message to client
  ssize_t numBytesSent = send(clntSocket, ackMessage, ackLength, 0);
  if(numBytesSent < 0)
    DieWithError("send() failed");

  //Receive BYE message from client
  numBytesRcvd = recv(clntSocket, byeBuffer, BUFFSIZE, 0);
  if(numBytesRcvd < 0)
    DieWithError("recv() failed");

  // Check if message received is in correct fromat for BYE messages
  p1 = strstr(byeBuffer, "CS332 BYE");
  p2 = strchr(byeBuffer, '\n');
  // Print BYE message if in correct format
  if(p1 != NULL && p2 != NULL)
    printBYE(byeBuffer);

  byeBuffer[0] = 0;
  close(clntSocket); // Close client socket
}

int main(int argc, char *argv[]) {
  if(!(argc == 5)){
    cout << "Error: Usage Project2Server -s <cookie> -p <port>" << endl;
    exit(1);
  }

  // Argument parsing variables
  string cookie;
  unsigned short port;
  char c;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      c = argv[i][1];

      /* found an option, so look at next
       * argument to get the value of
       * the option */
      switch (c) {
        case 's':
          cookie = argv[i+1];
          break;
        case 'p':
          port = stoi(argv[i+1]);
          break;
      }
    }
  }

  // Create socket for incoming connections
  int servSock; // Socket descriptor for server
  if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))  < 0)
    DieWithError("socket() failed");

  // Construct local address structure
  struct sockaddr_in servAddr; // Local address
  memset(&servAddr, 0 , sizeof(servAddr)); // Zero out structure
  servAddr.sin_family = AF_INET; // IPv4 address family
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
  servAddr.sin_port = htons(port);

  //Bind to the local address
  if(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    DieWithError("bind() failed");

  // Mark the socket so it will listen for incoming connections
  if(listen(servSock, MAXPENDING) < 0)
      DieWithError("listen() failed");

  // Loop forever
  for(;;){
    struct sockaddr_in clntAddr; // Client address
    socklen_t clntAddrLen = sizeof(clntAddr); // Set lenght of client address structure

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if(clntSock < 0)
      DieWithError("accept() failed");

    HandleTCPClient(clntSock, cookie);
  }
}
