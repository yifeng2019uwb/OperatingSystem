//
//  server.cpp
//  rpc
//
//  Created by Yifeng Zhang on 6/6/19.
//  Copyright © 2019 Yifeng Zhang. All rights reserved.
//

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include "server.h"
// #include "serialize.h"
#include <iostream>
#include <sys/stat.h>

using namespace std;

#define MAX_MSG_SIZE 256
#define BUF_SIZE 512


void usage(const char *prog) {
    fprintf(stderr, "usage: %s server port\n", prog);
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
  
    if (argc != 2) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int sockfd, length, fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    // char buf[BUF_SIZE];
    
    // socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("creating socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Server socket created \n ");
    // socklen_t socklen = sizeof(server_addr);

    length = sizeof(server);
    bzero(&server,length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
    
    // bind the port to socket
    int rc = bind(sockfd, (struct sockaddr *) &server, length);
    if (rc < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    
    printf("Server binding created \n ");
    
    fromlen = sizeof(from);
    
    /*
    socklen_t n;

    struct sockaddr_in client_address;
    socklen_t len = sizeof(client_address);

 
    // cout << " test connection - recv and send " << endl;
    bzero(buf, BUF_SIZE);
    n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*) &client_address,&len);
    if ( n < 0) {
        fprintf(stderr, "recvfrom failed\n");
    }
    
    //write(1, "Recived a datagram: ", 21);
    //write(1, buf, n);
    
    cout << "Received a datagram : " << buf << endl;
    n = sendto(sockfd, "Got your message\n", 17, 0, (struct sockaddr*)&from, fromlen);
    if (n < 0) {
        fprintf(stderr, "send back failed\n");
    }
*/
    
    // try main process
    // cout << "test file operations" << endl;
    while (1) {
        Packet pkt;
        // len = sizeof(Packet);

        // receive packet from client size and check if recvfrom succeeds
        int inBytes = recvfrom(sockfd, (void*)&pkt, sizeof(Packet), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen);
        
        cout << "done recv, bytes-in: " << inBytes << endl;
        if ( inBytes < 0) {
            fprintf(stderr, "recvfrom failed \n");
            exit(EXIT_FAILURE);
        }
       /*
        cout << " packet info" << endl;
        cout << "auth_token: " << pkt.auth_token << endl;
        cout << " seq - num: " << pkt.seq_num << endl;;
        cout << " op : " << pkt.op << endl;
        cout << " pathname : " << pkt.pathname << endl;
        cout << " size : " << pkt.size << endl;
        */
        
        operation op = pkt.op;
        Message msg;
        //bzero(msg, sizeof(msg));
        msg.seq_num = pkt.seq_num;
        
        switch (op) {
            case OPEN:
            {
                cout << " ------------  operation is OPEN ------------- " << endl;
                //char* filename = deserialize_chararr(p);
                char* filename = (char*)malloc(32);
                memcpy(filename, pkt.pathname, 32);
                cout << "open file : " << filename << endl;
                FILE * fd;
                
                if (pkt.size == 1) {
                    fd = fopen(filename, "r");
                    break;
                } else {
                    if (pkt.size == 2) {
                    // cout << " File open in read/write mode" << endl;
                    fd = fopen(filename, "r+");
 \
                    }else {
                        if ( pkt.size == 3) {
                            // write
                            fd = fopen(filename, "w");
                        } else {
                            if ( pkt.size == 4) {
                                fd = fopen(filename,"w+");
                            } else {
                                msg.result = -1;
                                break;
                            }
                        }
                    }
                }
                
                msg.result = (uint64_t)fd;
                // cout << "fd = " << fd << endl;
                // cout << "msg-result : " << msg.result << endl;
                //  cout << "conver back : " << (FILE*)msg.result << endl;
                cout << endl;
               
                // cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            case READ:
            {
                cout << " ------------ operation is READ ------------- " << endl;
                void* ptr = (void*)pkt.filedescriptor; // fd
                size_t size = (size_t)pkt.size;
                char data[MAX_MSG_SIZE];
                bzero(data, MAX_MSG_SIZE);
                size_t result = fread(data, 1, size, (FILE*)ptr);
                msg.result = result;
                cout << "data read from file : " << data << endl;
                
                if (result > 0) {
                    //
                    memcpy(msg.buffer, data, result);
                }
                cout << endl;
                
               // cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            case WRITE :
            {
                cout << " --------------- operation is WRITE ---------------- " << endl;
                void* ptr = (void*)pkt.filedescriptor; // fd
                size_t size = (size_t)pkt.size;
               // char data[size];
                cout << "write data to file: " << pkt.buf << endl;
                const void* ptrbuf = strdup(pkt.buf);
                size_t result = fwrite(ptrbuf, 1, size, (FILE*)ptr);
                msg.result = result;
                cout << "write " << result <<  " bytes to file" << endl;
                cout << endl;
                
               // cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            case LSEEK :
            {
                cout << " ------------------ operation is LSEEK -------------- " << endl;
               // cout << "file descriptor: " << pkt.filedescriptor << endl;
                void* ptr = (void*)pkt.filedescriptor; // fd
                size_t offset = (size_t)pkt.size;
                int whence = (int)pkt.buf[0];
                //cout << "whence = " << whence << endl;
                //cout << "offset = " << offset << endl;
               // cout << "fiele addres : " << (FILE*)ptr << endl;
                size_t result = fseek((FILE*)ptr, offset, whence);
                msg.result = result;
                //cout << "fseek result = " << result << endl;
                cout << " -----------------------------------------------------" << endl << endl;
                
            }
                break;
            case CLOSE:
            {
                cout << " ------------ operation is CLOSE --------------- " << endl;
                void* ptr = (void*)pkt.filedescriptor; // fd
                
                int result = fclose((FILE*)ptr);
                msg.result = result;
                if (result == 0) {
                    cout << "close file successed" << endl;
                }else {
                    cout << "close file failed" << endl;
                }
                cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            case CHMOD :
            {
                cout << " ------------ operation is CHMOD --------------- " << endl;
                msg.result = chmod(pkt.pathname, (mode_t)pkt.size);
                
                cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            case UNLINK:
            {
                cout << " ------------ operation is CHMOD --------------- " << endl;
                msg.result = unlink(pkt.pathname);
                
                cout << " -----------------------------------------------------" << endl << endl;

                
            }
                break;
            case RENAME :
            {
                cout << " ------------ operation is CHMOD --------------- " << endl;
                char newname[32];
                memcpy(newname, pkt.buf, 32);
                msg.result = rename(pkt.pathname, newname);
                                
                cout << " -----------------------------------------------------" << endl << endl;

            }
                break;
            default:
            {
               // do nothing - CLOSE FILE?
                
            }
        }// end switch
 
        int outBytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&from, fromlen);
        cout << "done sendto outBytes: " << outBytes << endl;
        if ( outBytes < 0) {
            fprintf(stderr, "error send to client\n");
            exit(EXIT_FAILURE);
        }
        
    } // end while

    
    exit(0);
}

