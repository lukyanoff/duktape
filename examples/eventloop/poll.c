/*
 *  C wrapper for poll().
 */

#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>

#include "duktape.h"

static int poll_poll(duk_context *ctx) {
	int timeout = duk_to_int(ctx, 1);
	int i, n, nchanged;
	int fd, rc;
	struct pollfd fds[20];
	struct timespec ts;

	memset(fds, 0, sizeof(fds));

	n = 0;
	duk_enum(ctx, 0, 0 /*enum_flags*/);
	while (duk_next(ctx, -1, 0)) {
		if ((size_t) n >= sizeof(fds) / sizeof(struct pollfd)) {
			return -1;
		}

		/* [... enum key] */
		duk_dup_top(ctx);  /* -> [... enum key key] */
		duk_get_prop(ctx, 0);  /* -> [... enum key val] */
		fd = duk_to_int(ctx, -2);

		duk_push_string(ctx, "events");
		duk_get_prop(ctx, -2);  /* -> [... enum key val events] */

		fds[n].fd = fd;
		fds[n].events = duk_to_int(ctx, -1);
		fds[n].revents = 0;

		duk_pop_n(ctx, 3);  /* -> [... enum] */

		n++;
	}
	/* leave enum on stack */

	memset(&ts, 0, sizeof(ts));
	ts.tv_nsec = (timeout % 1000) * 1000000;
	ts.tv_sec = timeout / 1000;

	/*rc = ppoll(fds, n, &ts, NULL);*/
	rc = poll(fds, n, timeout);
	if (rc < 0) {
		duk_error(ctx, DUK_ERR_ERROR, "%s (errno=%d)", strerror(errno), errno);
	}

	duk_push_array(ctx);
	nchanged = 0;
	for (i = 0; i < n; i++) {
		/* update revents */

		if (fds[i].revents) {
			duk_push_int(ctx, fds[i].fd);  /* -> [... retarr fd] */
			duk_put_prop_index(ctx, -2, nchanged);
			nchanged++;
		}

		duk_push_int(ctx, fds[i].fd);  /* -> [... retarr key] */
		duk_get_prop(ctx, 0);  /* -> [... retarr val] */
		duk_push_string(ctx, "revents");
		duk_push_int(ctx, fds[i].revents);  /* -> [... retarr val "revents" fds[i].revents] */
		duk_put_prop(ctx, -3);  /* -> [... retarr val] */
		duk_pop(ctx);
	}

	/* [retarr] */

	return 1;
}

void poll_register(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "Poll");
	duk_push_object(ctx);

	duk_push_c_function(ctx, poll_poll, 2);
	duk_put_prop_string(ctx, -2, "poll");

	duk_push_int(ctx, POLLIN);
	duk_put_prop_string(ctx, -2, "POLLIN");
	duk_push_int(ctx, POLLPRI);
	duk_put_prop_string(ctx, -2, "POLLPRI");
	duk_push_int(ctx, POLLOUT);
	duk_put_prop_string(ctx, -2, "POLLOUT");
#if 0
	/* Linux 2.6.17 and upwards, requires _GNU_SOURCE etc, not added
	 * now because we don't use it.
	 */
	duk_push_int(ctx, POLLRDHUP);
	duk_put_prop_string(ctx, -2, "POLLRDHUP");
#endif
	duk_push_int(ctx, POLLERR);
	duk_put_prop_string(ctx, -2, "POLLERR");
	duk_push_int(ctx, POLLHUP);
	duk_put_prop_string(ctx, -2, "POLLHUP");
	duk_push_int(ctx, POLLNVAL);
	duk_put_prop_string(ctx, -2, "POLLNVAL");

	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

