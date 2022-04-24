#include "osapi.h"
#include "user_interface.h"
#include "mem.h"

#include "http.h"
#include "wifi.h"
#include "log.h"

HttpdBuiltInUrl builtInUrls[] = {
        {"/", cgiRedirect, "/index.html"},
        {"/index.html", cgi_log, NULL},
        {NULL, NULL, NULL}
};

typedef struct {
    char *strings[LSTACK_SIZE+2];
    int   str_count;
    int   print_count;
    void *file;
} log_data_t;

int ICACHE_FLASH_ATTR cgi_log(HttpdConnData *connData) {

    static bool no_more = false;
    char *err = NULL;
    const char *end_doc = "</body></html>";
    const char *enter = "<br/>\n";
    char buff[128];

    if (!no_more) {
        if (cgiEspFsHook(connData) == HTTPD_CGI_DONE) {
            no_more = true;
            connData->cgiData = NULL;
        }
        return HTTPD_CGI_MORE;
    }

    log_data_t *log_data = connData->cgiData;

    if (connData->conn==NULL) {
        //Connection aborted. Clean up.
        os_free(log_data);
        return HTTPD_CGI_DONE;
    }

    if (log_data == NULL) {
        //First call to this cgi. Open the file so we can read it.
        log_data = os_malloc(sizeof(log_data_t));
        if (log_data == NULL) {
            err = "Error allocation memory";
            httpd_printf("%s. (%s:%u)\n", err, __FILE__, __LINE__);
            httpdSendErr(connData, HTTPD_500_INTERNAL_SERVER_ERROR, err);
            return HTTPD_CGI_DONE;
        }

        log_data->file = NULL;
        log_data->str_count = log_data->print_count = 0;

        lstack_elem_t *lstack = get_lstack();

        if (lstack == NULL) {
            os_free(log_data);
            err = "No log file";
            httpd_printf("%s. (%s:%u)\n", err, __FILE__, __LINE__);
            httpdSendErr(connData, HTTPD_500_INTERNAL_SERVER_ERROR, err);
            return HTTPD_CGI_DONE;
        }

        for (int i = 0; i < LSTACK_SIZE+2; i++) {
            log_data->strings[i] = NULL;
        }

        while(lstack) {
            if (lstack->str && *(lstack->str) != 0) {
                asprintf(&(log_data->strings[log_data->str_count]), "%s", lstack->str);
                if (log_data->strings[log_data->str_count] == NULL) {
                    log_printf("Error allocation memory. (%s:%u)\n", __FILE__, __LINE__);
                }
                log_data->str_count++;
            }
            lstack = lstack->next;
        }

        connData->cgiData = log_data;
    }

    if (log_data->print_count <= log_data->str_count) {
        char lt = '<';
        char gt = '>';
        char key[] = "<>";
        char *pos, *pstr = log_data->strings[log_data->print_count];

        if (pstr) {
            pos = strpbrk(pstr, key);

            while (pos) {
                size_t len = pos - pstr;
                if (len > 0) {
                    httpdSend(connData, pstr, len);
                }
                if (*pos == lt) {
                    httpdSend(connData, "&lt", 3);
                } else if (*pos == gt) {
                    httpdSend(connData, "&gt", 3);
                }
                pstr = pos + 1;
                pos = strpbrk(pstr, key);
            }

            if (strlen(pstr)) {
                httpdSend(connData, pstr, strlen(pstr));
            }

            os_free(log_data->strings[log_data->print_count]);
        }

        httpdSend(connData, enter, -1);
        log_data->print_count++;
        return HTTPD_CGI_MORE;
    }

    no_more = false;
    os_sprintf(buff, "<br/>Free memory: %u bytes<br/>", system_get_free_heap_size());
    httpdSend(connData, buff, strlen(buff));
    httpdSend(connData, end_doc, -1);
    os_free(log_data);
    return HTTPD_CGI_DONE;
}

