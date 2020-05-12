//
//  benchmarkingtest.cpp base on lab3.cc
//
// Author: Yifeng base on Prof. Bernstein's sketlton

#include "file.h"
#include <fcntl.h>        // open
#include <unistd.h>        // read
#include <sys/types.h>    // read
#include <sys/uio.h>    // read
#include <stdio.h>        // fopen, fread
#include <sys/time.h>    // gettimeofday
#include <iostream>
#include <string>

using namespace std;


const char *PROG_NAME = "none";

void usage();


class Timer {
public:
    Timer(const string& message): message(message) {
        gettimeofday(&start, nullptr);
    }
    
    ~Timer() {
        struct timeval end;
        gettimeofday(&end, nullptr);
        double elapsed =
        ((double) end.tv_sec + (double) end.tv_usec / 1e6) -
        ((double) start.tv_sec + (double) start.tv_usec / 1e6);
        printf("%s elapsed time:\n\t%12.9f seconds\n", message.c_str(), elapsed);
    }
    
private:
    string message;
    struct timeval start;
};


class Unix_Reader {
public:
    Unix_Reader(char *filename) {
        // TODO(lab): open the file using the Unix system call; exit
        // program with failure if file cannot be opened.
        fd_ = open(filename, O_RDONLY);
        if (fd_ < 0) {
            perror("open file failed\n");
            exit(EXIT_FAILURE);
        }
    }
    
    ~Unix_Reader() {
        // TODO(lab): close the file.
        close(fd_);
    }
    
    int fd() {return fd_;}
    
private:
    int fd_;
};


class Stdio_Reader {
public:
    Stdio_Reader(char *filename) {
        // TODO(lab): open the file using the C standard I/O; exit
        // program with failure if file cannot be opened.
        f_ = fopen(filename, "r");
        if (!f_) {
            perror("open file failed\n");
            exit(EXIT_FAILURE);
        }
    }
    
    ~Stdio_Reader() {
        // TODO(lab): close the file.
        fclose(f_);
    }
    
    FILE *f() {return f_;}
    
private:
    FILE *f_;
};


void usage() {
    fprintf(stderr,  "usage: %s filename chunk_size\n", PROG_NAME);
    fprintf(stderr, "   where chunk_size > 0\n");
    exit(EXIT_FAILURE);
}


int main( int argc, char *argv[] ) {
    PROG_NAME = argv[0];
    if ( argc != 3 ) {
        usage();
    }
    
    int chunk_size = atoi( argv[2] );
    if ( chunk_size < 1 ) {
        usage();
    }
    
    char *filename = argv[1];
    char *buf = new char[chunk_size];
    
    int total_bytes_read = 0;
    {
        // Unix I/O
        Unix_Reader r(filename);
        int fd = r.fd();
        Timer t(string("Unix system calls, chunk size ") + to_string(chunk_size));
        
        // TODO(lab): read the file in chunks of chunk_size using Unix
        // File I/O.  Count the number of bytes read.  Exit the loop on
        // EOF.  Print error message and terminate the program if there is
        // an error reading the file.
        
        ssize_t nread;
        do{
            nread = read(fd, buf, chunk_size);
            if(nread == static_cast<ssize_t>(-1)){
                perror("read file error");
                break;
            }
            total_bytes_read += static_cast<int>(nread);
        } while(nread);
    }
    
    printf("\tbytes read: %d\n", total_bytes_read);
    
    total_bytes_read = 0;
    
    {
        // C standard library  I/O.
        Stdio_Reader r(filename);
        FILE *file = r.f();
        
        // TODO(lab): read the file in chunks of chunk_size using C
        // standard I/O.  If chunk_size is 1, use fgetc. Count the number
        // of bytes read.  Exit the loop on EOF.  Print error message and
        // terminate the program if there is an error reading the file.
        
        
        if (chunk_size == 1) {
            Timer t(string("C standard library I/O fgetc elapsed time"));
            int c;
            while (!feof(file)) {
                c = fgetc(file);
                if (c < 0) {
                    perror("read file error");
                    break;
                }
                total_bytes_read++;
            }
            
        }else {
            Timer t(string("C standard library I/O, block size ") + to_string(chunk_size));
            size_t n;
            while (!feof(file)){
                // Timer t(string("C standard library I/O, time for read 1 block size ") );
                n = fread(buf, 1, chunk_size, file);
                if (n == static_cast<size_t>(-1)) {
                    perror("read file error");
                    break;
                }
                total_bytes_read += static_cast<int>(n);
            }
        }
    }
    
    printf("\tbytes read: %d\n", total_bytes_read);
    
    // BenchMarking test for my file library - file.h
    total_bytes_read = 0;
    File r(filename, "r");

     if (chunk_size == 1) {
        Timer t(string("My File I/O fgetc "));
         
        while (!r.feof()) {
            r.fgetc();
            total_bytes_read++;
            // cout << total_bytes_read << endl;
        }
         
     } else {
         ssize_t nread;
         Timer t(string("My File I/O calls, chunk size ") + to_string(chunk_size));
         
         int n = 0;
         while (!r.feof())
         {
            // Timer t1(string("My File I/O calls, time for read each chunk size ") );
             
             nread = r.fread(buf, 1, chunk_size);
            // cout << "nread : " << nread << endl;
             if(static_cast<int>(nread) < 0){
                cout << "read file error" << endl;
                 break;
             }
             // cout << "nread = " << nread << endl;
             total_bytes_read += static_cast<int>(nread);
             n++;
         }
     }
    
    cout << "\tbytes read: " << total_bytes_read << endl;
    r.~File();
    
    delete[] buf;
//    delete[] filename;
    
    exit(EXIT_SUCCESS);
    
}

