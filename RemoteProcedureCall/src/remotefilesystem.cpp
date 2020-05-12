

#include "remotefilesystem.h"
#include <fcntl.h>  // for size_t
#include <iostream>
#include <cassert>
#include <cstdarg>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "server.h"
#include <sys/types.h>

using namespace std;

#define MAX_MSG_SIZE 256

// destructor, close file
RemoteFileSystem::File::~File()
{
   // fclose(f_), create pkt and send it to server and ask to close file;
    Packet pkt;
    pkt.auth_token = filesystem->auth_token;
    pkt.seq_num = filesystem->getSeqNum();
    pkt.op = CLOSE;
    pkt.filedescriptor = (long long)fd;
    pkt.size = 0;
    
    // call rpc send() to send packet to server
    size_t outBytes = filesystem->send(&pkt, (size_t)(sizeof(Packet)));
    if (outBytes < 0) {
        perror("send failed\n");
    }
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = filesystem->recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        perror("recv failed");
    }
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
    }
}


// read from file
ssize_t RemoteFileSystem::File::read(void * buf, size_t count)
{
   //  cout << " ----------------- file.read ------------------ " << endl;
    // cout <<  "print filesytem in this file: "  << this->filesystem << endl;

  // create packet first
    Packet pkt;
    pkt.auth_token = filesystem->auth_token;
    pkt.seq_num = filesystem->getSeqNum();
    pkt.op = READ;
    pkt.filedescriptor = (long long)fd;
    bzero(pkt.buf, MAX_MSG_SIZE);

    pkt.size = count;
    /*
    cout << " ***** packet info ****** " << endl;
    // cout << "auth_token: " << pkt.auth_token << endl;
    cout << "seq - num: " << pkt.seq_num << endl;;
    cout << "op : " << pkt.op << endl;
    cout << "file descriptor : " << pkt.filedescriptor << endl;
    cout << "size : " << pkt.size << endl;
    // cout << "packet size:" << sizeof(Packet) << endl;
     */
    
    // call rpc send() to send packet to server
    size_t outBytes = filesystem->send(&pkt, (size_t)(sizeof(Packet)));
    if (outBytes < 0) {
        perror("send failed\n");
        return -1;
    }
    
    // call recv() to receive message from server
    Message msg;
    bzero(msg.buffer, MAX_MSG_SIZE);
    
    size_t inBytes = filesystem->recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        perror("recv failed");
        return -1;
    }
    
    // cout << "     packet seq = " << pkt.seq_num << endl;
    // cout << "     msg seq = " << msg.seq_num << endl;
    // cout << "in read - data return : " << msg.buffer << endl;
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }

    // buf = (void*)msg.buffer;
    memcpy(buf, msg.buffer, inBytes);
    return (size_t)msg.result;
    
   // return 0;
}


// write to file
ssize_t RemoteFileSystem::File::write(void * buf, size_t count)
{
   //  cout << "  ------------  file.write  ------------- " << endl;
    // create packet first
    Packet pkt;
    pkt.auth_token = filesystem->auth_token;
    pkt.seq_num = filesystem->getSeqNum();
    pkt.op = WRITE;
    //cout << "file descriptor : " << pkt.filedescriptor << endl;
    pkt.filedescriptor = (long long)fd;
    bzero(pkt.buf, MAX_MSG_SIZE);
    memcpy(pkt.buf, buf, count);
    
    pkt.size = count;
    
    cout << "  ***** packet info **** " << endl;
    //cout << "auth_token: " << pkt.auth_token << endl;
    cout << " seq - num: " << pkt.seq_num << endl;;
    cout << " op : " << pkt.op << endl;
    //cout << " pathname : " << pkt.pathname << endl;
    
    cout << " size : " << pkt.size << endl;
    cout << " data to write : " << pkt.buf << endl;
    cout << " packet size:" << sizeof(Packet) << endl;
    
    
    // call rpc send() to send packet to server
    size_t outBytes = filesystem->send(&pkt, (size_t)(sizeof(Packet)));
    if (outBytes < 0) {
        perror("send failed\n");
        return -1;
    }
    cout << " in write, send data to server : " << outBytes << endl;
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = filesystem->recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        perror("recv failed");
        return -1;
    }
    
    // cout << " in write, recv data to server : " << inBytes << endl;

    // cout << "packet seq = " << pkt.seq_num << endl;
    // cout << "msg seq = " << msg.seq_num << endl;
    // cout << "in write - data return : " << msg.result << endl;
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }
    
    buf = (void*)msg.buffer;
    return (size_t)msg.result;
    
    return 0;
}


