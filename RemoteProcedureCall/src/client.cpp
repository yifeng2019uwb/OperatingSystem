//
//  client.cpp
//  rpc
//
//  Created by Yifeng Zhang on 6/6/19.
//  Copyright © 2019 Yifeng Zhang. All rights reserved.
//

//#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.h"
#include "remotefilesystem.h"
#include <iostream>
#include <sys/stat.h>
#define MAX_MSG_SIZE 256


using namespace std;



/*
 The client take server host and port from command line as arguments
 The client will marshall and unmarshall the arguments and allocated space for results.
 The client uses methods of remotefilesystem to perform network communication to process
 open/read/write/fseek, chmod, unlink, rename file. client has timeout - 5 seconds and maimum
 retry times - 3.
 */

const int timeout = 20;

void usage(const char *prog) {
    fprintf(stderr, "usage: %s server port\n", prog);
    exit(EXIT_FAILURE);
}


int main (int argc, char* argv[]) {
    // validate arguments
    if (argc != 3) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // initialize RPC
    char *server = argv[1]; // host
    short port = atoi(argv[2]);
    
    /*
    struct addrinfo hints;
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    
    struct addrinfo *addresses;

    int result = getaddrinfo(server, port, &hints, &addresses);
    if (result != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(EXIT_FAILURE);
    }
*/
    // unsigned long auth_token = getAuthToken();
    long long auth_token = 1234;
    // create rpc to connect to server
    cout << " ------- rpc constructor ---------- " << endl;
    RemoteFileSystem rpc(server, port, auth_token, timeout);// connect
 
    // open a file
    cout << " ---------- Open file --------------" << endl;
    RemoteFileSystem::File* fd = rpc.open("file0.txt", (char*)"r+");
    
    cout << "in client class - file open : " << fd << endl;
    // cout << "in client class - rpc : " << &rpc << endl;
   //  cout << "in client class - file - fd " << fd->fd << endl;
    // cout << "in client class - file.rpc: " << fd->filesystem << endl;
    
    cout << " ---------- read from file ----------- " << endl;
    char buffer[MAX_MSG_SIZE];
    bzero(buffer, MAX_MSG_SIZE);
    
    int nread = fd->read(buffer, 3);
    if (nread < 0 ) {
        fprintf(stderr, " read file\n");
    }
    cout << "read file " << nread << " bytes data : " << buffer << endl << endl;
    
    
    cout << " ---------- write to file ----------- " << endl;

    char data[] = "Hello";
    
    int nwrite = fd->write(data, 5);
    if (nwrite < 0) {
        fprintf(stderr, "write to file \n" );
    }
    
    cout << endl;
    // cout << "write number : " << nwrite << endl;
    
    // int offset = fd->lseek(4, SEEK_SET);
    // cout << "fseek = " << offset << endl;
   
    /*
    cout << " --- try read again ---- " << endl;
    cout << "in client class - file open : " << fd << endl;
    cout << "in client class - rpc : " << &rpc << endl;
    cout << "in client class - file - fd " << fd->fd << endl;
    cout << "in client class - file.rpc: " << fd->filesystem << endl;
     */
    
    bzero(buffer, MAX_MSG_SIZE);
    nread = fd->read(buffer, 5);
    if (nread < 0 ) {
        fprintf(stderr, " read file\n");
    }
    cout << "read file again " << nread << " bytes data : " << buffer << endl << endl;
    
    int offset = fd->lseek(0, SEEK_SET);
    if (offset < 0) {
         fprintf(stderr, "seek file\n");
    }

    cout << " read again after lseek - back to beginning of file " << endl;
    bzero(buffer, MAX_MSG_SIZE);
    nread = fd->read(buffer, 5);
    if (nread < 0 ) {
        fprintf(stderr, " read file\n");
    }
    cout << " read file " << nread << " bytes data : " << buffer << endl << endl;
    
    /*
  
    //write to file
    n = file->write(reinterpret_cast<void*>("Hello"), 5);
    if ( n < 0) {
      fprintf(stderr, "write failed\n");
      
    }else {
      cout << "write to file " << n << " bytes" << endl;
    }
    
    // lseek
    n = file->lseek(5, SEEK_CUR);

    if ( n < 0) {
      fprintf(stderr, "lseek failed\n");
      
    }else {
      cout << "current file position after lseek " << n  << endl;
    }    
      */
    
    // chmod
    cout << " ------------ chmod file -------------" << endl;
    int result = chmod("file0.txt", 777);// change file mode: all users can r/w/e
    if (result < 0) {
          fprintf(stderr, "chmod file\n");
    }
    
    // rename
    cout << " ------------ rename file -------------" << endl;
    result = rename("file0.txt", "file_new.txt");
    if (result < 0) {
        fprintf(stderr, "rename file\n");
    }

    // unlink
    cout << " ------------ unlink file -------------" << endl;
    result = unlink("file_new.txt");// change file mode: all users can r/w/e
    if (result < 0) {
        fprintf(stderr, "unlink file\n");
    }

    
}






