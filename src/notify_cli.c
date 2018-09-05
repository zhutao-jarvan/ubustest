#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"

static struct ubus_context *ctx;
static struct blob_buf b;
static struct ubus_subscriber notify;
static uint32_t obj_id;

enum {
    ADJUST_NOW,
    __ADJUST_MAX
};

static const struct blobmsg_policy now_policy[__ADJUST_MAX] = {
    [ADJUST_NOW] = { .name = "time", .type = BLOBMSG_TYPE_STRING }
};

static int adjust_now(struct ubus_context *ctx, struct ubus_object *obj,
			      struct ubus_request_data *req,
			      const char *method, struct blob_attr *msg)
{
	//printf("adjust recieve time!\n");
	char *str;

	if (!msg)
		return;

	str = blobmsg_format_json(msg, true);
	printf("adjust_now: %s\n", str);
	free(str);
}
				  
static void adjust_server_remove(struct ubus_context *ctx,
					struct ubus_subscriber *obj, uint32_t id)
{
	printf("adjust time server exit!");
}


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

	notify.cb = adjust_now;
	notify.remove_cb = adjust_server_remove;

	ret = ubus_register_subscriber(ctx, &notify);
	if (ret) {
		fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));
		return -1;
	}

	ret = ubus_lookup_id(ctx, "adjust_time", &obj_id);
	if (ret) {
		fprintf(stderr, "Failed to look up id adjust_time: %s\n", ubus_strerror(ret));
		return -1;
	}

	ret = ubus_subscribe(ctx, &notify, obj_id);
	if (ret) {
		fprintf(stderr, "Failed to subscribe adjust_time: %s\n", ubus_strerror(ret));
		return -1;
	}

	uloop_run();
	
	ubus_free(ctx);
	uloop_done();

	return 0;
}

