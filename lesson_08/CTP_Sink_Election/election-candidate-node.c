/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Example code that uses the annuncement module
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "lib/random.h"

#include "net/rime/announcement.h"

#include <stdio.h>
#define MAX_PAYLOAD_LEN		30

#if CONTIKI_TARGET_NETSIM
#include "ether.h"
#endif

// Tricky way to manually count seqno
// Used for debug purpose
int count = 1;

static struct collect_conn tc;
int annuncement_control = 0;
int i = 0;
int sink_id = 1;


/*---------------------------------------------------------------------------*/
PROCESS(election_announcement_process, "Election announcement process");
AUTOSTART_PROCESSES(&election_announcement_process);
/*---------------------------------------------------------------------------*/
static void
sink_recv(const linkaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  // printf("Sink received message from node %d.%d, seqno %d, hops %d: len %d '%s'\n",
	//  originator->u8[0], originator->u8[1],
	//  seqno, hops,
	//  packetbuf_datalen(),
	//  (char *)packetbuf_dataptr());
   printf("DATA recv from %d %s\n", originator->u8[0], (char *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { sink_recv };
/*---------------------------------------------------------------------------*/
static void
received_announcement(struct announcement *a, const linkaddr_t *from,
		      uint16_t id, uint16_t value)
{
	int aux = 0;
	if(annuncement_control == value && from->u8[0] == annuncement_control) {
		printf("CALLBACK!!!\n");
		announcement_remove(a);
	} else {
		aux = value;
		if(linkaddr_node_addr.u8[0] > aux){
			aux = linkaddr_node_addr.u8[0];
		}
		if(annuncement_control > aux){
			aux = annuncement_control;
		}
		annuncement_control = aux;
		announcement_set_value(a, aux);
	}
  printf("ANNOUNCEMENT recv from %d, id %d, value %d, our new value is %d\n",
	 from->u8[0], id, value, annuncement_control);

#if CONTIKI_TARGET_NETSIM
  {
    char buf[8];
    sprintf(buf, "%d", aux);
    ether_set_text(buf);
  }
#endif

}
static struct announcement election_announcement;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(election_announcement_process, ev, data)
{
	static struct etimer kill_sink_timer;
	static struct etimer et;
	static struct etimer periodic;

  PROCESS_EXITHANDLER(announcement_remove(&election_announcement);)

  PROCESS_BEGIN();

	collect_open(&tc, 130, COLLECT_ROUTER, &callbacks);
	if(linkaddr_node_addr.u8[0] == sink_id &&
		 linkaddr_node_addr.u8[1] == 0) {
		 printf("I am sink!!\n");
		 collect_set_sink(&tc, sink_id);
	}
	// Allow some time for the network to settle.
	etimer_set(&et, 30 * CLOCK_SECOND);
	PROCESS_WAIT_UNTIL(etimer_expired(&et));
	//kills current sink node after 2 minutes
	etimer_set(&kill_sink_timer, 120 * CLOCK_SECOND);

  while(1) {
		annuncement_control = 0;

		/* Send a packet every 10 seconds. */
		if(etimer_expired(&periodic)) {
			etimer_set(&periodic, CLOCK_SECOND * 10);
			etimer_set(&et, random_rand() % (CLOCK_SECOND * 10));
		}

		PROCESS_WAIT_EVENT();

		if(etimer_expired(&et)) {
			static linkaddr_t oldparent;
			const linkaddr_t *parent;
			char buf[MAX_PAYLOAD_LEN];

			printf("DATA sent to %d seqn %d\n",
						sink_id, count);
			sprintf(buf, "seqn %d", count);
			packetbuf_clear();
			packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
					"%s", buf) + 1);
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


		if(etimer_expired(&kill_sink_timer)){
			printf("sink %d is dead!\n", sink_id);
			if(linkaddr_node_addr.u8[0] == sink_id){
				printf("REMOVE SINK NOW\n");
				// NETSTACK_RADIO.off();
				// PROCESS_WAIT_UNTIL(0);
				// break;
			}

			// ----------------- SINK ELECTION ----------------------
			announcement_register(&election_announcement,
					128,
					received_announcement);
			announcement_set_value(&election_announcement, linkaddr_node_addr.u8[0]);
			etimer_set(&et, CLOCK_SECOND * 5);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			announcement_listen(1);
			sink_id = annuncement_control;

			printf("NEW SINK! --> %d\n",sink_id);

			if(sink_id == 0){
				printf("END\n");
				break;
			}

			if(linkaddr_node_addr.u8[0] == sink_id &&
				 linkaddr_node_addr.u8[1] == 0) {
				 printf("I am sink!!\n");
				 collect_set_sink(&tc, sink_id);
			}
			etimer_set(&et, 60 * CLOCK_SECOND);
			PROCESS_WAIT_UNTIL(etimer_expired(&et));

			etimer_set(&kill_sink_timer, 120 * CLOCK_SECOND);
		}
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
