#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"


static struct ubus_context *ctx;
static struct blob_buf b;


static const struct ubus_method notify_methods[] = {};

static struct ubus_object_type notify_object_type =
	UBUS_OBJECT_TYPE("adjust_time", notify_methods);

static struct ubus_object notify_object = {
	.name = "adjust_time",
	.type = &notify_object_type,
	.methods = notify_methods,
	.n_methods = ARRAY_SIZE(notify_methods),
};

static void notify_loop(void)
{
	char buf[256];   
	
	while (1) {
		time_t t = time(NULL);
		memset(buf, 256, 0);
		strftime(buf, 255, "%Y%m%d%H%M%S", localtime(&t));

		blob_buf_init(&b, 0);
		blobmsg_add_string(&b, "time", buf);
		
		ubus_notify(ctx, &notify_object, "now", b.head, -1);

		sleep(5);
	}
}


void
termination_handler(int s)
{
	printf("*** recv signal: %d\n", s);
	ubus_remove_object(ctx, &notify_object);
	exit(1);
}

static void init_signals(void)
{
    struct sigaction sa;

	printf("add signal handler!\n");
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    /* Trap SIGTERM */
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        printf("set SIGTERM fail\n");
        exit(1);
    }

    /* Trap SIGQUIT */
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        printf("set SIGINT fail\n");
        exit(1);
    }

    /* Trap SIGINT */
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        printf("set SIGINT fail\n");
        exit(1);
    }
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

	ret = ubus_add_object(ctx, &notify_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));

	init_signals();
	notify_loop();

	uloop_run();
	
	ubus_free(ctx);
	uloop_done();

	return 0;
}

