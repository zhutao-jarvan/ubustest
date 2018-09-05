#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <libubox/blobmsg_json.h>
#include "libubus.h"

enum {
    AREA_CITY,
	AREA_NUMBER,
    __AREA_MAX
};

static struct ubus_context *ctx;
static struct blob_buf b;

struct area_code {
	char *city;
	char *number;
	int valid;
};

#define MAX_AREA 100
static struct area_code areas[MAX_AREA] = {
		{"chengdu", 	 "028",  1},
		{"nanchong",    "0817",  1},
		{"beijing",      "010",  1}
};

static const struct blobmsg_policy area_policy[__AREA_MAX] = {
    [AREA_CITY] = { .name = "city", .type = BLOBMSG_TYPE_STRING },
	[AREA_NUMBER] = { .name = "number", .type = BLOBMSG_TYPE_STRING }
};


static int invoke_query(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__AREA_MAX];
	const char *city = NULL;
	int i;
	int find = 0;

	blobmsg_parse(area_policy, ARRAY_SIZE(area_policy), tb, blob_data(msg), blob_len(msg));
#if 0
	for (i=0; i<__AREA_MAX; i++)
		if (tb[i]) {
			printf("%d: %s\n", i, (char*)blobmsg_data(tb[i]));
		} else {
			printf("%d: *** null pointer\n", i);
		}
#endif
	blob_buf_init(&b, 0);
	if (tb[AREA_CITY]) {
		city = blobmsg_data(tb[AREA_CITY]);
		for (i=0; i<MAX_AREA; i++) {
			if (areas[i].valid && !strcmp(city, areas[i].city)) {
				find = 1;
				blobmsg_add_string(&b, "city", city);
				blobmsg_add_string(&b, "number", areas[i].number);
				break;
			}
		}
	}

	blobmsg_add_u32(&b, "find", find);	

	ubus_send_reply(ctx, req, b.head);

	return 0;
}

static int invoke_add(struct ubus_context *ctx, struct ubus_object *obj,
			struct ubus_request_data *req, const char *method,
			struct blob_attr *msg)
{
	struct blob_attr *tb[__AREA_MAX];
	char *city = NULL, *number = NULL;
	int i;
	int result = -1;

	blobmsg_parse(area_policy, ARRAY_SIZE(area_policy), tb, blob_data(msg), blob_len(msg));

	blob_buf_init(&b, 0);
	if (tb[AREA_CITY] && tb[AREA_NUMBER]) {
		city = strdup(blobmsg_data(tb[AREA_CITY]));
		number = strdup(blobmsg_data(tb[AREA_NUMBER]));
		if (!city || !number) {
			result = -2; //分配内存失败
			goto err;
		}
		
		for (i=0; i<MAX_AREA; i++) {
			if (areas[i].valid) {
			  	if (!strcmp(city, areas[i].city)) {
					result = -3; //城市已经存在
					goto out;
			  	}
			} else {
				areas[i].city = city;
				areas[i].number = number;
				areas[i].valid = 1;
				printf("add city: %s, number: %s\n", city, number);
				result = 0;
				goto out;
			}
		}

		result = -4; //最大城市数量
	}

out:
err:
	blobmsg_add_u32(&b, "result", result);  

	ubus_send_reply(ctx, req, b.head);

	return 0;
}


static const struct ubus_method invoke_methods[] = {
	UBUS_METHOD("query", invoke_query, area_policy),
	UBUS_METHOD("add", invoke_add, area_policy),
};

static struct ubus_object_type invoke_object_type =
	UBUS_OBJECT_TYPE("area_number", invoke_methods);

static struct ubus_object invoke_object = {
	.name = "area_number",
	.type = &invoke_object_type,
	.methods = invoke_methods,
	.n_methods = ARRAY_SIZE(invoke_methods),
};

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

	ret = ubus_add_object(ctx, &invoke_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));


	uloop_run();
	
	ubus_free(ctx);
	uloop_done();

	return 0;
}

