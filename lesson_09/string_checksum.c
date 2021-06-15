#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//  https://stackoverflow.com/a/34957656/8020845
int split (const char *txt, char delim, char ***tokens)
{
    int *tklen, *t, count = 1;
    char **arr, *p = (char *) txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc (count, sizeof (int));
    for (p = (char *) txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = malloc (count * sizeof (char *));
    t = tklen;
    p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
    while (*txt != '\0')
    {
        if (*txt == delim)
        {
            p = *arr++ = calloc (*(t++) + 1, sizeof (char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free (tklen);
    return count;
}


// ---------------------------- reverse --------------------------------

// Reverses (reflects) bits in a 32-bit word.
unsigned reverse(unsigned x) {
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) |
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

// ----------------------------- crc32a --------------------------------

/* This is the basic CRC algorithm with no optimizations. It follows the
logic circuit as closely as possible. */

unsigned int crc32a(unsigned char *message) {
   int i, j;
   unsigned int byte, crc;

   i = 0;
   crc = 0xFFFFFFFF;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      byte = reverse(byte);         // 32-bit reversal.
      for (j = 0; j <= 7; j++) {    // Do eight times.
         if ((int)(crc ^ byte) < 0)
              crc = (crc << 1) ^ 0x04C11DB7;
         else crc = crc << 1;
         byte = byte << 1;          // Ready next msg bit.
      }
      i = i + 1;
   }
   return reverse(~crc);
}

int main(void){
  char my_str[35];
  sprintf(my_str, "seqn %d;", 10);
  unsigned int checksum = crc32a(my_str);
  unsigned int received_cs;
  char buf[35] = "";
  char checksum_string[35] = "";

  sprintf(checksum_string, "%u", checksum);

  strncat(buf, my_str, sizeof(my_str));
  strncat(buf, checksum_string, sizeof(checksum_string));

  printf("my_string = %s\n", my_str);
  printf("checksum = %u\n", checksum);
  printf("buf = %s\n", buf);

  char rcv_msg[35]="";
  char rcv_cs[35]="";
  int found_lim = 0;
  int cs_idx_count = 0;
  int i = 0;

  for(i; i < 35; i++){
    if(buf[i] == '\0'){
      rcv_cs[cs_idx_count] = '\0';
      rcv_msg[i] = '\0';
      break;
    }
    if(found_lim){
      rcv_cs[cs_idx_count] = buf[i];
      cs_idx_count++;
    } else{
      rcv_msg[i] = buf[i];
    }
    if(buf[i] == ';')
      found_lim = 1;
  }

  // if(i < 34){
  //   for(i; i < 35; i++){
  //     printf("i - %d\n",i);
  //     rcv_msg[i] = 'O';
  //   }
  // }

  received_cs = atoi(rcv_cs);
  // received_cs = atoi(tokens[1]);
  printf("received message = %s\n", rcv_msg);
  printf("received checksum = %u\n", received_cs);

  if(received_cs == checksum){
    printf("RECEIVED IS EQUALS TO THE ORIGINAL\n");
  }


  return 0;
}