// lseek file current position
off_t RemoteFileSystem::File::lseek(off_t offset, int whence)
{
    //fseek(f_, offset, whence);
   //  cout << "  --------------  file.seek  -------------- " << endl;
    // create packet first
    Packet pkt;
    pkt.auth_token = filesystem->auth_token;
    pkt.seq_num = filesystem->getSeqNum();
    pkt.op = LSEEK;
    pkt.filedescriptor = (long long)fd;
    pkt.size = offset;
    bzero(pkt.buf, MAX_MSG_SIZE);
    pkt.buf[0] = whence;
    
    /*
    cout << "packet info : " << endl;
    cout << "auth_token: " << pkt.auth_token << endl;
    cout << " seq - num: " << pkt.seq_num << endl;;
    cout << " op : " << pkt.op << endl;
    cout << " file descriptor : " << pkt.filedescriptor << endl;
    cout << " size : " << pkt.size << endl;
    cout << " data for lseek : " << pkt.buf[0] << endl;
    cout << " packet size:" << sizeof(Packet) << endl;
    */
    
    // call rpc send() to send packet to server
    size_t outBytes = filesystem->send(&pkt, (size_t)(sizeof(Packet)));
    if (outBytes < 0) {
        perror("send failed\n");
        return -1;
    }
    // cout << " in lseek, send data to server : " << outBytes << endl;
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = filesystem->recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        perror("recv failed");
        return -1;
    }
    // cout << " in lseek, recv data to server : " << inBytes << endl;
    
    // cout << "packet seq = " << pkt.seq_num << endl;
    // cout << "msg seq = " << msg.seq_num << endl;
    // cout << "in read - data return : " << msg.result << endl;
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }
    
    return (off_t)msg.result;
}


// file construct with rpc
RemoteFileSystem::File::File(RemoteFileSystem* filesystem, const char *pathname, char *mode){
    this->filesystem = filesystem;
    // cout << "in file constructor, print this: " << this << endl;
    // cout << " in file constructor , print filesystem" << filesystem << endl;
    // cout << "in file constructor, print fd : " << fd << endl;
    // this->filesystem->open(pathname, mode);
}


// rpc constructor
RemoteFileSystem::RemoteFileSystem(char * host, short port, unsigned long auth_token, int timeout)
{
   //  cout << " ------------- rpc constructor --------------- " << endl;
    this->auth_token = auth_token;
    this->seq_num = 0;

    struct hostent *hp;
    
    // open socket
    // AF_INET: IPV4; AF_INET6: IPV6;
    // SOCK_STRAM: BYTE STREAM, SOCK_DGRAM: DATAGRAM
    sockfd = socket(AF_INET, SOCK_DGRAM , 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket error\n");
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;
    hp = gethostbyname(host);
    if (hp == 0) {
        perror("host\n");
    }
    cout << "socket success " << endl;
    bcopy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
    server.sin_port = htons(port);
    length = sizeof(from);
    
    
    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        // The client libaray may retry once after a timeout
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
            fprintf(stderr, "error setting socket timeout\n");
            exit(EXIT_FAILURE);
        }
    }
    
