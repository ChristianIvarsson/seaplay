#include <stdint.h>  // utin8_t
#include <stdio.h> // printf, fopen
#include <stdlib.h> // strtoul
#include <string.h>
#include "returnValues.h"
#include "ssls3.h"

/*
xxx  initialize console at 1Mbaud with no echo by doing  BR 3

  or lets stay with 38400...
*/


// the lower with this the better but it will slow things down.


#define SERIALPORT  "/dev/ttyUSB1"

#define TIMEOUT 2500

void chomp(char * string);
void values(char *s);

uint8_t buff[65538];

int main(void) {

 SSLS_t port;
 char * s;
 long int i;
 FILE * output;
 
 output = fopen("readback.bin", "wb");
 
 SerInit (&port);
 //SerOpen (&port, SERIALPORT, 1000000); // BR 3 with 3Mbaud base, BR 6 with 6Mbaud base
 SerOpen (&port, SERIALPORT, 38400); // default we know just works

 s = strdup("AP 40000000\r");   // Set address to what should be the start of internal flash
 converse(&port, &s, TIMEOUT);
 printf("set address 0 -> %s\n", s);

 
 free(s);  s = strdup("RD\r");
 converse(&port, &s, TIMEOUT);
 if (s) values(s);
 
 for (i = 0; i < 4096; i++) {  // the internal rom is only 0x1000 long. (0x0000-0x1000)
   free(s);  s = strdup("RD\r");
   converse(&port, &s, TIMEOUT);

   if (s) {
     values(s);
   } else {
      free(s);  s = strdup("?");
      converse(&port, &s, TIMEOUT);
   }
 }

 fwrite(buff, 1, 4096, output);
 fclose(output);

 SerFini (&port);

  return 0;
}


void chomp(char * string) {
  int target;
  
  target = strcspn(string,"\r\n");
  string[target] = '\0';
  
}


void values(char *s){

  unsigned long a;
  uint8_t d;
     if (strlen(s) < 23) { printf("Arg, partial reply!?? >%s<\n", s); return; }

     chomp(s+5);     
   //  printf("%s\n", s+5); 
     
     if (strstr(s+5, "Addr") == (s+5)) {
      //printf("Yes.\n");
      
      a = strtoul(s+9, NULL, 16);   //printf( "0x%08X", a);
      d = strtoul(s+22, NULL, 16); // printf( " 0x%02X\n", d);
      
      if ((a & 0x0F) == 0) { printf("0x%08X\r", a); fflush(stdout); }
      
      a &= 0xFFFF;
      buff[a] = d;
      
     // if ((a & 0x0F) == 0) { printf("."); fflush(stdout); }
     }

}

