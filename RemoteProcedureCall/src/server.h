//  serilization.hpp
//  rpc
//  Created by Yifeng Zhang on 6/6/19.
//  Copyright © 2019 Yifeng Zhang. All rights reserved.
//

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
//#include <linux/random.h>
#include <cstdlib>  // for srand() and rand()

#define MAX_BUF_SIZE 256

// define operation/command
enum operation {
    OPEN = 0,
    READ = 1,
    WRITE = 2,
    LSEEK = 3,
    CHMOD = 4,
    UNLINK = 5,
    RENAME = 6,
    CLOSE = 7
};


// define packet sending from client to server
/* struct Packet size = 8 + 4 + 4 + 32 + 8 + 8 + 256  = 320 bytes */
typedef struct s_Packet {
    long long auth_token;  /* client authorization token for having permission to do sth - 8bytes */
    int seq_num;   /* sequence number to indicate which client request the server is responding to - 4bytes*/
    operation op;       /* operation: read/write, etc - size: int - 4 bytes */
    char pathname[32]; // file pathname - 32 bytes
    uint64_t filedescriptor;   // 8 bytes
    long long size;     /*  buf length, offset, file descriptor, mode - 8 bytes */
    char buf[MAX_BUF_SIZE];      /* for write or newpathname - 1000 bytes */
}Packet;


// define packet sending from server to client
/* total size = 4 + 8 + 256 = 268 + 4 =  272 */
typedef struct s_Message {
    int seq_num;        // int - 4 bytes
    // operation op;      // int - 4 bytes
    uint64_t result; // buf size, offset, ssize_t/size_t for r/w,  file*
    char buffer[MAX_BUF_SIZE];  // for read
}Message;




// Simulate authentication & authorization using a 64 bit number as an "auth" token
// Normally, this would be provided by an credentialing service or through some other
// suitable mechanism, you may use getrandom(2) to generate a random number
// getrandom(autoTokenList, 10, GRND_RANDOM)
/*
uint64_t authTokenList[5] = {1234001, 1234002, 1234003, 1234004, 1234005};
long getAuthToken = authTokenList[rand()%5];
bool isAuthToken(uint64_t authToken) {
    for (int i = 0; i < 5; i++ ) {
        if ( authTokenList[i] == authToken ) return true;
    }
    return false;
}
*/

//getrandom(reinterpret_cast<void*)authTokenList, 5, 0);

// client side: create packet& get_message
// server side: create_message & get_packet
Packet create_packet(uint64_t auth_token, uint64_t seq_num, operation op, uint64_t filep, char* pathname, int mode, long count, char* buf);

//Packet get_packet(void* buffer);

Message create_message(uint64_t seq_num, operation op, uint64_t result, char* buffer );

//Message get_message(void* buffer);



#endif
