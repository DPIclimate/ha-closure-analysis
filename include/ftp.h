#ifndef PROGRAM_FTP_H
#define PROGRAM_FTP_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <errno.h>

#include "utils.h"

CURLcode FTPRequest(const char* url, Utils_ReqData_TypeDef* stream);

#endif //PROGRAM_FTP_H
