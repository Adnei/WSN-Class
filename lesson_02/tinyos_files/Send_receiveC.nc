#include "Send_receive.h"
#include <Timer.h>
/* #include "stdlib.h" */
/* #include "stdio.h" */
/* #include "printf.h" */

module Send_receiveC{
	uses interface Boot;
	uses interface Leds;
	uses interface Timer<TMilli> as Timer0;
	uses interface Packet;
	uses interface AMPacket;
	uses interface AMSend;
	uses interface SplitControl as AMControl;
	uses interface Receive;
}
implementation{
	uint32_t counter;
	bool busy=FALSE;
	message_t pkt;

	event void Boot.booted(){
		/* printf("Im booting. ID: %d", TOS_NODE_ID); */
		/* printf("Im booting"); */
		/* dbg("hello") */
		call AMControl.start();
	}

	event void AMControl.startDone(error_t error){
		if(error == SUCCESS){
			call Timer0.startPeriodic(TIMER_PERIOD);
		}else{
			call AMControl.start();
		}
	}

	event void AMControl.stopDone(error_t error){
	}

	event void Timer0.fired(){
		SendCMsg* btrpkt=(SendCMsg*)(call Packet.getPayload(&pkt, sizeof (SendCMsg)));
		counter++;
		btrpkt->nodeid=TOS_NODE_ID;
		btrpkt->counter=counter;
		if(call AMSend.send(AM_BROADCAST_ADDR,&pkt,sizeof(SendCMsg))==SUCCESS){
			busy=TRUE;
		}
	}

	event void AMSend.sendDone(message_t *msg, error_t error){
		if (&pkt == msg) {
			busy = FALSE;
		}
	}

		event message_t * Receive.receive(message_t *msg, void *payload, uint8_t len){

			SendCMsg* btrpkt = (SendCMsg*)payload;
			call Leds.set(btrpkt->counter);

			return msg;
	}
}
