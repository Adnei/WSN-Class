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
#include "powertrace.h"
#include "net/netstack.h"

#include <stdio.h>

// #define LOWER_BOUND 28
// #define UPPER_BOUND 30
#define MAX_PAYLOAD_LEN		30

static struct collect_conn tc;
// Tricky way to manually count seqno
// Used for debug purpose
int count = 0;
// Each mote has their own period for sending packets. It may vary from 25 to 30 seconds.
// int periodic_seconds = 0;

/*---------------------------------------------------------------------------*/
PROCESS(example_collect_process, "Test collect process");
AUTOSTART_PROCESSES(&example_collect_process);
/*---------------------------------------------------------------------------*/
static void
recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  printf("Sink received message from node %d.%d, seqno %d, hops %d: len %d '%s'\n",
	 originator->u8[0], originator->u8[1],
	 seqno, hops,
	 packetbuf_datalen(),
	 (char *)packetbuf_dataptr());
   printf("DATA recv '%s' from %d\n", (char *)packetbuf_dataptr(), originator->u8[0]);
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_collect_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;
  // periodic_seconds = random_rand() % (UPPER_BOUND - LOWER_BOUND +1) + LOWER_BOUND;
  // printf("I send packets every %d seconds!!\n", periodic_seconds);

  PROCESS_BEGIN();

  // printf("Power Trace: Every 5 secs\n");
  powertrace_start(CLOCK_SECOND*5);

  collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);

  if(linkaddr_node_addr.u8[0] == 1 &&
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
      char buf[MAX_PAYLOAD_LEN];

      // printf("Sending -- seqno %d\n", count);
      printf("DATA send to %d 'Hello %d'\n",
            1, count);
      sprintf(buf, "Hello %d from the source", count);
      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
				  "%s", buf) + 1);
      collect_send(&tc, 15);
      count = count + 1;

      parent = collect_parent(&tc);
      if(!linkaddr_cmp(parent, &oldparent)) {
        if(!linkaddr_cmp(&oldparent, &linkaddr_null)) {
          printf("#L %d 0\n", oldparent.u8[0]);
        }
        if(!linkaddr_cmp(parent, &linkaddr_null)) {
          printf("#L %d 1\n", parent->u8[0]);
        }
        linkaddr_copy(&oldparent, parent);
      }
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
