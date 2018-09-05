#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"


static struct ubus_context * ctx = NULL;
static struct blob_buf b;
static const char * cli_path;
 

static const struct blobmsg_policy query_policy[] = {
	{.name = "city", .type = BLOBMSG_TYPE_STRING},
};

static int timeout = 30;
 

static void query_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	char *str;

	if (!msg)
		return;


	str = blobmsg_format_json(msg, true);
	printf("query result: %s\n", str);
	free(str);
}

static int client_ubus_call()
{
	unsigned int id;
	int ret;

	blob_buf_init(&b, 0);

	blobmsg_add_string(&b, "city", "chengdu");
	ret = ubus_lookup_id(ctx, "area_number", &id);
	if (ret != UBUS_STATUS_OK) {
		printf("lookup chengdu failed\n");
		return ret;
	} else {
		printf("lookup chengdu successs\n");
	}

	return ubus_invoke(ctx, id, "query", b.head, query_cb, NULL, timeout * 1000);
}

static int client_ubus_init(const char *path)
{
	uloop_init();
	cli_path = path;

	ctx = ubus_connect(path);
	if (!ctx) {
		printf("ubus connect failed\n");
		return -1;
	}

	printf("connected as %08x\n", ctx->local_id);

	return 0;
}

static void client_ubus_done(void)
{
	if (ctx)
		ubus_free(ctx);
}

int main(int argc, char * argv[])
{
	char * path = NULL;

	if (UBUS_STATUS_OK != client_ubus_init(path)) {
		printf("ubus connect failed!\n");
		return -1;
	}

	client_ubus_call(); 
	client_ubus_done(); 

	return 0;

}

