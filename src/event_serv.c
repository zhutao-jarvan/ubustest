#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"

static struct ubus_context *ctx;
static struct blob_buf b;

int main(int argc, char **argv)
{
	int ret;
	const char *ubus_socket = NULL;

	uloop_init();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	blob_buf_init(&b, 0);
	blobmsg_add_string(&b, "interface", "eth0.5");
	ubus_send_event(ctx, "ifup", b.head);
	
	ubus_free(ctx);
	uloop_done();

	return 0;
}

