/*
 *  Try different format string lengths.
 */

/*===
length sum: 2147713027.000000
===*/

char buf[65536 + 1024];
double len_sum = 0.0;

void test_2(duk_context *ctx, int fmt_len, va_list ap) {
	int i;

	for (i = 0; i < fmt_len; i++) {
		buf[i] = 'x';
	}
	buf[fmt_len + 0] = '%';
	buf[fmt_len + 1] = 'd';
	buf[fmt_len + 2] = '\0';

	duk_push_vsprintf(ctx, buf, ap);
	len_sum += (double) duk_get_length(ctx, -1);  /* trivial "checksum" */
	duk_pop(ctx);
}

void test_1(duk_context *ctx, int fmt_len, ...) {
	va_list ap;

	va_start(ap, fmt_len);
	test_2(ctx, fmt_len, ap);
	va_end(ap);
}

void test(duk_context *ctx) {
	int fmt_len;

	/* Note: don't reuse 'ap' in multiple calls, so the fmt_len loop
	 * is here.
	 */

	for (fmt_len = 0; fmt_len <= 65536; fmt_len++) {
		test_1(ctx, fmt_len, 123, 234, 345);
	}

	/* Length sequence is 3, 4, ..., 65539 ->
	 * (65539 + 3)/2 * (65539 - 3 + 1) = 2147713027
	 *
	 * (See test-push-sprintf.c for comments.)
	 */
	printf("length sum: %lf\n", len_sum);
}

