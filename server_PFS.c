// client_PFS.c
// Authors: Luke Connors and Kevin Colin

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define MAXBUFSIZE 2048
#define MAXFILES 1024
#define MAXCLIENTS 64

struct client {
  char name[MAXBUFSIZE];
  char ip[MAXBUFSIZE];
  int port;
};

struct file {
  char name[MAXBUFSIZE];
  int size;
  struct client location;
};

struct file masterFileList[MAXFILES];
struct client myClients[MAXCLIENTS];
int cli_socketfds[MAXCLIENTS];

int initConnection(int cli_socketfd, int *sizeMyClients, int* sizeMasterFileList);

int addClient(char name[MAXBUFSIZE], char ip[MAXBUFSIZE], int port, int *sizeMyClients);

void removeClient(char name[MAXBUFSIZE], int *sizeMyClients, int *sizeCliSocketfds, int *sizeMasterFileList);

void addFile(char *name, char *size, char *source, char *ip, char *port, int *sizeMasterFileList);

void removeFile(int index, int *sizeMasterFileList);

void sendFileList(int socketfd, int *sizeMasterFileList);


int main (int argc, char * argv[]) {
  
  int sizeMasterFileList = 0;
  int sizeMyClients = 0;
  int serv_socketfd = 0;
  int accept_socketfd = 0;

  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  unsigned int cli_len = 0;

  //int cli_socketfds[MAXCLIENTS];
  int sizeCliSocketfds;
  
  if (argc < 2) {
    printf("USAGE:  <Port>\n");
    exit(1);
  }

  // set up tcp socket to listen for connections
  memset(&serv_addr, '0', sizeof(serv_addr));

  // init addr with port provided in argv
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  // create socketfd and set SO_REUSEADDR
  serv_socketfd = socket(AF_INET, SOCK_STREAM, 0);
  int optVal = 1;
  setsockopt(serv_socketfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
  
  // bind socket
  if(bind(serv_socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
    printf("Bind Failed: %s\n", strerror(errno));
    return 1;
  }; 
  // setup socket to listen
  if(listen(serv_socketfd, MAXCLIENTS)){
    printf("Listen Failed: %s\n", strerror(errno));
    return 1;
  };

  // set socket to non-blocking
  fcntl(serv_socketfd, F_SETFL, O_NONBLOCK);

  int i, j;
  char buffer[1];
  while(1) {
    cli_len = sizeof(cli_addr);
    if((accept_socketfd = accept(serv_socketfd, (struct sockaddr*)&cli_addr, &cli_len)) > 0) {
	 
      // add client to list of client socketfds
      cli_socketfds[sizeCliSocketfds] = accept_socketfd;
      sizeCliSocketfds++;

      // register client will return true if client added succesfully and the client 
      // updated the master file list
      if(initConnection(accept_socketfd, &sizeMyClients, &sizeMasterFileList)) {
	// forward new file list to all clients
	for(i = 0; i < sizeMyClients; i++) {
	  sendFileList(cli_socketfds[i], &sizeMasterFileList);
	}
      }

      // set socket to non-blocking
      fcntl(accept_socketfd, F_SETFL, O_NONBLOCK);
    }
    
    for(i = 0; i < sizeMyClients; i++) {
      bzero(buffer, sizeof(buffer));
      if(recv(cli_socketfds[i], buffer, sizeof(buffer), 0) > 0) {
	// process command
	if(strstr(buffer, "l") != NULL) {
	  sendFileList(cli_socketfds[i], &sizeMasterFileList);
	}
	else if(strstr(buffer, "e") != NULL) {
	  removeClient(myClients[i].name, &sizeMyClients, &sizeCliSocketfds, &sizeMasterFileList);
	  // forward new file list to all clients
	  for(j = 0; j < sizeMyClients; j++) {
	    sendFileList(cli_socketfds[j], &sizeMasterFileList);
	  }
	}
      }
    }
  }

  return 0;
}


// rerturn 1 if add files to master list, 0 if did not add files to master list
int initConnection(int cli_socketfd, int* sizeMyClients, int* sizeMasterFileList) {
  char name[MAXBUFSIZE], size[MAXBUFSIZE], source[MAXBUFSIZE], ip[MAXBUFSIZE], port[MAXBUFSIZE];
  char buffer[MAXBUFSIZE];
  bzero(buffer, sizeof(buffer));
  bzero(name, sizeof(name));
  bzero(ip, sizeof(ip));
  bzero(port, sizeof(port));
  
  //wait for client name
  if(recv(cli_socketfd, buffer, sizeof(buffer), 0) > 0) {
    // parse buffer into strings
    int i = 0;
    char* start;
    char* end;
    start = buffer;
    end = strchr(buffer, '|');
    while(start != end) {
      name[i] = *start;
      i++;
      start++;
    }
    i = 0;
    start++;
    end = strchr(end+1, '|');
    while(start != end) {
      ip[i] = *start;
      i++;
      start++;
    }
    end++;
    strcpy(port, end);

    // add client to list of clients
    bzero(buffer, sizeof(buffer));
    if(addClient(name, ip, atoi(port), sizeMyClients)) {;
      strcat(buffer, "success"); 
      if(send(cli_socketfd, buffer, sizeof(buffer), 0) <= 0) {
	printf("Send name resgistration response Failed: %s\n", strerror(errno));
	return 0;
      }
    }
    else {
      strcat(buffer, "failure"); 
      if(send(cli_socketfd, buffer, sizeof(buffer), 0) <= 0) {
	printf("Send name resgistration response Failed: %s\n", strerror(errno));
	return 0;
      }
      return 0;
    }
    

  }
  else {
    printf("Recive Name Failed: %s\n", strerror(errno));
    return 0;
  }

  
  bzero(buffer, sizeof(buffer));
  //wait for client files
  if(recv(cli_socketfd, buffer, sizeof(buffer), 0) > 0) {

    // client has no files to add to master list
    if(strlen(buffer) == 0) {
      bzero(buffer, sizeof(buffer));
      strcat(buffer, "success"); 
      if(send(cli_socketfd, buffer, sizeof(buffer), 0) <= 0) {
	printf("Send file resgistration response Failed: %s\n", strerror(errno));
	return 0;
      }
      return 0;
    }

    char* start = buffer;
    char* end = buffer;
    char* comma = buffer;

    while((start = strchr(start, '<')) != NULL) {
      end = strchr(end + 1, '>');

      bzero(name, sizeof(name));
      bzero(source, sizeof(source));
      bzero(ip, sizeof(ip));
      bzero(port, sizeof(port));

      start++;

      comma = strchr(start, ',');
      strncpy(name, start, comma-start);
      comma++;
      start = comma;

      comma = strchr(start, ',');
      strncpy(size, start, comma-start);
      comma++;
      start = comma;

      comma = strchr(start, ',');
      strncpy(source, start, comma-start);
      comma++;
      start = comma;

      comma = strchr(start, ',');
      strncpy(ip, start, comma-start);
      comma++;
      start = comma;

      strncpy(port, start, end-start);
      
      // add files to master list of files
      addFile(name, size, source, ip, port, sizeMasterFileList);
    }

    return 1;
  }
  else {
    printf("Recive Files Failed: %s\n", strerror(errno));
    return 0;
  }

  return 1;
}


int addClient(char name[MAXBUFSIZE], char ip[MAXBUFSIZE], int port, int* sizeMyClients) {
  // return 1 if client successfully added
  // return 0 if client already exists
  int i;

  for(i = 0; i < *sizeMyClients; i++) {
    if(strcmp(myClients[i].name, name) == 0) {
      return 0;
    }
  }
  
  strcpy(myClients[*sizeMyClients].name, name);
  strcpy(myClients[*sizeMyClients].ip, ip);
  myClients[*sizeMyClients].port = port;
  (*sizeMyClients)++;
  return 1;
}

void removeClient(char name[MAXBUFSIZE], int* sizeMyClients, int* sizeCliSocketfds, int* sizeMasterFileList) {
  int i, j;
  int found = 0;

  for(i = 0; i < *sizeMyClients; i++) {
    if(!found) {
      if(strcmp(myClients[i].name, name) == 0) {
	found = 1;
	for(j = 0; j < *sizeMasterFileList; ) {
	  if(strcmp(myClients[i].name, masterFileList[j].location.name) == 0) {
	    removeFile(j, sizeMasterFileList);
	  } 
	  else {
	    j++;
	  }
	}
	close(cli_socketfds[i]);
      }
    }
    else {
      strcpy(myClients[i - 1].name, myClients[i].name);
      strcpy(myClients[i - 1].ip, myClients[i].ip);
      myClients[i - 1].port = myClients[i].port;
      cli_socketfds[i - 1] = cli_socketfds[i];
    }
  }
  
  if(found) {
    (*sizeMyClients)--;
    (*sizeCliSocketfds)--;
  }
}

void addFile(char *name, char *size, char *source, char *ip, char *port, int *sizeMasterFileList) {
  strcpy(masterFileList[*sizeMasterFileList].name, name);
  masterFileList[*sizeMasterFileList].size = atoi(size);
  strcpy(masterFileList[*sizeMasterFileList].location.name, source);
  strcpy(masterFileList[*sizeMasterFileList].location.ip, ip);
  masterFileList[*sizeMasterFileList].location.port = atoi(port);
  (*sizeMasterFileList)++;
}


void removeFile(int index, int *sizeMasterFileList) {
  int i;
  for(i = index + 1; i < *sizeMasterFileList; i ++) {
    strcpy(masterFileList[i - 1].name, masterFileList[i].name);
    masterFileList[i - 1].size = masterFileList[i].size;
    strcpy(masterFileList[i - 1].location.name, masterFileList[i].location.name);
    strcpy(masterFileList[i - 1].location.ip, masterFileList[i].location.ip);
    masterFileList[i - 1].location.port = masterFileList[i].location.port;
  }
  (*sizeMasterFileList)--;
}

void sendFileList(int socketfd, int *sizeMasterFileList) {
  char buffer[MAXBUFSIZE];
  bzero(buffer, sizeof(buffer));
  char currSize[MAXBUFSIZE];
  char currPort[MAXBUFSIZE];
  int i;
  
  for(i = 0; i < *sizeMasterFileList; i++) {
    bzero(currSize, sizeof(currSize));
    bzero(currPort, sizeof(currPort));

    strcat(buffer, masterFileList[i].name);
    strcat(buffer, " || ");
    sprintf(currSize, "%d", masterFileList[i].size);
    strcat(buffer, currSize);
    strcat(buffer, " || ");
    strcat(buffer, masterFileList[i].location.name);
    strcat(buffer, " || ");
    strcat(buffer, masterFileList[i].location.ip);
    strcat(buffer, " || ");
    sprintf(currPort, "%d", masterFileList[i].location.port);
    strcat(buffer, currPort);
    strcat(buffer, "\n");
  }

  if(send(socketfd, buffer, sizeof(buffer), 0) <= 0) {
    printf("Send file list Failed: %s\n", strerror(errno));
    return;
  }

  return;
}
