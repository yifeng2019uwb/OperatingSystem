//
// RemoteFileSystem.h
//
// Client-side remote (network) filesystem
//
// Author: Morris Bernstein
// Copyright 2019, Systems Deployment, LLC.

#if !defined(RemoteFileSystem_H)
#define RemoteFileSystem_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>



class RemoteFileSystem {
    
public:
    
    // File represents an open file object
    class File {
    public:
        RemoteFileSystem* filesystem;
        File* fd;
        
        // Destructor closes open file.
        ~File();
        
        ssize_t read(void *buf, size_t count);
        ssize_t write(void *buf, size_t count);
        off_t lseek(off_t offset, int whence);
        
    private:
        // Only RemoteFileSystem can open a file.
        friend class RemoteFileSystem;
                
        File(RemoteFileSystem* filesystem, const char *pathname, char *mode);
        
        // Disallow copy & assignment
      //  File(File const &) = delete;
       // void operator=(File const &) = delete;
    };
    
    static const int bufsiz = 8192;
    static const int eof = -1;
    
    // Connect to remote system.  Throw error if connection cannot be made
    // param host : the remote host IP address
    // param port: the remote host port number
    // param auth_token: for remote host to know the permission
    // param timeout: the socket timesout in seconds, default 5 seconds?
    RemoteFileSystem(char *host,
                     short port,
                     unsigned long auth_token,
                     int timeout);
    
    // Disconnect
    ~RemoteFileSystem();
    
    // Return new open file object.  Client is responsible for
    // deleting.
    File *open(const char *pathname, char* mode);
    
    int chmod(const char *pathname, mode_t mode);
    int unlink(const char *pathname);
    int rename(const char *oldpath, const char *newpath);
    
    int getSeqNum(); // return a packet sequence number
    ssize_t send(const void* buffer, size_t size);  // RPC senf for client
    ssize_t recv(void *buffer, size_t size);    // RPC receive for client
    
    
private:
    // File class may use private methods of the RemoteFileSystem to
    // implement their operations.  Alternatively, you can create a
    // separate Connection class that is local to your implementation.
    friend class File;
    
    int sockfd; // socket id
    struct sockaddr_in server; // serverIP address
    struct sockaddr_in from;    // client
    int length; // length of sockaddr_in
    int seq_num;  // packet id
    unsigned long auth_token;    // authentication token
    // int maxTry; // maximum retry times
          
    // Disallow copy & assignment
    RemoteFileSystem(RemoteFileSystem const &) = delete;
    void operator=(RemoteFileSystem const &) = delete;
};


#endif
