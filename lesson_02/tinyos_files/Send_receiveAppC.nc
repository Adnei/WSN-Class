#include "Send_receive.h"

configuration Send_receiveAppC {
}
implementation {
	components MainC;
	components LedsC;
	/* components printfC; */
	components Send_receiveC as App;
	components new TimerMilliC() as Timer0;
	components ActiveMessageC;
	components new AMSenderC(AM_CHANNEL);
	components new AMReceiverC(AM_CHANNEL);

	App.Boot -> MainC;
	App.Leds -> LedsC;
	App.Timer0 -> Timer0;
	App.Packet -> AMSenderC;
	App.AMPacket -> AMSenderC;
	App.AMSend -> AMSenderC;
	App.AMControl -> ActiveMessageC;
	App.Receive -> AMReceiverC;
}
