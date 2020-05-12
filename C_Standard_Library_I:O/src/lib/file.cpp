//
// file.cc
//
// Implement the C standard I/O library in C++ style.
//
// Author: Yifeng Zhang based on Prof. Morris Bernstein's skelton file
// Copyright 2019, Systems Deployment, LLC.


#include "file.h"

#include <iostream>
#include <cassert>
#include <cstdarg>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

using namespace std;
/*
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

*/


// Open a file.
// Mode can be "r", "r+", "w", "w+", "a", and "a+"
// The initial file position for reading is at the beginning of the file,
// but output is always appended to the end of the file
// Use default buffering: FULL_BUFFER.

File::File(const char *name, const char *mode) {
 
    switch(*mode) {
        case 'r' :
        {
            if (mode[1] == '\0') {
                // 'r' - O_RDONLY
                flag = O_RDONLY;
            }else if (mode[1] == '+' && mode[2] == '\0'){
                // 'r+' - O_RDWR
                flag = O_RDWR;
            } else {
                // error
                flag = -1;
            }
        }
            break;
        case 'w' :
        {
            if (mode[1] == '\0') {
                // 'w' - O_WRONLY | O_CREAT | O_TRUNC
	      flag = O_WRONLY | O_CREAT | O_TRUNC;    //
            }else if (mode[1] == '+' && mode[2] == '\0') {
                // 'w+' - O_RDWR | O_CREAT | O_TRUNC
                flag = O_RDWR | O_CREAT;
            }else {
                flag = -1;
            }
        }
            break;
        default:
        {
            // else error
            flag = -1;
        }
    }// end switch
    
    // throw error--- not use try and catch??
 
        if (flag == -1)
        {
            throw my_ex;
        }
        
        b_mode = FULL_BUFFER;
        //mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |S_IWOTH;
        mode_t f_mode = S_IRWXU; // read, write, execute/search by owner
        // fd = open(name, flag, buffermode);
        fd = open(name, flag, f_mode);

        if (fd < 0) {
            throw  my_ex;
        }
    
    
    // init
    // allocate the memory for buffer
    buf = (unsigned char*)(malloc(bufsiz * sizeof(unsigned char)));
    
    b_real_size = 0;
    f_pos = 0;
    b_pos = 0;
    if (flag == O_WRONLY ) { //| O_CREAT | O_TRUNC)) {
      iseof = true;
    }else {
      iseof = false;
    }
    errorNum = NO_ERROR;

}


// Close the file.  Make sure any buffered data is written to disk,
// and free the buffer if there is one.
File::~File() {
    fflush();
    free(buf);
    close(fd);
}


// Return non-zero value if the file is in an error state.
// When the file is in an error state, I/O operations are not
// performed and appropriate values are returned.
int File::ferror() {
    if ( errorNum != NO_ERROR) {
        //fprintf("error");
        return -1;  // non-zero
    }

    return 0;            // placeholder
}

// Return true if end-of-file is reached.  This is not an error
// condition.  Reading past eof is an error.
bool File::feof() {
  if (iseof && (b_pos == b_real_size)) {
      return true;
    }else{
      return false;
    }

  // If I call lseek every time, it's expensive
  /*
    // compare the cur_pos and end position/file length
  cout << "lssek 3 times" << endl << endl << endl;
    off_t cur_pos = lseek(fd, 0, SEEK_CUR);
    off_t end_pos = lseek(fd, 0, SEEK_END);
    lseek(fd, cur_pos, SEEK_SET);   // set cur position back
    
    // cout << "feof : cur = " << cur_pos << "end = " << end_pos << "real current = " << lseek(fd, 0, SEEK_CUR) << endl;
    
    if (b_real_size == 0) {
        if (cur_pos == end_pos) {
            return true;
        }
    }else {
        if ( (b_pos == b_real_size) && (f_pos + b_pos) >= end_pos ){
            return true;
        }
    }
    
    return false;            // placeholder
  */
}


// Add a user-defined buffer and set the buffering mode.  If
// non-null, the buffer must have been created by malloc and will be
// freed by the destructor (or by another call to setvbuf).
// LINE_BUFFER mode need not be supported.
int File::setvbuf(char *buf, BufferMode mode, size_t size) {
    assert(0);            // Not implemented.
}


// If data is buffered for writing, write the buffered data to
// disk.  Reset the buffer to empty. Reset the file pointer so it
// behaves the way the user would expect.
int File::fflush() {
    // if buffer status == WRITE, write the buffered data to disk
  
   if (flag != O_RDONLY && b_rw == WRITE) {
     lseek(fd, f_pos, SEEK_SET);
     size_t numWrite = write(fd, buf, b_real_size);
     if (numWrite < 0) {
       errorNum = FILE_WRITE;
       return -1;  //
     }
   }
    
   // reset file pointer
   // f_pos = lseek(fd, 0, SEEK_CUR);
   // reset buffer position = 0, buffer real used size = 0; buffer default status is READ
   b_pos = 0;
   b_real_size = 0;
   b_rw = READ;    //
    
   return 0;             // placeholder
}


