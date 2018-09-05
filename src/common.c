#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"


static struct ubus_context *local_ctx = NULL;
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


static void ubus_connection_lost(struct ubus_context *ctx)
{
	uloop_done();
	ubus_free(ctx);
	ctx = NULL;
	ubus_reconn_timer(NULL);
}
 

int init_ubus(struct ubus_context **ctxp, char *path)
{
	struct ubus_context *ctx = NULL;

	uloop_init();

	ctx = ubus_connect(path);
	if (!ctx) {
		printf("ubus connect failed\n");
		return -1;
	}
	ctx->connection_lost = ubus_connection_lost;
	ubus_add_uloop(ctx);

	*ctxp = ctx;
	local_ctx = ctx;
	return UBUS_STATUS_OK;
}

