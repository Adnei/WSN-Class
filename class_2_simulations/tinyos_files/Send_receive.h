#ifndef Send_receive_H
#define Send_receive_H
enum{
	AM_CHANNEL = 26,
	TIMER_PERIOD=5000
};
typedef nx_struct SendCMsg {
	nx_uint32_t nodeid;
	nx_uint32_t counter;
}SendCMsg;

#endif /* Send_receive_H */
