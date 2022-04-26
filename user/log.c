#include "osapi.h"
#include "mem.h"

#include "log.h"

static log_queue_t log_queue;

/* delete '\n' from str */
static char ICACHE_FLASH_ATTR *trim_str_log_queue(char *str) {

    char *str_dest, *str_source; //, *pos;

    str_dest = str_source = str;

    do {
        while (*str_source == '\n') str_source++;
        if (str_source != str_dest) *str_dest = *str_source;
        if (*str_source == '\0') break;
        str_dest++;
        str_source++;
    } while (1);

    return str;
}

char ICACHE_FLASH_ATTR *get_str_from_log_queue(bool begin) {
    static int first = 0;

    if (begin) first = log_queue.first;

    if (first == log_queue.last) {
        return NULL;
    }
    char *str = log_queue.queue[first++];
    first &= QUEUE_MASK;

    return str;
}

static void ICACHE_FLASH_ATTR add_log_queue(char *str) {
    static bool contin = false;

    if (str == NULL) return;

    if (contin) {
        if (str[strlen(str)-1] == '\n') contin = false;
        str = trim_str_log_queue(str);
        if (*str == 0) {
            os_free(str);
            return;
        }
        char *buff = os_malloc(strlen(log_queue.queue[log_queue.last])+strlen(str)+1);
        if (buff == NULL) {
            log_printf("Error allocation memory. (%s:%u)\n", __FILE__, __LINE__);
            return;
        }
        os_sprintf(buff, "%s%s", log_queue.queue[log_queue.last], str);
        os_free(log_queue.queue[log_queue.last]);
        os_free(str);
        log_queue.queue[log_queue.last] = buff;
        return;
    }


    if (((log_queue.last + 1) & QUEUE_MASK) == log_queue.first) {
        os_free(log_queue.queue[log_queue.first]);
        log_queue.queue[log_queue.first++] = NULL;
        log_queue.first &= QUEUE_MASK;
    }

    if (str[strlen(str)-1] != '\n') contin = true;
    str = trim_str_log_queue(str);
    if (*str == 0) {
        os_free(str);
        return;
    }
    log_queue.queue[log_queue.last++] = str;
    log_queue.last &= QUEUE_MASK;
}

void ICACHE_FLASH_ATTR init_log_queue() {
    os_install_putc1((void *)log_put_char);
    os_bzero(&log_queue, sizeof(log_queue_t));
}


static int ICACHE_FLASH_ATTR vscprintf(const char *format, va_list ap) {
    char buff[1];
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int retval = ets_vsnprintf(buff, 1, format, ap_copy);
    va_end(ap_copy);
    return retval;
}

int ICACHE_FLASH_ATTR vasprintf(char **strp, const char *format, va_list ap) {
    int len = vscprintf(format, ap);
    if (len == -1)
        return -1;
    char *str = os_malloc((size_t) len + 1);
    if (!str)
        return -1;
    int retval = ets_vsnprintf(str, len + 1, format, ap);
    if (retval == -1) {
        os_free(str);
        return -1;
    }
    *strp = str;
    return retval;
}

int ICACHE_FLASH_ATTR asprintf(char **strp, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(strp, format, ap);
    va_end(ap);
    return retval;
}

int ICACHE_FLASH_ATTR log_printf(const char *format, ...) {
    char *buff = NULL;
    va_list ap;
    va_start(ap, format);
    int retval = vasprintf(&buff, format, ap);
    va_end(ap);

    if (buff) {
        ets_uart_printf(buff);
        add_log_queue(buff);
    }
    return retval;
}

#define PUT_CHAR_BUFF_LEN   128

void ICACHE_FLASH_ATTR log_put_char(char c) {
    static char buff[PUT_CHAR_BUFF_LEN] = {0};
    static int len = 0;

#ifndef LOG_DEBUG
    return;
#endif

    if (len < PUT_CHAR_BUFF_LEN-1) {
        buff[len++] = c;
        if (c == '\n') {
            log_printf(buff);
            len = 0;
            os_bzero(buff, PUT_CHAR_BUFF_LEN);
        }
    } else {
        log_printf(buff);
        len = 0;
        os_bzero(buff, PUT_CHAR_BUFF_LEN);
        buff[len++] = c;
        if (c == '\n') {
            log_printf(buff);
            len = 0;
            buff[len] = 0;
        }
    }
}

