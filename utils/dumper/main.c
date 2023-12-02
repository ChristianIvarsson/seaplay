#define _GNU_SOURCE
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

int StrAccumChar( char ** s, char  c) ;
int termCom(SSLS_t * this, char **string) ;

int main(void) {

 SSLS_t port;
 char * s;
 long int i;
 FILE * output;
 
 printf("Pausing movie...\n");
 
 output = fopen("readback.bin", "wb");
 
 SerInit (&port);
 //SerOpen (&port, SERIALPORT, 1000000); // BR 3 with 3Mbaud base, BR 6 with 6Mbaud base
 SerOpen (&port, SERIALPORT, 38400); // default we know just works

// Set address to what should be the start of internal flash

/*
   PCB100434407
*/
 s = strdup("AP 40000000\r"); // the firmware address for larger drives  

/*
  PCB100466824
*/
// s = strdup("AP 00100000\r");   // the firmware address for smaller drives
 
 
// waitConverse(&port, &s, TIMEOUT*3, TIMEOUT/2);
 printf("Setting address of memory inquiry...\n");
 termCom(&port, &s);
 
 printf("Reading tids and bits from target system...\n");
 free(s);  s = strdup("RD\r");
 termCom(&port, &s);
 if (s) values(s);
 
 for (i = 0; i < 4096; i++) {  // the internal rom is only 0x1000 long. (0x0000-0x1000)
   free(s);  s = strdup("RD\r");
   termCom(&port, &s);

   if (s) {
     values(s);
   } else {
      free(s);  s = strdup("?");
      termCom(&port, &s);
   }
 }
 printf("\nWriting results to local filesystem, eeeeeee twang twang TSHHSHSHHSHHSHSHHSHSHHSHS....\n");

 fwrite(buff, 1, 4096, output);
 fclose(output);

 SerFini (&port);

  printf("And back to my movie...\n");

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
     if (strlen(s) < 23) { printf("\nArg, partial reply!?? >%s<\n", s); return; }

     chomp(s+5);     
     
     if (strstr(s+5, "Addr") == (s+5)) {
      
      a = strtoul(s+9, NULL, 16);   //printf( "0x%08X", a);
      d = strtoul(s+22, NULL, 16); // printf( " 0x%02X\n", d);
      
      if ((a & 0x0F) == 0) { printf("0x%08X\r", a); fflush(stdout); }
      
      a &= 0xFFFF;
      buff[a] = d;
      
     }

}


// These boards seem to stall for a LONG time sometimes, FSM for efficiency
int termCom(SSLS_t * this, char **string) {

  char buff[2];
  uint8_t flag = 0;
  int retval;

  if ((retval = SerWrite(this, *string, strlen(*string))) != OK) 
       return retval;
  
  free(*string);  *string = NULL;
  
  while ( read(this->fd, buff, 1) != -1) {
          
    if (StrAccumChar( string, buff[0]) == -1)     return -1;
    
    // ARG MADE ME WRITE A STATE MACHINE.
    if (buff[0] == 0x0A) {                        flag = 1;
    } else if ((buff[0] == '>') && (flag == 1)) { flag = 2;
    } else if ((buff[0] == ' ') && (flag == 2)) { return 1;
    } else                                        flag = 0;
    
  //  printf(".%s + 0x%02X @%d \n", *string, buff[0], flag);
    
  }
  
  return -1;

}


// those who see my hacks are doomed to repeat them

int StrAccumChar( char ** s, char  c) {

   char * tmp;
   if (*s) {
     if (asprintf(&tmp, "%s%c", *s, c) == -1) return -1;
   } else {
     if (asprintf(&tmp, "%c", c) == -1) return -1;
   }  
     
   free(*s);   *s = tmp;
   
   return 1;
   
}




























































