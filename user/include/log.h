#ifndef USER_INCLUDE_LOG_H_
#define USER_INCLUDE_LOG_H_

#include <stdarg.h>

/* number of log lines */
#define LSTACK_SIZE 30

/* uncomment to print everything */
#define LOG_DEBUG

typedef struct lstack_elem_t lstack_elem_t;

struct lstack_elem_t {
    lstack_elem_t *prev;
    lstack_elem_t *next;
    char *str;
};

int ets_vsprintf(char *str, const char *format, va_list arg);
int ets_vsnprintf(char *buffer, size_t size, const char *format, va_list arg);

int asprintf(char **strp, const char *format, ...);
int vasprintf(char **strp, const char *format, va_list ap);
int log_printf(const char *format, ...);
void log_put_char(char c);
bool write_to_lstack(char *str);
lstack_elem_t *get_lstack();

#endif /* USER_INCLUDE_LOG_H_ */
