#include <stdio.h>
#include <stdlib.h>
#include "admp_err.h"

#define ADMP_ERR_OFFSET ADMP_ERR_INVALID_ARGS

/* printerr: print error message */
void admp_printerr(admp_res_t res, const char *err_func)
{
    char *errmsg[] = {
	"Invalid arguments (e.g., NULL)",
	"socket() error",
	"IW command of ioctl() error",
	"IF command of ioctl() error",
	"close() error",
    };

    if (res < ADMP_ERR_OFFSET || res > ADMP_ERR_COUNT)
	return ;

    if (err_func == NULL)
	return ;

    perror(err_func);
    fprintf(stderr, "\n%s in %s\n", errmsg[res - ADMP_ERR_OFFSET], err_func);
}
