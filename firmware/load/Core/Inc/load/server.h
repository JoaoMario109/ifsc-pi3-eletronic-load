#ifndef _LOAD_SERVER_H_
#define _LOAD_SERVER_H_

typedef enum server_tx_state
{
	SERVER_TX_IDLE = 0,
	SERVER_TX_HALF_COMPLETE
} server_tx_state_t;

void server_init();

void server_update();

#endif // !_LOAD_SERVER_H_