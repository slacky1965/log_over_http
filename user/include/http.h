#ifndef USER_INCLUDE_HTTP_H_
#define USER_INCLUDE_HTTP_H_

#include "platform.h"
#include "httpd.h"
#include "auth.h"
#include "espfs.h"
#include "httpdespfs.h"
#include "webpages-espfs.h"

extern HttpdBuiltInUrl builtInUrls[];

int cgi_log(HttpdConnData *connData);

#endif /* USER_INCLUDE_HTTP_H_ */
