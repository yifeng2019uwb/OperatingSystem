//
//  main.cpp
//  Assignment3
//
//  Created by Yifeng Zhang on 5/19/19.
//  Copyright © 2019 Yifeng Zhang. All rights reserved.
//

#include <iostream>
#include "file.h"
#include <unistd.h>
#include <sys/uio.h>
//#include <string>
#include <stdio.h>
using namespace std;


const char *PROG_NAME = "None";

void usage();


int main(int argc, const char *argv[]) {
    // insert code here...
    // Since constructors and destructors don't return values,
    // we have to alter the semantics slightly: if an error occurs, throw an exception.
    
  char c;
    char *s = new char[1024];
    const char *str = "Hello!";
    
    const char *s1 = " CSS503: Course Description: This course examines the logical design and programming aspects of operating systems and network communication.";
    
    char *s2 = new char[16384];
    char *s3 = new char[1024];

    
    // Your test program should verify that writing to a read-only file is an error;
    
    cout << " ------------- test read-only file ----------------- " << endl;
   
    try {      
      File rfile("file0.txt","r");
      cout << " ** test fgetc for 3 time ** " << endl;
      for (int i = 0; i < 3; i++) {
        c = rfile.fgetc();
	if (c > 0 ) {
	  cout << "test fgetc:   Success! "  << endl;
	  cout << "fgetc : " << c << endl;
	}
      }
      
      cout << endl;
    
      cout << " ** test fgets -  8 bits for 3 times ** " << endl;
      for (int i = 0; i < 3; i++) {
	 //if ( rfile.fgets(s, 8) != nullptr ) {
	 // cout << "test fgets :      Success!" << endl;
	  cout << "fgets string: " << endl << s << endl;
	//}
      }
      cout << endl;
    
      int n = 0;
      cout << " ** test fread with 1024 numbers with each size i  ** " << endl;
      cout << " check how many bits to read every time and compare with asked read size " << endl;
      for (int i = 1; i < 17 ; i++) {
        n = rfile.fread(s2, i , 1024);
        cout << " ******* The " << i << " time fread and read " << n << " bits ********" << endl;
        cout << s2 << endl;
        cout << " ********************************************* " << endl;
        cout << endl << endl<< endl;
      }
      cout << " ********************************************* " << endl;
      cout << endl << endl;
    
    
      cout << "  ----- test write to read only file -----" << endl;
      if ( rfile.fwrite(s1, 2, 8) < 0) {
          cout << "test write to read only file:     Success!" << endl;
      }
      if (rfile.ferror() < 0) {
          cout << "test ferror:     Success!" << endl;

      }
       
    cout  << endl << endl;
    } catch (File::File_Exception &e) {
      cerr << "Error! " << endl;
    }
    

    // read/write from a write-only file is an error;
    cout << " -------------- test rw file --------------- " << endl;
    try {
      
      File rwfile("file1.txt","r+");
    
      cout << " ** test fgetc for 3 times ** " << endl;
      for (int i = 0; i < 3; i++) {
	if ( ( c = rwfile.fgetc()) > 0) {
	  cout << "test fgetc:     Success! " << endl;
	  cout << static_cast<char>(c) << endl;
	}
      }
    
      cout << " ** test fputc for 3 times ** " << endl;
      if ( rwfile.fputc('X') > 0 ) {
          cout << "test fputc:    Success! " << endl;
      }
      if ( rwfile.fputc('Y') > 0 ) {
          cout << "test fputc:    Success! " << endl;
      }
      if ( rwfile.fputc('Z') > 0 ) {
          cout << "test fputc:    Success! " << endl;
      }

    
    cout << " ** test fgets with getting 16 bits ** " << endl;
     //  if (rwfile.fgets(s, 16) > 0 ) {
	// cout << "test fgets :    Success! " << endl;
    cout << "fgets : " << endl << s << endl;
    //  }
    
      cout << " ** test fputs : string should store at the position (3+3+16 = 21) ** " << endl;
      if ( rwfile.fputs(str) > 0 ) {
          cout << " test fputs :    Success! " << endl;
      }

      cout << " ** test fread: 32 numbers' data with each size 8 bit **" << endl;
      if ( rwfile.fread(s3, 8, 32) > 0 ) {
          cout << "test fread :     Success! " << endl;
          cout << "fread : " << endl << s3 << endl;
      }
    
      if ( rwfile.fwrite(s1, 2, 32) > 0 ) {
          cout << "test fwrite :     Success! " << endl;
      }
    
      cout  << endl << endl;

    } catch (File::File_Exception &e) {
      cerr << "Error! " << endl;
    }
   
    
  // write from a write-only file is an error;
    cout << " -------------- test write-only file  -------------- " << endl;

    try {
      File wfile("file2.txt","w");
    
      cout << " ** test fputc for 3 times ** " << endl;
      if (  wfile.fputc('X') > 0 ) {
          cout << "test fputc :    Success! " << endl;
      }
       if (  wfile.fputc('Y') > 0 ) {
           cout << "test fputc :    Success! " << endl;
      }
       if (  wfile.fputc('Z') >  0 )   {
           cout << "test fputc :    Success! " << endl;
      }
    
      cout << " ** test fputs **" << endl;
      if ( wfile.fputs(str) > 0 ) {
          cout << "test fputs :    Success! " << endl;
      }
    
      cout << " ** test fwrite **" << endl;
      if ( wfile.fwrite(s1, 8, 16) > 0 ) {
          cout << "test fwrite:    Success!" << endl;
      }
    
      cout << "  ----------  test read to write only file -----------" << endl;
      if ( wfile.fread(s3, 2, 128) < 0) {
          cout << "test read to write only file :    Success! " << endl;
      }
      
      if ( wfile.ferror() < 0 ) {
          cout << "test ferror :    Success! " << endl;
      }
      
      cout  << endl;
      } catch (File::File_Exception &e) {
          cerr << "Error! " << endl;
      }
   
    /*
    delete[] s;
    delete[] str;
    delete[] s1;
    delete[] s2;
    delete[] s3;
    */
     // exception test
    cout << " ------- test contructor exception error ---------" << endl;
    
    try {
      File e_file("filfcch.txt", "rr");
      
    } catch (File::File_Exception &e) {
      cerr << "Error " << endl;
    }
   
    
    return 0;
    
}