/*
    cout << " here is test connect example " << endl;
    char buf[] = "Hello, I am client!";
    
    int n = send(buf, 128);
    if (n < 0 ) {
        cout << "send error for testing" << endl;
    }
    
    n = recv(buf, 128);
    
    if (n < 0 ) {
        cout << "recv error for testing" << endl;
    }
    
    cout << "recv data: " << buf << endl;
 */
    
    /*
     // init
     memset(&this->sin, 0, sizeof(sin));
     this->sin.sin_family = AF_INET;
     this->sin.sin_addr = server_ip;
     this->sin.sin_port = htons(port);
     this->slen = sizeof(this->sin);
     printf("connect to 0x%08x, %s\n", sin.sin_addr.s_addr, inet_ntoa(sin.sin_addr));
     
     // open file
     
     */
    
   // cout << "rpc construct success" << endl;
    
}

RemoteFileSystem::~RemoteFileSystem()
{
    // disconnect
}


// Only remoteFileSystem can open a file
RemoteFileSystem::File * RemoteFileSystem::open(const char * pathname, char * mode)
{
   // cout << " --------  start to open file ---------- " << endl;
    Packet pkt;
    
    pkt.auth_token = 1234;
    pkt.seq_num = getSeqNum();;
    pkt.op = OPEN;
    memcpy(pkt.pathname, pathname, 32);
    //pkt.pathname = strdup(pathname);
    
    switch(*mode) {
        case 'r' :
        {
            if (mode[1] == '\0') {
                pkt.size = O_RDONLY;
            }else if (mode[1] == '+' && mode[2] == '\0'){
                pkt.size = O_RDWR;
            } else {
                pkt.size = -1;
            }
        }
            break;
        case 'w' :
        {
            if (mode[1] == '\0') {
                pkt.size = O_WRONLY | O_CREAT | O_TRUNC;
            }else if (mode[1] == '+' && mode[2] == '\0') {
                pkt.size = O_RDWR | O_CREAT;
            }else {
                pkt.size = -1;
            }
        }
            break;
        default:
        {
            pkt.size = -1;
        }
    }// end switch
    
   // pkt.value = 0;
    /*
    cout << " ****** packet info ******* " << endl;
    cout << "auth_token: " << pkt.auth_token << endl;
    cout << " seq - num: " << pkt.seq_num << endl;;
    cout << " op : " << pkt.op << endl;
    cout << " pathname : " << pkt.pathname << endl;
    cout << " size : " << pkt.size << endl;
    cout << " packet size:" << sizeof(Packet) << endl;
    */
    
    // call rpc send() to send packet to server
    size_t outBytes = send(&pkt, sizeof(Packet));
    if (outBytes < 0) {
        perror("send failed\n");
        return nullptr;
    }
    
    // cout << "in Open, sent success : " << outBytes << endl;
    
    // call recv() to receive message from server
    Message msg;
    msg.seq_num = 0;
    msg.result = 0;
    size_t inBytes = recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        perror("recv failed");
        return nullptr;
    }
    /*
    cout << " recv bytes: " << inBytes << endl;
    cout << " ------ msg info -----" << endl;
    cout << "msg size : " << sizeof(msg) << endl;
    cout << "msg - seq : " << msg.seq_num << endl;
    cout << "msg - result: " << msg.result << endl;
    */
    
    /*
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return nullptr;
    }
*/
   // cout << "in Open , recv success" <<  endl;
    
    //File *file = (File*)msg.result;
    // cout << " in open -- print this filesystem : " << this << endl;
    File* file;
    file = new File(this, pathname, mode);
    file->fd = (File*)msg.result;
    // cout << "in open - file.fd = " << file->fd << endl;
    // cout << "in open - file = " << file << endl << endl;
   // file->filesystem = *&this;
   //  cout << "before return in open" << endl;
    return file;
}


