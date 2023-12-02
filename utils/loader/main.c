
#define _GNU_SOURCE

#include <stdint.h>  // utin8_t
#include <stdio.h> // asprintf, normal printf, fopen
#include <stdlib.h> // strtoul
#include <string.h>
#include "returnValues.h"
#include "ssls3.h"

/*
xxxxx initialize console at 1Mbaud with no echo by doing
xxxxx BR 3

This requires MUCH MUCH longer reply time than the dumper.
  I should have just used a state machine...

*/

#define SERIALPORT  "/dev/ttyUSB1"

#define TIMEOUT 2000


void chomp(char * string);
int value(char *s);

int main(int argc, char** argv) {

 SSLS_t port;
 char * s;
 int d, r;
 FILE * input;
 
 if (argc < 2) {                          // do we have input paramiters??   
      puts("Need filename to load\n");
      return 0;    
  }
  
  if ((input = fopen(argv[1], "rb")) == NULL) {  //open text file 'param 1' w/ err chk 
      printf("Unable to open %s for input.\n", argv[1]);
      return -1;
  }

 SerInit (&port);
 SerOpen (&port, SERIALPORT, 38400); // yea this really should check for errors...

 s = strdup("?");
 waitConverse(&port, &s, TIMEOUT*3, TIMEOUT );
 *s = 0;
 free(s);

 s = strdup("AP 00000080\r");
 waitConverse(&port, &s, TIMEOUT*3, TIMEOUT );
 if (s) printf("set address 0 -> %s\n", s);

 
 while ( (d = fgetc(input)) != EOF) {
   free(s);  s = NULL; asprintf(&s, "WT %02X\r", d);
   waitConverse(&port, &s, TIMEOUT*3, TIMEOUT );

   if (s) {
     //printf(": %s", s);
     if ((r = value(s)) != d) {
       printf("\nError, mismatch! }:( %02X != %02X\n", r, d);
       break; // hopefully this doesn't happen... ever casue otherwise I'll have write more code...
     }
   } else {
       printf("\nError, data didn't ack! (:/ \n");
       break; // hopefully this doesn't happen... ever casue otherwise I'll have write more code...  
   }
 } 

 if (d == EOF) { 
   printf("SUCCESS!!!!\n"); // ¯\_('')_/¯
 }

 fclose(input);
 SerFini (&port);

  return 0;
}


void chomp(char * string) {
  int target;
  
  target = strcspn(string,"\r\n");
  string[target] = '\0';
  
}


int value(char *s){

  unsigned long a;
  uint8_t d;

   
 if (strlen(s) < 28) { printf("Arg, partial reply!?? >%s<\n", s); return -1; }

    chomp(s+8); 
   // printf("-VALUE-)%s", s+8);    
     
     if (strstr(s+8, "Addr") == (s+8)) {
       //printf("OK, ");
       a = strtoul(s+13, NULL, 16); //  printf( " a=0x%08X", a);
       d = strtoul(s+26, NULL, 16); //  printf( " d=0x%02X\n", d);

      // if ((a & 0xFF) == 0) { printf("0x%08X\r", a); fflush(stdout); }  
           
       printf("0x%08X\r", a); fflush(stdout);
       
       return d;
       
     }
  return -1;
}

