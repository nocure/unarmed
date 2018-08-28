#include <string.h>
#include "lwip/tcp.h"

static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
		err_t err);
static err_t server_poll(void *arg, struct tcp_pcb *pcb);
static void server_err(void *arg, err_t err);

void my_server_init(void) {
	struct tcp_pcb *pcb;

	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 8000); //server port for incoming connection
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, server_accept);
}

static void server_close(struct tcp_pcb *pcb) {
	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_close(pcb);

	printf("\nserver_close(): Closing...\n");
}

static err_t server_accept(void *arg, struct tcp_pcb *pcb, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	tcp_setprio(pcb, TCP_PRIO_MIN);

	tcp_arg(pcb, NULL);

	tcp_recv(pcb, server_recv);

	tcp_err(pcb, server_err);

	tcp_poll(pcb, server_poll, 4); //every two seconds of inactivity of the TCP connection

	tcp_accepted(pcb);
	printf("\nserver_accept(): Accepting incoming connection on server...\n");
	return ERR_OK;
}

static err_t server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p,
		err_t err) {
	char *string;
	int length;
	LWIP_UNUSED_ARG(arg);

	if (err == ERR_OK && p != NULL) {
		tcp_recved(pcb, p->tot_len);

		string = p->payload;
		length = strlen(string);
		printf("\nserver_recv(): Incoming string is %s\n", string);
		printf("\nserver_recv(): String length is %d byte\n", length);
		printf("server_recv(): pbuf->len is %d byte\n", p->len);
		printf("server_recv(): pbuf->tot_len is %d byte\n", p->tot_len);
		printf("server_recv(): pbuf->next is %d\n", p->next);

		pbuf_free(p);
		server_close(pcb);
	} else {
		printf("\nserver_recv(): Errors-> ");
		if (err != ERR_OK)
			printf("1) Connection is not on ERR_OK state, but in %d state->\n",
					err);
		if (p == NULL)
			printf("2) Pbuf pointer p is a NULL pointer->\n ");
			printf("server_recv(): Closing server-side connection...");

		pbuf_free(p);
		server_close(pcb);
	}

	return ERR_OK;
}

/*
static err_t server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
	LWIP_UNUSED_ARG(len);
	LWIP_UNUSED_ARG(arg);

	printf(
			"\nserver_sent(): Correctly ACK'ed, closing server-side connection...\n");
	server_close(pcb);

	return ERR_OK;
}
*/

static err_t server_poll(void *arg, struct tcp_pcb *pcb) {
	static int counter = 1;
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(pcb);

	printf("\nserver_poll(): Call number %d\n", counter++);

	return ERR_OK;
}

static void server_err(void *arg, err_t err) {
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	printf("\nserver_err(): Fatal error, exiting...\n");

	return;
}