int RemoteFileSystem::chmod(const char * pathname, mode_t mode)
{
    // File file(pathname, mode);
    Packet pkt;
    //pkt.auth_token = reinterpret_cast<uint64_t>(authTokenList[0]);
    pkt.seq_num = getSeqNum();
    pkt.op = CHMOD;
    memcpy(pkt.pathname, pathname, 32);
    // pkt.pathname = strdup(pathname);
    pkt.size = 0;
    //pkt.buf = nullptr;
    
    //int f = file.f();
    
    // call rpc send() to send packet to server
    size_t outBytes = send(&pkt, sizeof(pkt));
    if (outBytes < 0) {
        // perror("send failed\n");
        return -1;
    }
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        // perrof("recv failed");
        return -1;
    }
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }

    // validate
    
    return msg.result;

}

int RemoteFileSystem::unlink(const char * pathname)
{
    // File file(pathname, mode);
    Packet pkt;
    //pkt.auth_token = reinterpret_cast<uint64_t>(authTokenList[0]);
    pkt.seq_num = getSeqNum();
    pkt.op = UNLINK;
    bzero(pkt.buf, 32);
    memcpy(pkt.pathname, pathname, 32);
    //pkt.pathname = strdup(pathname);
    pkt.size = 0;
    
    //int f = file.f();
    
    // call rpc send() to send packet to server
    size_t outBytes = send(&pkt, sizeof(Packet));
    if (outBytes < 0) {
        // perror("send failed\n");
        return -1;
    }
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        // perrof("recv failed");
        return -1;
    }
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }
    
    // validate
    
    return msg.result;
}

int RemoteFileSystem::rename(const char * oldpath, const char * newpath)
{
    // File file(pathname, mode);
    Packet pkt;
    //pkt.auth_token = reinterpret_cast<uint64_t>(authTokenList[0]);
    pkt.seq_num = getSeqNum();
    pkt.op = RENAME;
    memcpy(pkt.pathname, oldpath, 32);
    bzero(pkt.buf, MAX_MSG_SIZE);
    memcpy(pkt.buf, newpath, 32);
    //pkt.pathname = strdup(pathname);
   // pkt.size = 0;
    
    //int f = file.f();
    
    // call rpc send() to send packet to server
    size_t outBytes = send(&pkt, sizeof(Packet));
    if (outBytes < 0) {
        // perror("send failed\n");
        return -1;
    }
    
    // call recv() to receive message from server
    Message msg;
    size_t inBytes = recv(&msg, sizeof(Message));
    if ( inBytes < 0) {
        // perrof("recv failed");
        return -1;
    }
    
    // valid sequence number
    if (msg.seq_num != pkt.seq_num) {
        fprintf(stderr, "received packet sequence number is not send packet sequence number");
        return -1;
    }
      
    return msg.result;
    
}


ssize_t RemoteFileSystem::send(const void* buffer, size_t size){
    
    // cout << " start to send " << endl;
    // cout << " send size : " << size << endl;
    ssize_t outBytes;
    outBytes = sendto(sockfd, buffer, size, 0, (struct sockaddr*)&server, length);
    
    /*
    if (outBytes < 0) {
        fprintf(stderr, "sending failed\n");
        return -1;
    }
    */
    
    // cout << "in send, outBytes: " << outBytes << endl;
    return outBytes;
    
}//


// setsocketpot();
ssize_t RemoteFileSystem::recv(void *buffer, size_t size) {
    // cout << " start to recv " << endl;

    ssize_t inBytes = 0;
    inBytes = recvfrom(sockfd, buffer, size, 0, (struct sockaddr*)&from, (socklen_t*)&length);
    
    /*
    if (inBytes < 0) {
        fprintf(stderr, "recvfrom failed\n");
        return -1;
    }
    */
    // cout << " in recv, inBytes = " << inBytes << endl;
    return inBytes;
}


// RemoteFileSystem getPktId return id of packet
int RemoteFileSystem::getSeqNum() {
    return this->seq_num++;
}
