#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL
} log_level_t;

#ifndef DISABLE_SHORT_LOG_NAMES
typedef enum
{
	TRACE = 0,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL
} short_log_level_t;
#endif

struct logger_ctx_t;
typedef struct logger_ctx_t logger_ctx_t;
typedef int (*start_msg_func)(void *data, int level);
typedef ssize_t (*write_func)(void *data, const char *buf, size_t len);
typedef int (*end_msg_func)(void *data);

struct logger_ctx_t
{
	char *name;
	int min_level;
	
	void *data;
	
	start_msg_func start_msg;
	write_func write;
	end_msg_func end_msg;
	
	FILE *fp;
};

extern int logger_default_level;

void logger_init();
logger_ctx_t *get_logger(const char *name);

void Log_impl(logger_ctx_t *l, const char *file, int line, const char *func, int level, const char *fmt, ...);
void LogAs_impl(const char *name, const char *file, int line, const char *func, int level, const char *fmt, ...);

void StartLog_impl(logger_ctx_t *l, const char *file, int line, const char *func, int level);
void PartialLog_impl(logger_ctx_t *l, const char *fmt, ...);
void EndLog_impl(logger_ctx_t *l);

void StartLogAs_impl(const char *name, const char *file, int line, const char *func, int level);
void PartialLogAs_impl(const char *name, const char *fmt, ...);
void EndLogAs_impl(const char *name);

#define Log(level, fmt, ...) Log_impl(NULL, __FILE__, __LINE__, __func__, level,           fmt, ##__VA_ARGS__)
#define LogTrace(fmt, ...)   Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define LogDebug(fmt, ...)   Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...)    Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...)    Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define LogError(fmt, ...)   Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LogFatal(fmt, ...)   Log_impl(NULL, __FILE__, __LINE__, __func__, LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define StartLog(level)      StartLog_impl(NULL, __FILE__, __LINE__, __func__, level)
#define PartialLog(fmt, ...) PartialLog_impl(NULL, fmt, ##__VA_ARGS__)
#define EndLog               EndLog_impl(NULL)

#define LogTo(l_ctx, level, fmt, ...) Log_impl(l_ctx, __FILE__, __LINE__, __func__, level,           fmt, ##__VA_ARGS__)
#define LogTraceTo(l_ctx, fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define LogDebugTo(l_ctx, fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LogInfoTo(l_ctx,  fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LogWarnTo(l_ctx,  fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define LogErrorTo(l_ctx, fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LogFatalTo(l_ctx, fmt, ...)   Log_impl(l_ctx, __FILE__, __LINE__, __func__, LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define StartLogTo(l_ctx, level)      StartLog_impl(l_ctx, __FILE__, __LINE__, __func__, level)
#define PartialLogTo(l_ctx, fmt, ...) PartialLog_impl(l_ctx, fmt, ##__VA_ARGS__)
#define EndLogTo(l_ctx)               EndLog_impl(l_ctx)

#define LogAs(name, level, fmt, ...) LogAs_impl(name, __FILE__, __LINE__, __func__, level,           fmt, ##__VA_ARGS__)
#define LogTraceAs(name, fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define LogDebugAs(name, fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LogInfoAs(name,  fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LogWarnAs(name,  fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define LogErrorAs(name, fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LogFatalAs(name, fmt, ...)   LogAs_impl(name, __FILE__, __LINE__, __func__, LOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define StartLogAs(name, level)      StartLogAs_impl(name, __FILE__, __LINE__, __func__, level)
#define PartialLogAs(name, fmt, ...) PartialLogAs_impl(name, fmt, ##__VA_ARGS__)
#define EndLogAs(name)               EndLogAs_impl(name)

#ifdef __cplusplus
}
#endif

#endif