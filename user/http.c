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

int ICACHE_FLASH_ATTR cgi_log(HttpdConnData *connData) {
    static bool no_more = false;
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

    char lt = '<';
    char gt = '>';
    char key[] = "<>";

    char *str = get_str_from_log_queue(true);

    while(str) {
        char *pos, *pstr = str;

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

        httpdSend(connData, enter, -1);
        str = get_str_from_log_queue(false);
    }

    no_more = false;
    os_sprintf(buff, "<br/>Free memory: %u bytes<br/>", system_get_free_heap_size());
    httpdSend(connData, buff, strlen(buff));
    httpdSend(connData, end_doc, -1);
    return HTTPD_CGI_DONE;

}

