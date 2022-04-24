#include "osapi.h"
#include "mem.h"

#include "log.h"

static lstack_elem_t *lstack = NULL;

lstack_elem_t* ICACHE_FLASH_ATTR get_lstack() {
    return lstack;
}

/* delete color symbols, '\n' from str */
static char* ICACHE_FLASH_ATTR trim_str_lstack(char *str) {

    char *str_dest, *str_source; //, *pos;

    str_dest = str_source = str;

    do {
//        if (*str_source == '\033') {
//            pos = strchr(str_source, 'm');
//            if (pos) {
//                str_source = pos + 1;
//            } else {
//                str_source++;
//            }
//        }

        while (*str_source == '\n') str_source++;

        if (str_source != str_dest) *str_dest = *str_source;

        if (*str_source == '\0') break;

        str_dest++;
        str_source++;

    } while (1);

    return str;
}

bool ICACHE_FLASH_ATTR write_to_lstack(char *str) {

    bool ret = false;
    static bool contin = false;
    char *buff = NULL;
    uint8_t count_lstack = 0;

    lstack_elem_t *lstack_new, *lstack_current, *lstack_prev;

    if (!str) {
        return ret;
    }

    if (contin) {
        for(lstack_current = lstack; lstack_current->next; lstack_current = lstack_current->next);
        if (str[strlen(str)-1] == '\n') contin = false;
        str = trim_str_lstack(str);
        if (*str == 0) {
            os_free(str);
            return true;
        }
        buff = os_malloc(strlen(lstack_current->str)+strlen(str)+1);
        if (buff == NULL) {
            log_printf("Error allocation memory. (%s:%u)\n", __FILE__, __LINE__);
            return ret;
        }
        os_sprintf(buff, "%s%s", lstack_current->str, str);
        os_free(lstack_current->str);
        os_free(str);
        lstack_current->str = buff;
        return true;
    }

    lstack_new = os_malloc(sizeof(lstack_elem_t));

    if (lstack_new == NULL) {
        log_printf("Error allocation memory. (%s:%u)\n", __FILE__, __LINE__);
        return ret;
    }

    lstack_new->next = lstack_new->prev = NULL;
    lstack_new->str = NULL;

    if (str[strlen(str)-1] != '\n') contin = true;
    lstack_new->str = trim_str_lstack(str);

    if (*(lstack_new->str) == 0) {
        os_free(lstack_new->str);
        os_free(lstack_new);
        return true;
    }

    if (!lstack) {
        lstack = lstack_new;
        ret = true;
    } else {
        lstack_current = lstack;
        for (;;) {
            if (lstack_current->next) {
                lstack_current = lstack_current->next;
                count_lstack++;
                continue;
            } else {
                lstack_current->next = lstack_new;
                lstack_new->prev = lstack_current;
                lstack_current = lstack_new;
                count_lstack++;
            }

            if (count_lstack > LSTACK_SIZE) {
                lstack_prev = lstack->next;
                os_free(lstack_prev->prev->str);
                os_free(lstack_prev->prev);
                lstack_prev->prev = NULL;
                lstack = lstack_prev;
            }
            ret = true;
            break;
        }
    }
    return ret;
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
        write_to_lstack(buff);
//        os_free(buff);
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

