/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Example of how the collect primitive works.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"
#include "net/netstack.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <integrity_utils.h>



// #############################################################################
//         IT WAS PREVIOUSLY IMPLEMENTED INSIDE integrity_utils.c              #
// #############################################################################


// CRC32 implementation from
//    https://stackoverflow.com/a/34155178/8020845

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


// #############################################################################
//                      END integrity_utils.c code                             #
// #############################################################################


// #define LOWER_BOUND 28
// #define UPPER_BOUND 30
#define MAX_PAYLOAD_LEN 30

static struct collect_conn tc;
// Tricky way to manually count seqno
// Used for debug purpose
int count = 0;
int sink_id = 1;
// Each mote has their own period for sending packets. It may vary from 25 to 30 seconds.
// int periodic_seconds = 0;

/*---------------------------------------------------------------------------*/
PROCESS(collect_integrity_process, "Test collect process");
AUTOSTART_PROCESSES(&collect_integrity_process);
/*---------------------------------------------------------------------------*/
static void
recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  char *buf = (char *)packetbuf_dataptr();
  unsigned int received_cs;
  unsigned int checksum_verifying;
  char rcv_msg[MAX_PAYLOAD_LEN] = "";
  char rcv_cs[MAX_PAYLOAD_LEN] = "";
  int found_lim = 0;
  int cs_idx_count = 0;
  int i;
  for(i = 0; i < 35; i++){
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

  received_cs = atoi(rcv_cs);
  checksum_verifying = crc32a(rcv_msg);
  if(received_cs == checksum_verifying){
    printf("DATA recv from %d checksum %u VERIFIED %s\n",originator->u8[0],
      received_cs, rcv_msg);
  } else{
    printf("DATA recv from %d checksum %u FAILED %s\n",originator->u8[0],
      received_cs, rcv_msg);
  }

  // printf("%s\n", );
  // printf("Sink received message from node %d.%d, seqno %d, hops %d: len %d '%s'\n",
	//  originator->u8[0], originator->u8[1],
	//  seqno, hops,
	//  packetbuf_datalen(),
	//  (char *)packetbuf_dataptr());
   // printf("DATA recv from %d %s\n", originator->u8[0], (char *)packetbuf_dataptr());

}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(collect_integrity_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;
  // periodic_seconds = random_rand() % (UPPER_BOUND - LOWER_BOUND +1) + LOWER_BOUND;
  // printf("I send packets every %d seconds!!\n", periodic_seconds);

  PROCESS_BEGIN();

  collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

  if(linkaddr_node_addr.u8[0] == sink_id &&
     linkaddr_node_addr.u8[1] == 0) {
       printf("I am sink\n");
       collect_set_sink(&tc, 1);
  }

  /* Allow some time for the network to settle. */
  etimer_set(&et, 120 * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {

    /* Send a packet every 30 seconds. */
    if(etimer_expired(&periodic)) {
      etimer_set(&periodic, CLOCK_SECOND * 30);
      etimer_set(&et, random_rand() % (CLOCK_SECOND * 30));
    }

    PROCESS_WAIT_EVENT();


    if(etimer_expired(&et)) {
      static linkaddr_t oldparent;
      const linkaddr_t *parent;
      char buf[MAX_PAYLOAD_LEN] = "";
      unsigned char msg[MAX_PAYLOAD_LEN] = "";
      char checksum_string[MAX_PAYLOAD_LEN] = "";

      sprintf(msg, "seqn %d;", count);

      unsigned int checksum = crc32a(msg);
      sprintf(checksum_string, "%u", checksum);

      printf("DATA sent to %d|checksum %u|seqn %d\n",sink_id, checksum, count);
      // sprintf(buf, "seqn %d", count);
      strncat(buf, msg, sizeof(msg));
      strncat(buf, checksum_string, sizeof(checksum_string));
      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(),"%s", buf) + 1);
      collect_send(&tc, 15);
      count = count + 1;

      parent = collect_parent(&tc);
      if(!linkaddr_cmp(parent, &oldparent)) {
        if(!linkaddr_cmp(&oldparent, &linkaddr_null)) {
          // printf("#L %d 0\n", oldparent.u8[0]);
        }
        if(!linkaddr_cmp(parent, &linkaddr_null)) {
          // printf("#L %d 1\n", parent->u8[0]);
        }
        linkaddr_copy(&oldparent, parent);
      }
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