// If the amount of data to be read or written exceeds the buffer,
// avoid double-buffering by reading/writing data directly to/from
// the source/destination.
// nmemb = number items of data with each size bytes long
size_t File::fread(void *ptr, size_t size, size_t nmemb) {
       
    // if write only or error return -1
  if( flag == (O_WRONLY | O_CREAT | O_TRUNC)) {
        // cout << " read file error: write only mode" << endl;
        errorNum = FILE_MODE;
        return -1;

    }
   
    unsigned char * p   = (unsigned char*)ptr;
    
    size_t read_size = size * nmemb;// chunk_size, totl byte to read
    if (read_size <= 0) return -1;
    size_t numRead = 0;
  
    // 2 cases for pfread, from buffer or source file
    // case 1 - read from buffer if read_size < bufsize, may read buffer again
    // case 2 - read_size > bufsiz, read from buffer first, and then read from source
    // both case start to read buffer first, if read read_size bit, return
	
    while ( (numRead < read_size) &&  (b_real_size > b_pos) ) {
      p[numRead++] = buf[b_pos++];
    }
    
    // compare if read read_size or if feof == > return
    if (numRead == read_size ) {
        return numRead;
    }else {
      // if feof
      if (b_pos != 0 && b_pos != bufsiz && b_pos == b_real_size) {
	return numRead;
      }
    }
    
    // before read from source or refill buffer, fflush buffer
    if (b_rw == WRITE) {
        fflush();
    }
 
    if (read_size < static_cast<size_t>(bufsiz) ) {
        // case 1 -- fill buffer again and read from buffer
        // numReadm < read_size
      
        f_pos = lseek(fd, 0, SEEK_CUR); // update f_pos for write back     
        b_real_size = read(fd, buf, bufsiz);
       if (b_real_size < 0) {
            return -1;
        }
        b_pos = 0;
	
	if (b_real_size < bufsiz ) {
	  iseof = true;
	}
        
        // read from buffer again       
        while ( (numRead < read_size) &&  (b_real_size > b_pos) ) {
            p[numRead++] = buf[b_pos++];
        }


    }else {
        // case 2, read from source
        // read directly from source file

        read_size -= numRead;
        f_pos = lseek(fd, 0, SEEK_CUR); // track file position for write back
        size_t n = read(fd, p, read_size);
        // cout << "fread 1 time for larger chunk , read n = " << n << " + numRead = " << numRead <<  endl;
        if (n < 0 ) {
            errorNum = FILE_READ;
            return -1;
        }

      	if (n < read_size ) {
	  iseof = true;
	}
        
        numRead += n;
    }
 
    return numRead;             // placeholder
}


size_t File::fwrite(const void *ptr, size_t size, size_t nmemb) {
    // if read only or error return -1
    if( flag == O_RDONLY) {
        errorNum = FILE_MODE;
        return -1;
    }
    
    size_t write_size = size * nmemb;// chunk_size, totl byte to write
    if (write_size <= 0) {
        return -1;
    }
    
    unsigned char* p = (unsigned char*)(ptr);
    size_t numWrite = 0;
    // size_t i = 0;
    // 2 cases for fwrite, write to buffer or to file
    // case 1 - write directly to source
    // case 2 - write to buffer, fflush() then write to buffer or source file
    
    // write to buffer first
    while ( (numWrite < write_size) && ( b_pos < static_cast<off_t>(bufsiz)) ) {
        buf[b_pos++] = static_cast<unsigned char>(p[numWrite++]);
    }

    // cout << "numWrite = " << numWrite << endl; 
    // numWrite < write_size, b_pos == bufsize, buffer is full
    if (numWrite > 0) {
       b_rw = WRITE; 
    }
 
    if (b_pos >= b_real_size) {
      b_real_size = b_pos + 1;
    }
    
    // if buffer has enough space for write in
    if (numWrite == write_size) {
      return numWrite;
    }
   
    // buffer is full, fflush
    fflush();
    
    // need more write again to buffer or to source file directly
    if (write_size >= bufsiz) {
      // case1 : write to source directly
      size_t n = write(fd, p, (write_size - numWrite));
        
      if (n < 0) {
	return -1;
      }
      numWrite += n;
	
    }else {
      // write to buffer again if need
      // if not write only, need fill buffer first
      if (flag != (O_WRONLY | O_CREAT | O_TRUNC)) {
	  f_pos = lseek(fd, 0, SEEK_CUR);
	  b_real_size  = read(fd, buf, bufsiz);
	  if (b_real_size  < 0) {
	    errorNum = FILE_READ;
	    return -1;
	  }
	  b_pos = 0;
      }
	
       while ( (numWrite < write_size) && ( b_pos < static_cast<off_t>(bufsiz))) {
	 buf[b_pos++] = static_cast<unsigned char>(p[numWrite++]);
       }

       if (b_real_size < b_pos) {
	  b_real_size = b_pos + 1;
       }
	
      if (numWrite > 0) {
        b_rw = WRITE;
      }   
	
    }
      
    return numWrite;

    // return 0;             // placeholder
}


