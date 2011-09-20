#include "fitz.h"

/* Warning context
 * TODO: move into fz_context
 */
enum { LINE_LEN = 160, LINE_COUNT = 25 };

static char warn_message[LINE_LEN] = "";
static int warn_count = 0;

void fz_flush_warnings(void)
{
	if (warn_count > 1)
		fprintf(stderr, "warning: ... repeated %d times ...\n", warn_count);
	warn_message[0] = 0;
	warn_count = 0;
}

void fz_warn(char *fmt, ...)
{
	va_list ap;
	char buf[LINE_LEN];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	if (!strcmp(buf, warn_message))
	{
		warn_count++;
	}
	else
	{
		fz_flush_warnings();
		fprintf(stderr, "warning: %s\n", buf);
		fz_strlcpy(warn_message, buf, sizeof warn_message);
		warn_count = 1;
	}
}

/* Error context */

static void throw(fz_error_context *ex)
{
	if (ex->top >= 0)
		longjmp(ex->stack[ex->top].buffer, 1);
	else {
		fprintf(stderr, "uncaught exception: %s\n", ex->message);
		exit(EXIT_FAILURE);
	}
}

void fz_push_try(fz_error_context *ex)
{
	assert(ex != NULL);
	if (ex->top + 1 >= nelem(ex->stack))
	{
		fprintf(stderr, "exception stack overflow!\n");
		exit(EXIT_FAILURE);
	}
	ex->top++;
}

char *fz_caught(fz_context *ctx)
{
	assert(ctx != NULL);
	assert(ctx->error != NULL);
	return ctx->error->message;
}

void fz_throw(fz_context *ctx, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(ctx->error->message, sizeof ctx->error->message, fmt, args);
	va_end(args);

	fprintf(stderr, "error: %s\n", ctx->error->message);

	throw(ctx->error);
}

void fz_rethrow(fz_context *ctx)
{
	throw(ctx->error);
}

/* Deprecated error bubbling */

static char error_message[LINE_COUNT][LINE_LEN];
static int error_count = 0;

static void
fz_emit_error(char what, char *location, char *message)
{
	fz_flush_warnings();

	fprintf(stderr, "%c %s%s\n", what, location, message);

	if (error_count < LINE_COUNT)
	{
		fz_strlcpy(error_message[error_count], location, LINE_LEN);
		fz_strlcat(error_message[error_count], message, LINE_LEN);
		error_count++;
	}
}

int
fz_get_error_count(void)
{
	return error_count;
}

char *
fz_get_error_line(int n)
{
	return error_message[n];
}

fz_error
fz_error_make_imp(const char *file, int line, const char *func, char *fmt, ...)
{
	va_list ap;
	char one[LINE_LEN], two[LINE_LEN];

	error_count = 0;

	snprintf(one, sizeof one, "%s:%d: %s(): ", file, line, func);
	va_start(ap, fmt);
	vsnprintf(two, sizeof two, fmt, ap);
	va_end(ap);

	fz_emit_error('+', one, two);

	return -1;
}

fz_error
fz_error_note_imp(const char *file, int line, const char *func, fz_error cause, char *fmt, ...)
{
	va_list ap;
	char one[LINE_LEN], two[LINE_LEN];

	snprintf(one, sizeof one, "%s:%d: %s(): ", file, line, func);
	va_start(ap, fmt);
	vsnprintf(two, sizeof two, fmt, ap);
	va_end(ap);

	fz_emit_error('|', one, two);

	return cause;
}

void
fz_error_handle_imp(const char *file, int line, const char *func, fz_error cause, char *fmt, ...)
{
	va_list ap;
	char one[LINE_LEN], two[LINE_LEN];

	snprintf(one, sizeof one, "%s:%d: %s(): ", file, line, func);
	va_start(ap, fmt);
	vsnprintf(two, sizeof two, fmt, ap);
	va_end(ap);

	fz_emit_error('\\', one, two);
}

fz_error
fz_error_make_impx(char *fmt, ...)
{
	va_list ap;
	char buf[LINE_LEN];

	error_count = 0;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	fz_emit_error('+', "", buf);

	return -1;
}

fz_error
fz_error_note_impx(fz_error cause, char *fmt, ...)
{
	va_list ap;
	char buf[LINE_LEN];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	fz_emit_error('|', "", buf);

	return cause;
}

void
fz_error_handle_impx(fz_error cause, char *fmt, ...)
{
	va_list ap;
	char buf[LINE_LEN];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	fz_emit_error('\\', "", buf);
}
