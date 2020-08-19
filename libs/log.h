#ifndef LOG_H
#define LOG_H

#if DEBUG
#define thorq_debug(txt) \
	fprintf(stdout, "%-42sLINE %-4i FUNC %-6s OUTPUT " txt "\n", __FILE__, __LINE__, __func__); \
	fflush(stdout);
#define thorq_error(txt) \
	fprintf(stderr, "%-42sLINE %-4i FUNC %-6s OUTPUT " txt "\n", __FILE__, __LINE__, __func__); \
	fflush(stderr);
#define thorq_debug_fmt(fmt, ...) \
	fprintf(stdout, "%-42sLINE %-4i FUNC %-6s OUTPUT " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); \
	fflush(stdout);
#define thorq_error_fmt(fmt, ...) \
	fprintf(stderr, "%-42sLINE %-4i FUNC %-6s OUTPUT " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__); \
	fflush(stderr);
#else
#define thorq_debug(txt) \
	fprintf(stdout, txt "\n"); \
	fflush(stdout);
#define thorq_error(txt) \
	fprintf(stderr, txt "\n"); \
	fflush(stderr);
#define thorq_debug_fmt(fmt, ...) \
	fprintf(stdout, fmt "\n", __VA_ARGS__); \
	fflush(stdout);
#define thorq_error_fmt(fmt, ...) \
	fprintf(stderr, fmt "\n", __VA_ARGS__); \
	fflush(stderr);
#endif

#endif // LOG_H
