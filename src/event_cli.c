#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"


static struct ubus_context *ctx = NULL;
static struct ubus_event_handler listener;
static const char *cli_path = NULL;

static void ubus_reconn_timer(struct uloop_timeout *timeout)
{
	int t = 2;

	static struct uloop_timeout retry = {
		.cb = ubus_reconn_timer,
	};

	printf("[%s, %d]\n", __func__, __LINE__);
	if (ubus_reconnect(ctx, cli_path) != 0) {
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	ubus_add_uloop(ctx);
}


static void ubus_connection_lost(struct ubus_context *lctx)
{
	ubus_reconn_timer(NULL);
}
 
static void ubus_probe_device_event(struct ubus_context *ctx, struct ubus_event_handler *ev,

			  const char *type, struct blob_attr *msg)
{
	char *str;

	if (!msg)
		return;

	str = blobmsg_format_json(msg, true);
	
	printf("type: %s, str: %s\n", type, str);
	free(str);
}


int main(int argc, char * argv[])
{
	char *path = NULL;
	int ret;

	uloop_init();
	ctx = ubus_connect(path);
	if (!ctx) {
		printf("ubus connect failed\n");
		return -1;
	}
	ctx->connection_lost = ubus_connection_lost;

	ubus_add_uloop(ctx);

	listener.cb = ubus_probe_device_event;
	ret = ubus_register_event_handler(ctx, &listener, "ifup");
	if (ret) {
		printf("register event ifup error!\n");
	}

	ret = ubus_register_event_handler(ctx, &listener, "ifdown");
	if (ret) {
		printf("register event ifdown error!\n");
	}

	uloop_run();
	ubus_free(ctx);

	return 0;
}

