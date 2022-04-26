#ifndef USER_INCLUDE_LOG_H_
#define USER_INCLUDE_LOG_H_

#include <stdarg.h>

/* number of log lines */
#define QUEUE_SIZE  0x20
#define QUEUE_MASK  (QUEUE_SIZE-1)

/* uncomment to print everything */
#define LOG_DEBUG

typedef struct {
    char *queue[QUEUE_SIZE];
    int first;
    int last;
} log_queue_t;

int ets_vsprintf(char *str, const char *format, va_list arg);
int ets_vsnprintf(char *buffer, size_t size, const char *format, va_list arg);

int asprintf(char **strp, const char *format, ...);
int vasprintf(char **strp, const char *format, va_list ap);
int log_printf(const char *format, ...);
void log_put_char(char c);
void init_log_queue();
char *get_str_from_queue(bool contin);

#endif /* USER_INCLUDE_LOG_H_ */