int File::fgetc() {

  if (flag == (O_WRONLY | O_CREAT | O_TRUNC)) {
        // cout << " read file error: write only mode" << endl;
        errorNum = FILE_MODE;
        return -1;
    }
    
    unsigned char* char_array = (unsigned char*)(malloc(sizeof(unsigned char)));
    fread(char_array, 1, 1);
    
    unsigned char c = char_array[0];
    free(char_array);
    return (int)(c);
    // if eof, return 0
    //return eof;             // placeholder
}


int File::fputc(int c) {
    if (flag == O_RDONLY) {
        // cout << " write file error: read only mode" << endl;
        errorNum = FILE_MODE;
        return eof;
    }
    
    unsigned char* char_array = (unsigned char*)(malloc (sizeof(unsigned char)));
    char_array[0] = static_cast<unsigned char>(c);
    if ((fwrite(char_array, 1, 1)) > 0 ) {
      // b_rw = WRITE;//it already update in fwrite
        free(char_array);
        return c;
    }

    free(char_array);
    return -1;      // error
}


char *File::fgets(char *s, int size) {
  if (flag == (O_WRONLY | O_CREAT | O_TRUNC)) {
        //cout << " read file error: write only mode" << endl;
        errorNum = FILE_MODE;
        return s;
    }
    
    if (size < 1) {
        // nothing read
        return s;
    }
    // size--; // fgets function reads at most one less than the number of characters specified by size
    
    unsigned char* str = (unsigned char*)(s);
    fread(str, 1, static_cast<size_t>(size));

    return s;
    
}


int File::fputs(const char *str) {
    if (flag == O_RDONLY) {
        // cout << " write file error: read only mode" << endl;
        errorNum = FILE_MODE;
        return eof;
    }
    size_t strlen = 0;
    
    while (str[strlen] != '\0') {
        strlen++;
    }
    
    if (strlen == 0) return -1;
    unsigned char* p = (unsigned char*)(str);
    size_t numWrite = fwrite(p, 1, strlen);
    
    if (numWrite > 0) {
      //  b_rw = WRITE;//update in fwrite
        return static_cast<int>(numWrite);
    }
   
    return -1;      // error
    
}


int File::fseek(long offset, Whence whence) {
    fflush();
    off_t cur;
    if (whence == seek_set) {
        cur = lseek(fd, offset, SEEK_SET); // position = file_start + offset
        
    } else if (whence == seek_cur) {
        cur = lseek(fd, offset, SEEK_CUR);  // position = cur + offset
        
    }else if (whence == seek_end) {
        cur = lseek(fd, offset, SEEK_END);  // position = end + offset
        
    } else {
        ferror();
        cur = eof;
    }
    
    f_pos = cur;
    return static_cast<int>(cur);
    
}


bool File::clearerr(){
    errorNum = NO_ERROR;
    return true;
}


static const int ITOA_BUFSIZE = 32; // log10(2**64 < 21) + sign character

static char *itoa(int i, char a[ITOA_BUFSIZE]) {
    char * p = a + (ITOA_BUFSIZE - 1);
    *p = '\0';            // Trailing NUL byte: end-of-string.
    if (i == 0) {
        *--p = '0';
        return p;
    }
    bool negative = (i < 0);
    if (negative) {
        i = -i;
    }
    for (; i > 0; i = i / 10) {
        *--p = '0' + (i % 10);
    }
    if (negative) {
        *--p = '-';
    }
    return p;
}


// Stripped-down version: only implements %d, %s, and %% format codes.
int File::fprintf(const char *format, ...) {
    int n = 0;            // Number of characters printed.
    va_list arg_list;
    va_start(arg_list, format);
    for (const char *p = format; *p != '\0'; p++) {
        if (*p != '%') {
            if (fputc(*p) < 0) {
                return -1;
            }
            n++;
            continue;
        }
        switch(*++p) {
            case 's':
            {
                char *s = va_arg(arg_list, char *);
                for (;*s != '\0'; s++) {
                    if (fputc(*s) < 0) {
                        return -1;
                    }
                    n++;
                }
            }
                break;
            case 'd':
            {
                int i = va_arg(arg_list, int);
                char sbuf[ITOA_BUFSIZE];
                char *s = itoa(i, sbuf);
                for (;*s != '\0'; s++) {
                    if (fputc(*s) < 0) {
                        return -1;
                    }
                    n++;
                }
            }
                break;
            default:
                if (fputc(*p) < 0) {
                    return -1;
                }
                n++;
        }
    }
    va_end(arg_list);
    return n;
}
