#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"

int main(int argc, char** args) {


  WSADATA wsaData;
  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;
  SOCKET ClientSocket = INVALID_SOCKET;

  struct addrinfo *result = NULL;
  struct addrinfo hints;

  int iSendResult;
  char recvbuf[DEFAULT_BUFLEN];
  int recvbuflen = DEFAULT_BUFLEN;
    
  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0) {
    printf("WSAStartup failed with error: %d\n", iResult);
    return 1;
  }
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;


  printf("before get\n");
  // Resolve the server address and port
  iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

  if ( iResult != 0 ) {
    printf("error!!\n");
    printf("getaddrinfo failed with error: %d\n", iResult);
    WSACleanup();
    return 1;
  }
    

  printf("I survived the getaddrinfo stuff!\n");
    
  // Create a SOCKET for connecting to server
  ListenSocket = socket(result->ai_family,
			result->ai_socktype,
			result->ai_protocol);
    
  if (ListenSocket == INVALID_SOCKET) {
    printf("socket failed with error: %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return 1;
  }

  u_long NonBlock = 1;
  if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR){
    printf("Setting non blocking failed: %d\n", WSAGetLastError());
    return 1;
  }
  printf("socket ok\n");

  // Setup the TCP listening socket
  iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    printf("bind failed with error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ListenSocket);
    WSACleanup();
    return 1;
  }

  freeaddrinfo(result);

  fflush(stdout);

  while (true) {
    iResult = listen(ListenSocket, SOMAXCONN);

    if (iResult == SOCKET_ERROR) {
      printf("listen failed with error: %d\n", WSAGetLastError());
      closesocket(ListenSocket);
      WSACleanup();
      return 1;
    

    }
      
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
      int error = WSAGetLastError();
      if (error != WSAEWOULDBLOCK)
	{
	  closesocket(ListenSocket);
	  WSACleanup();
	  return 1;
	}
	
    } else {

      printf("a client!!\n");
      fflush(stdout);

      u_long NonBlock = 0;
      if (ioctlsocket(ClientSocket, FIONBIO, &NonBlock) == SOCKET_ERROR){
	printf("Setting non blocking failed: %d\n", WSAGetLastError());
	return 1;
      }
      printf("set client socket to non blocking\n");
      fflush(stdout);

      // Receive until the peer shuts down the connection
      do {

	printf("trying to receive from client\n");
	fflush(stdout);
	iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	if (iResult > 0) {
	  printf("Bytes received: %d\n", iResult);

	  // Echo the buffer back to the sender
	  char sendBuf[1000];
	  ZeroMemory(sendBuf, sizeof(sendBuf));
	  std::string response = "HTTP/1.1 200 OK\n"
	    "Date: Mon, 27 Oct 2017 11:00:34 GMT+1\n"
	    "Server: TTechREST/0.0.1 (Win32)\n"
	    "Content-Length: 20\n"
	    "Connection: Closed\n"
	    "Content-Type: text/html\n\r\n\r"
	    "<h2>hallo!!!</h2>";
	  sprintf(sendBuf, response.c_str());
	  iSendResult = send( ClientSocket, sendBuf, sizeof(sendBuf), 0 );
	  if (iSendResult == SOCKET_ERROR) {
	    printf("send failed with error: %d\n", WSAGetLastError());
	    closesocket(ClientSocket);
	    WSACleanup();
	    return 1;
	  }
	  printf("Bytes sent: %d\n", iSendResult);
	  break;

	    
	}
	else if (iResult == 0)
	  printf("Connection closing...\n");
	else  {
	  printf("recv 'error': %d\n", WSAGetLastError());
	  fflush(stdout);
	  int error = WSAGetLastError();
	    
	  if (error == WSAEWOULDBLOCK || error == WSAECONNABORTED)
	    {
	      break;
	    }
	    
	}

      } while (iResult > 0);

      printf("breaking out of recv. loop\n");
      fflush(stdout);
      // shutdown the connection since we're done
      iResult = shutdown(ClientSocket, SD_SEND);
      if (iResult == SOCKET_ERROR) {
	printf("shutdown failed with error: %d\n", WSAGetLastError());
	closesocket(ClientSocket);
	WSACleanup();
	return 1;
      }
      closesocket(ClientSocket);
	

    }

    // Lets give the system some rest.
    Sleep(10);

  }

  printf("we have a client!\n");
  fflush(stdout);
    
  // No longer need server socket
  closesocket(ListenSocket);

    

    

  // cleanup
  closesocket(ClientSocket);
  WSACleanup();

  return 0;



}
