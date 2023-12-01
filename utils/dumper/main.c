#include <stdint.h>  // utin8_t
#include <stdio.h> // printf, fopen
#include <stdlib.h> // strtoul
#include <string.h>
#include "returnValues.h"
#include "ssls3.h"

/*
initialize console at 1Mbaud with no echo by doing

BR 3


*/

void chomp(char * string);
void values(char *s);

uint8_t buff[65538];

int main(void) {

 SSLS_t port;
 char * s;
 long int i;
 FILE * output;
 
 output = fopen("readback.bin", "wb");
 
 s = strdup("AP 40000000\r");

 SerInit (&port);
 SerOpen (&port, "/dev/ttyUSB1", 1000000);

 converse(&port, &s, 900);
 printf("set address 0 -> %s\n", s);

 
 free(s);  s = strdup("RD\r");
 converse(&port, &s, 900);
 values(s);
 
 for (i = 0; i < 65535; i++) {
   free(s);  s = strdup("RD\r");
   converse(&port, &s, 1000);

   if (s) {
     values(s);
   } else {
      free(s);  s = strdup("?");
      converse(&port, &s, 1000);
   }
 }

 fwrite(buff, 1, 65536, output);
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

     chomp(s+5);     
   //  printf("%s\n", s+5); 
     
     if (strstr(s+5, "Addr") == (s+5)) {
      //printf("Yes.\n");
      
      a = strtoul(s+9, NULL, 16);   //printf( "0x%08X", a);
      d = strtoul(s+22, NULL, 16); // printf( " 0x%02X\n", d);
      
      if ((a & 0xFF) == 0) { printf("0x%08X\r", a); fflush(stdout); }
      
      a &= 0xFFFF;
      buff[a] = d;
      
     // if ((a & 0xFF) == 0) { printf("."); fflush(stdout); }
     }

}

